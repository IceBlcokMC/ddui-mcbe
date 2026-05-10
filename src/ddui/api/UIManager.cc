#include "UIManager.h"
#include "CustomForm.h"
#include "MessageBox.h"

#include "ll/api/base/Macro.h"

#include "ll/api/form/FormIdManager.h"

#include "mc/deps/core/utility/pub_sub/Subscription.h"
#include "mc/network/packet/ClientboundDataDrivenUIShowScreenPacket.h"
#include "mc/network/packet/ClientboundDataDrivenUIShowScreenPacketPayload.h"
#include "mc/scripting/data_sync/DDUI.h"
#include "mc/scripting/data_sync/DataStoreSyncServer.h"
#include "mc/scripting/data_sync/PathQueryError.h"
#include "mc/scripting/data_sync/PathUtility.h"
#include "mc/server/ServerPlayer.h"

#include <unordered_map>
#include <vector>


ClientboundDataDrivenUIShowScreenPacketPayload ::ClientboundDataDrivenUIShowScreenPacketPayload() = default;

ClientboundDataDrivenUIShowScreenPacketPayload::ClientboundDataDrivenUIShowScreenPacketPayload(
    ClientboundDataDrivenUIShowScreenPacketPayload const&
) = default;

Bedrock::PubSub::Subscription::Subscription(Bedrock::PubSub::Subscription const&) = default;

namespace ddui {

struct UIManager::Impl {
    struct PlayerState {
        std::vector<Bedrock::PubSub::Subscription> subscriptions;
    };
    std::unordered_map<void*, PlayerState> states;

    void sendShowPacket(ServerPlayer& player, const std::string& screenId, uint formId) {
        ClientboundDataDrivenUIShowScreenPacketPayload payload;
        payload.mScreenId       = screenId;
        payload.mFormId         = formId;
        payload.mDataInstanceId = std::nullopt;

        ClientboundDataDrivenUIShowScreenPacket packet(std::move(payload));
        player.sendNetworkPacket(packet);
    }
};

UIManager::UIManager() : mImpl(std::make_unique<Impl>()) {}
UIManager::~UIManager() = default;

UIManager& UIManager::getInstance() {
    static UIManager instance;
    return instance;
}

void UIManager::flush(ServerPlayer& player) {
    if (player.mDataStoreSync) {
        Bedrock::DDUI::sendDataStorePacketsToClient(*player.mDataStoreSync, player.mPacketSender, nullptr);
    }
}

void UIManager::show(ServerPlayer& player, const MessageBox& form) {
    auto* sync = player.mDataStoreSync.get();
    if (!sync) return;

    // 清理旧的订阅
    mImpl->states[&player].subscriptions.clear();
    uint        formId = ll::form::FormIdManager::genFormId();
    std::string ds     = "script_ui"; // 客户端 UI JSON 中定义的数据存储名

    // 初始化属性
    sync->setPropertyUpdateAllowed(ds, "message_box", "response", true);
    sync->setPath(ds, "message_box", "title", form.getTitle(), true, true);
    sync->setPath(ds, "message_box", "body", form.getBody(), true, true);
    sync->setPath(ds, "message_box", "button1", form.getButton1(), true, true);
    sync->setPath(ds, "message_box", "button2", form.getButton2(), true, true);
    sync->setPath(ds, "message_box", "response", -1.0, true, true);

    // 订阅客户端点击返回
    auto sub = sync->listen(
        ds,
        "message_box",
        "response",
        [&player, cb = form.getCallback()](cereal::DynamicValue const* val) {
            if (cb && val) {
                // 简化：假定 response = 1 为 button1, 2 为 button2
                // cb(player, static_cast<int>(val->asNumber()));
                cb(player, 1); // TODO: fix
            }
        }
    );

    mImpl->states[&player].subscriptions.push_back(std::move(sub));

    // 1. 同步 DataStore 状态给客户端
    flush(player);
    // 2. 发送配对的 ShowScreen 数据包以打开界面
    mImpl->sendShowPacket(player, "script_ui:message_box", formId);
}

void UIManager::show(ServerPlayer& player, const CustomForm& form) {
    auto* sync = player.mDataStoreSync.get();
    if (!sync) return;

    mImpl->states[&player].subscriptions.clear();
    uint        formId = ll::form::FormIdManager::genFormId();
    std::string ds     = "script_ui";

    sync->setPropertyUpdateAllowed(ds, "custom_form", "submit", true);
    sync->setPath(ds, "custom_form", "title", form.getTitle(), true, true);

    auto const& controls = form.getControls();
    for (size_t i = 0; i < controls.size(); ++i) {
        std::string bp = "controls/" + std::to_string(i);
        sync->setPath(ds, "custom_form", bp + "/type", controls[i]->type, true, true);
        sync->setPath(ds, "custom_form", bp + "/text", controls[i]->text, true, true);

        if (controls[i]->type == "button") {
            // 给按钮注册特殊的点击监听
            std::string btnPath = bp + "/pressed";
            sync->setPath(ds, "custom_form", btnPath, false, true, true);
            sync->setPropertyUpdateAllowed(ds, "custom_form", btnPath, true);

            auto sub = sync->listen(
                ds,
                "custom_form",
                btnPath,
                [&player, btnCb = controls[i]->onClick](cereal::DynamicValue const* val) {
                    if (btnCb) btnCb(player); // 触发按钮点击的内联事件
                }
            );
            mImpl->states[&player].subscriptions.push_back(std::move(sub));
        } else {
            sync->setPath(ds, "custom_form", bp + "/value", controls[i]->value, true, true);
            sync->setPropertyUpdateAllowed(ds, "custom_form", bp + "/value", true);
        }
    }

    // 订阅表单整体提交
    auto submitSub = sync->listen(
        ds,
        "custom_form",
        "submit",
        [&player, cb = form.getSubmitCallback()](cereal::DynamicValue const* val) {
            if (cb) cb(player);
        }
    );
    mImpl->states[&player].subscriptions.push_back(std::move(submitSub));

    flush(player);
    mImpl->sendShowPacket(player, "script_ui:custom_form", formId); // 对应的客户端 UI id
}

} // namespace ddui