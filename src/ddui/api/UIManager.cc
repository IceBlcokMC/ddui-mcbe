#include "UIManager.h"
#include "CustomForm.h"
#include "MessageBox.h"

#include "ll/api/base/Macro.h"

#include "ll/api/form/FormIdManager.h"

#include "mc/deps/core/utility/pub_sub/Subscription.h"
#include "mc/network/PacketSender.h"
#include "mc/network/packet/ClientboundDataDrivenUICloseScreenPacket.h"
#include "mc/network/packet/ClientboundDataDrivenUICloseScreenPacketPayload.h"
#include "mc/network/packet/ClientboundDataDrivenUIShowScreenPacket.h"
#include "mc/network/packet/ClientboundDataDrivenUIShowScreenPacketPayload.h"
#include "mc/scripting/data_sync/DDUI.h"
#include "mc/scripting/data_sync/DataStoreSyncServer.h"
#include "mc/scripting/data_sync/PathQueryError.h"
#include "mc/server/ServerPlayer.h"

// #include "mc/scripting/data_sync/PathUtility.h"
#include "ddui/patches/PathUtility.h"

#include "nlohmann/json.hpp"
#include "nlohmann/json_fwd.hpp"

#include <unordered_map>
#include <vector>


ClientboundDataDrivenUIShowScreenPacketPayload ::ClientboundDataDrivenUIShowScreenPacketPayload() = default;

ClientboundDataDrivenUIShowScreenPacketPayload::ClientboundDataDrivenUIShowScreenPacketPayload(
    ClientboundDataDrivenUIShowScreenPacketPayload const&
) = default;

Bedrock::PubSub::Subscription::Subscription(Bedrock::PubSub::Subscription const&) = default;

namespace ddui {

namespace {

constexpr auto DefaultMessageBoxScreenId = "minecraft:message_box";
constexpr auto DefaultCustomFormScreenId = "minecraft:custom_form";

void applyJsonToDataStore(
    Bedrock::DDUI::DataStoreSyncServer& sync,
    const std::string&                  ds,
    const std::string&                  prop,
    const nlohmann::json&               data
) {
    std::string jsonStr = data.dump();

    auto opt = Bedrock::DDUI::PathUtility::stringToDynamicValue(jsonStr);
    if (!opt) {
        throw std::runtime_error("Failed to convert JSON to dynamic value");
    }
    sync.set(ds, prop, *opt, true);
}

} // namespace

struct UIManager::Impl {
    struct PlayerState {
        std::vector<Bedrock::PubSub::Subscription> subscriptions;
        std::optional<uint>                        currentFormId;
    };
    std::unordered_map<void*, PlayerState> states;

