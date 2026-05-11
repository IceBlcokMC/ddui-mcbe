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

namespace {

constexpr auto MessageBoxDataStoreName = "script_ui";
constexpr auto MessageBoxPropertyName  = "message_box";
constexpr auto CustomFormDataStoreName = "script_ui";
constexpr auto CustomFormPropertyName  = "custom_form";

// These ids must exist in the client's DataDrivenUIRepository. A server-side mod
// cannot create the matching ddui/root document just by sending a ShowScreen packet.
constexpr auto DefaultMessageBoxScreenId = "script_ui:message_box";
constexpr auto DefaultCustomFormScreenId = "script_ui:custom_form";

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
        payload.mScreenId = screenId;
        payload.mFormId   = formId;
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

    uint        formId  = ll::form::FormIdManager::genFormId();
    std::string ds      = MessageBoxDataStoreName;
    auto&       state   = mImpl->states[&player];
    state.currentFormId = formId;

    // 初始化属性
    sync->setPropertyUpdateAllowed(ds, MessageBoxPropertyName, "response", true);
    sync->setPath(ds, MessageBoxPropertyName, "title", form.getTitle(), true, true);
    sync->setPath(ds, MessageBoxPropertyName, "body", form.getBody(), true, true);

    sync->setPath(ds, MessageBoxPropertyName, "button1.text", form.getButton1(), true, true);
    sync->setPath(ds, MessageBoxPropertyName, "button2.text", form.getButton2(), true, true);
    sync->setPath(ds, MessageBoxPropertyName, "response", -1.0, true, true);

    auto sub = sync->listen(
        ds,
        MessageBoxPropertyName,
        "response",
        [&player, cb = form.getCallback()](cereal::DynamicValue const* val) {
            if (cb && val) {
                // TODO: Fake SDK 未暴露完整 DynamicValue 解包，暂传固定或强转
                cb(player, 1);
            }
        }
    );

    mImpl->states[&player].subscriptions.push_back(std::move(sub));

    flush(player);
    mImpl->sendShowPacket(player, screenId, formId, formId);
}
void UIManager::show(ServerPlayer& player, const CustomForm& form) { show(player, form, DefaultCustomFormScreenId); }

void UIManager::show(ServerPlayer& player, const CustomForm& form, std::string const& screenId) {
    auto* sync = player.mDataStoreSync.get();
    if (!sync) return;

    uint        formId  = ll::form::FormIdManager::genFormId();
    std::string ds      = CustomFormDataStoreName;
    auto&       state   = mImpl->states[&player];
    state.currentFormId = formId;

    sync->setPropertyUpdateAllowed(ds, CustomFormPropertyName, "submit", true);
    sync->setPath(ds, CustomFormPropertyName, "title", form.getTitle(), true, true);

    auto const& controls = form.getControls();
    for (size_t i = 0; i < controls.size(); ++i) {
        // 按照 PathUtility 的解析规则，使用数组语法 controls[i]
        std::string bp = "controls[" + std::to_string(i) + "]";

        // 属性访问使用 '.'
        sync->setPath(ds, CustomFormPropertyName, bp + ".type", controls[i]->type, true, true);
        sync->setPath(ds, CustomFormPropertyName, bp + ".text", controls[i]->text, true, true);

        if (controls[i]->type == "button") {
            std::string btnPath = bp + ".pressed";
            sync->setPath(ds, CustomFormPropertyName, btnPath, false, true, true);
            sync->setPropertyUpdateAllowed(ds, CustomFormPropertyName, btnPath, true);

            auto sub = sync->listen(
                ds,
                CustomFormPropertyName,
                btnPath,
                [&player, btnCb = controls[i]->onClick](cereal::DynamicValue const* val) {
                    (void)val;
                    if (btnCb) btnCb(player); 
                }
            );
            mImpl->states[&player].subscriptions.push_back(std::move(sub));
        } else {
            sync->setPath(ds, CustomFormPropertyName, bp + ".value", controls[i]->value, true, true);
            sync->setPropertyUpdateAllowed(ds, CustomFormPropertyName, bp + ".value", true);
        }
    }

    auto submitSub = sync->listen(
        ds,
        CustomFormPropertyName,
        "submit",
        [&player, cb = form.getSubmitCallback()](cereal::DynamicValue const* val) {
            (void)val;
            if (cb) cb(player);
        }
    );
    mImpl->states[&player].subscriptions.push_back(std::move(submitSub));

    flush(player);
    mImpl->sendShowPacket(player, screenId, formId, formId);
}

} // namespace ddui