    void sendShowPacket(
        ServerPlayer&       player,
        const std::string&  screenId,
        uint                formId,
        std::optional<uint> dataInstanceId = std::nullopt
    ) {
        ClientboundDataDrivenUIShowScreenPacketPayload payload;
        payload.mScreenId       = screenId;
        payload.mFormId         = formId;
        payload.mDataInstanceId = dataInstanceId.value_or(formId);

        ClientboundDataDrivenUIShowScreenPacket packet(std::move(payload));
        player.mPacketSender.sendToClient(&player.getUserEntityIdentifier(), packet);
    }
};

UIManager::UIManager() : mImpl(std::make_unique<Impl>()) {}
UIManager::~UIManager() = default;

UIManager& UIManager::getInstance() {
    static UIManager instance;
    return instance;
}

void UIManager::flush(ServerPlayer& player) {
    std::cout << "flush: mDataStoreSync=" << player.mDataStoreSync << std::endl;
    if (player.mDataStoreSync) {
        Bedrock::DDUI::sendDataStorePacketsToClient(
            *player.mDataStoreSync,
            player.mPacketSender,
            &player.getUserEntityIdentifier()
        );
    }
}

void UIManager::show(ServerPlayer& player, const MessageBox& form) { show(player, form, DefaultMessageBoxScreenId); }

void UIManager::show(ServerPlayer& player, const MessageBox& form, std::string const& screenId) {
    auto* sync = player.mDataStoreSync.get();
    if (!sync) return;

    uint        formId = ll::form::FormIdManager::genFormId();
    std::string ds     = "minecraft";

    // 1. 构建符合抓包规范的 MessageBox 对象
    nlohmann::json data = {
        {"title",   form.getTitle()                                                },
        {"body",    form.getBody()                                                 },
        {"button1",
         {{"label", form.getButton1()},
          {"onClick", 0}, // 对应抓包中的 onClick: 0
          {"tooltip", ""}}                                                         },
        {"button2", {{"label", form.getButton2()}, {"onClick", 0}, {"tooltip", ""}}}
    };

    // 2. 注入数据
    applyJsonToDataStore(*sync, ds, "message_box_data", data);

    // 3. 激活表单状态（必须！）
    cereal::DynamicValue active{};
    active.mType          = cereal::DynamicValue::Type::Boolean;
    active.mStorage.mBool = true;
    sync->set(ds, "ddui_form_active", active, true);

    // 4. 允许客户端回传响应
    sync->setPropertyUpdateAllowed(ds, "message_box_data", "response", true);

    // 5. 监听响应
    auto sub = sync->listen(
        ds,
        "message_box_data",
        "response",
        [&player, cb = form.getCallback()](cereal::DynamicValue const* val) {
            if (cb) cb(player, 1); // TODO 逻辑处理
        }
    );
    mImpl->states[&player].subscriptions.push_back(std::move(sub));

    flush(player);
    mImpl->sendShowPacket(player, "minecraft:message_box", formId, formId);
}
void UIManager::show(ServerPlayer& player, const CustomForm& form) { show(player, form, DefaultCustomFormScreenId); }

void UIManager::show(ServerPlayer& player, const CustomForm& form, std::string const& screenId) {
    auto* sync = player.mDataStoreSync.get();
    if (!sync) return;

    uint        formId = ll::form::FormIdManager::genFormId();
    std::string ds     = "minecraft";
    std::string prop   = "custom_form_data";

    nlohmann::json layoutObj = nlohmann::json::object();
    auto const&    controls  = form.getControls();

    for (size_t i = 0; i < controls.size(); ++i) {
        nlohmann::json item;
        std::string    type = controls[i]->type;

        // 核心：OreUI 依赖 xxx_visible 字段来决定显示哪个控件
        item["visible"]         = true;
        item[type + "_visible"] = true;

        if (type == "label") {
            item["text"] = controls[i]->text;
        } else if (type == "toggle") {
            item["label"]       = controls[i]->text;
            item["toggled"]     = std::get<bool>(controls[i]->value);
            item["description"] = "";
            item["disabled"]    = false;
        } else if (type == "slider") {
            item["label"]    = controls[i]->text;
            item["value"]    = std::get<double>(controls[i]->value);
            item["minValue"] = 0.0;
            item["maxValue"] = 100.0;
            item["step"]     = 1.0;
            item["disabled"] = false;
        } else if (type == "button") {
            item["label"]    = controls[i]->text;
            item["onClick"]  = 0;
            item["tooltip"]  = "";
            item["disabled"] = false;
        }

        // 使用字符串索引以符合 Mojang "Fake Array"
        layoutObj[std::to_string(i)] = item;
    }
    layoutObj["length"] = controls.size();

    nlohmann::json root = {
        {"title",       form.getTitle()                                               },
        {"closeButton", {{"button_visible", true}, {"label", "Close"}, {"onClick", 0}}},
        {"layout",      layoutObj                                                     }
    };

    // 1. 注入完整树
    applyJsonToDataStore(*sync, ds, prop, root);

    // 2. 激活 UI 状态
    cereal::DynamicValue active{};
    active.mType          = cereal::DynamicValue::Type::Boolean;
    active.mStorage.mBool = true;
    sync->set(ds, "ddui_form_active", active, true);

    // 3. 注册所有子控件的可写权限（否则滑块、开关动不了）
    for (size_t i = 0; i < controls.size(); ++i) {
        std::string bp = "layout." + std::to_string(i); // 注意：这里用 '.'
        if (controls[i]->type == "toggle") {
            sync->setPropertyUpdateAllowed(ds, prop, bp + ".toggled", true);
        } else if (controls[i]->type == "slider") {
            sync->setPropertyUpdateAllowed(ds, prop, bp + ".value", true);
        }
        // auto submitSub = sync->listen(
        //     ds,
        //     "Property",
        //     "submit",
        //     [&player, cb = form.getSubmitCallback()](cereal::DynamicValue const* val) {
        //         (void)val;
        //         if (cb) cb(player);
        //     }
        // );
        // mImpl->states[&player].subscriptions.push_back(std::move(submitSub));
    }

    flush(player);
    mImpl->sendShowPacket(player, "minecraft:custom_form", formId, formId);
}

} // namespace ddui
