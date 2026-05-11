#include "UIManager.h"

#include "ll/api/base/Macro.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/event/player/PlayerDisconnectEvent.h"
#include "ll/api/form/FormIdManager.h"
#include "ll/api/memory/Hook.h"

#include "mc/deps/core/utility/pub_sub/Subscription.h"
#include "mc/network/NetworkIdentifier.h"
#include "mc/network/PacketSender.h"
#include "mc/network/ServerNetworkHandler.h"
#include "mc/network/packet/ClientboundDataDrivenUICloseScreenPacket.h"
#include "mc/network/packet/ClientboundDataDrivenUIShowScreenPacket.h"
#include "mc/network/packet/ClientboundDataDrivenUIShowScreenPacketPayload.h"
#include "mc/network/packet/ServerboundDataDrivenScreenClosedPacket.h"
#include "mc/scripting/data_sync/DDUI.h"
#include "mc/scripting/data_sync/DataStoreSyncServer.h"
#include "mc/scripting/data_sync/PathQueryError.h"
#include "mc/server/ServerPlayer.h"
#include "mc/world/actor/player/Player.h"

#include "ddui/patches/PathUtility.h"
#include "nlohmann/json.hpp"

#include <unordered_map>
#include <vector>

ClientboundDataDrivenUIShowScreenPacketPayload::ClientboundDataDrivenUIShowScreenPacketPayload() = default;
ClientboundDataDrivenUIShowScreenPacketPayload::ClientboundDataDrivenUIShowScreenPacketPayload(
    ClientboundDataDrivenUIShowScreenPacketPayload const&
)                                                                                 = default;
Bedrock::PubSub::Subscription::Subscription(Bedrock::PubSub::Subscription const&) = default;

namespace ddui {

namespace {

constexpr auto DDUI_GLOBAL_NAMESPACE = "minecraft";
constexpr auto DDUI_ACTIVE_FLAG      = "ddui_form_active";

constexpr auto MSG_BOX_SCREEN_ID = "minecraft:message_box";
constexpr auto MSG_BOX_PROP      = "message_box_data";

constexpr auto CSTM_FORM_SCREEN_ID = "minecraft:custom_form";
constexpr auto CSTM_FORM_PROP      = "custom_form_data";

void applyJsonToDataStore(
    Bedrock::DDUI::DataStoreSyncServer& sync,
    const std::string&                  prop,
    const nlohmann::json&               data
) {
    std::string jsonStr = data.dump();
    auto        opt     = Bedrock::DDUI::PathUtility::stringToDynamicValue(jsonStr);
    if (!opt) {
        throw std::runtime_error("Failed to parse JSON to cereal::DynamicValue");
    }
    sync.set(DDUI_GLOBAL_NAMESPACE, prop, *opt, true);
}

cereal::DynamicValue getNullDynamicValue() { return cereal::DynamicValue{}; }

cereal::DynamicValue getBooleanDynamicValue(bool value) {
    cereal::DynamicValue val;
    val.mType          = cereal::DynamicValue::Type::Boolean;
    val.mStorage.mBool = value;
    return val;
}

} // namespace

LL_AUTO_TYPE_INSTANCE_HOOK(
    DDUIScreenClosedHook,
    ll::memory::HookPriority::Normal,
    ServerNetworkHandler,
    &ServerNetworkHandler::$handle,
    void,
    ::NetworkIdentifier const&                       source,
    ::ServerboundDataDrivenScreenClosedPacket const& packet
) {
    origin(source, packet);
    if (auto player = this->_getServerPlayer(source, packet.mSenderSubId)) {
        UIManager::getInstance().handleScreenClosed(*player);
    }
}

struct UIManager::Impl {
    struct PlayerUIContext {
        std::vector<Bedrock::PubSub::Subscription>                        subscriptions;
        std::vector<std::pair<std::shared_ptr<ObservableBase>, uint64_t>> observableSyncs;
        CustomForm::SubmitCallback                                        submitCallback;
        MessageBox::Callback                                              msgBoxCallback;
        uint                                                              currentFormId = 0;
        std::string                                                       currentProp   = "";
    };
    std::unordered_map<Player*, PlayerUIContext> contexts;

    template <typename T>
    void bindObservableProperty(
        PlayerUIContext&                    ctx,
        ServerPlayer*                       sp,
        Bedrock::DDUI::DataStoreSyncServer* sync,
        const std::string&                  prop,
        const std::string&                  path,
        const ObservableValue<T>&           obsVal
    ) {
        if (obsVal.isObservable()) {
            auto obs    = obsVal.getObservable();
            auto syncId = obs->_subscribeSync([sp, sync, prop, path](T const& val) {
                sync->setPath(DDUI_GLOBAL_NAMESPACE, prop, path, val, true, true);
                Bedrock::DDUI::sendDataStorePacketsToClient(*sync, sp->mPacketSender, &sp->getUserEntityIdentifier());
            });
            ctx.observableSyncs.push_back({obs, syncId});
        }
    }

    void flushAndShowPacket(ServerPlayer& sp, uint formId, const std::string& screenId) {
        if (sp.mDataStoreSync) {
            Bedrock::DDUI::sendDataStorePacketsToClient(
                *sp.mDataStoreSync,
                sp.mPacketSender,
                &sp.getUserEntityIdentifier()
            );
        }

        ClientboundDataDrivenUIShowScreenPacketPayload payload;
        payload.mScreenId       = screenId;
        payload.mFormId         = formId;
        payload.mDataInstanceId = std::nullopt; // 切勿给 InstanceId 赋值，否则客户端无法响应

        ClientboundDataDrivenUIShowScreenPacket packet(std::move(payload));
        sp.mPacketSender.sendToClient(&sp.getUserEntityIdentifier(), packet);
    }

    void closeAndCleanupUI(Player& player, bool isServerSideCall = true) {
        auto* sp = static_cast<ServerPlayer*>(&player);
        auto  it = contexts.find(&player);
        if (it == contexts.end()) return;

        uint        formId = it->second.currentFormId;
        std::string prop   = it->second.currentProp;

        it->second.subscriptions.clear();
        for (auto& [obsBase, syncId] : it->second.observableSyncs) {
            if (auto obsStr = std::dynamic_pointer_cast<ObservableString>(obsBase)) obsStr->_unsubscribeSync(syncId);
            else if (auto obsNum = std::dynamic_pointer_cast<ObservableNumber>(obsBase))
                obsNum->_unsubscribeSync(syncId);
            else if (auto obsBool = std::dynamic_pointer_cast<ObservableBoolean>(obsBase))
                obsBool->_unsubscribeSync(syncId);
        }
        it->second.observableSyncs.clear();

        if (sp->mDataStoreSync) {
            sp->mDataStoreSync->set(DDUI_GLOBAL_NAMESPACE, DDUI_ACTIVE_FLAG, getBooleanDynamicValue(false), true);
            sp->mDataStoreSync->set(DDUI_GLOBAL_NAMESPACE, prop, getNullDynamicValue(), true);
            Bedrock::DDUI::sendDataStorePacketsToClient(
                *sp->mDataStoreSync,
                sp->mPacketSender,
                &sp->getUserEntityIdentifier()
            );
        }

        if (isServerSideCall) {
            ClientboundDataDrivenUICloseScreenPacket packet{};
            packet.mFormId = formId;
            sp->mPacketSender.sendToClient(&sp->getUserEntityIdentifier(), packet);
        }

        contexts.erase(it);
    }
};

UIManager::UIManager() : mImpl(std::make_unique<Impl>()) {
    ll::event::EventBus::getInstance().emplaceListener<ll::event::player::PlayerDisconnectEvent>(
        [this](ll::event::player::PlayerDisconnectEvent& ev) { mImpl->closeAndCleanupUI(ev.self(), false); }
    );
}

UIManager::~UIManager() = default;

UIManager& UIManager::getInstance() {
    static UIManager instance;
    return instance;
}

void UIManager::handleScreenClosed(Player& player) {
    auto it = mImpl->contexts.find(&player);
    if (it != mImpl->contexts.end()) {
        auto cbcstm = std::move(it->second.submitCallback);
        auto cbmsg  = std::move(it->second.msgBoxCallback);
        mImpl->closeAndCleanupUI(player, false);

        if (cbcstm) cbcstm(player, FormCancelReason::UserClosed);
        else if (cbmsg) cbmsg(player, std::nullopt, FormCancelReason::UserClosed);
    }
}

bool UIManager::showMessageBox(Player& player, const MessageBox& form, bool isUpdate, MessageBox::Callback callback) {
    auto* sp   = static_cast<ServerPlayer*>(&player);
    auto* sync = sp->mDataStoreSync.get();
    if (!sync) return false;

    mImpl->closeAndCleanupUI(player);

    uint formId = ll::form::FormIdManager::genFormId();

    nlohmann::json data = {
        {"title",   form.getTitle().getValue()                                                                     },
        {"body",    form.getBody().getValue()                                                                      },
        {"button1",
         {{"label", form.getBtn1Label().getValue()}, {"onClick", 0}, {"tooltip", form.getBtn1Tooltip().getValue()}}},
        {"button2",
         {{"label", form.getBtn2Label().getValue()}, {"onClick", 0}, {"tooltip", form.getBtn2Tooltip().getValue()}}}
    };

    applyJsonToDataStore(*sync, MSG_BOX_PROP, data);
    sync->set(DDUI_GLOBAL_NAMESPACE, DDUI_ACTIVE_FLAG, getBooleanDynamicValue(true), true);

    auto& ctx          = mImpl->contexts[&player];
    ctx.currentFormId  = formId;
    ctx.currentProp    = MSG_BOX_PROP;
    ctx.msgBoxCallback = std::move(callback);

    mImpl->bindObservableProperty(ctx, sp, sync, MSG_BOX_PROP, "title", form.getTitle());
    mImpl->bindObservableProperty(ctx, sp, sync, MSG_BOX_PROP, "body", form.getBody());
    mImpl->bindObservableProperty(ctx, sp, sync, MSG_BOX_PROP, "button1.label", form.getBtn1Label());
    mImpl->bindObservableProperty(ctx, sp, sync, MSG_BOX_PROP, "button1.tooltip", form.getBtn1Tooltip());
    mImpl->bindObservableProperty(ctx, sp, sync, MSG_BOX_PROP, "button2.label", form.getBtn2Label());
    mImpl->bindObservableProperty(ctx, sp, sync, MSG_BOX_PROP, "button2.tooltip", form.getBtn2Tooltip());

    sync->setPropertyUpdateAllowed(DDUI_GLOBAL_NAMESPACE, MSG_BOX_PROP, "button1.onClick", true);
    sync->setPropertyUpdateAllowed(DDUI_GLOBAL_NAMESPACE, MSG_BOX_PROP, "button2.onClick", true);

    auto sub1 = sync->listen(
        DDUI_GLOBAL_NAMESPACE,
        MSG_BOX_PROP,
        "button1.onClick",
        [this, &player](cereal::DynamicValue const* val) {
            auto it = mImpl->contexts.find(&player);
            if (it != mImpl->contexts.end() && it->second.msgBoxCallback) {
                auto cb = std::move(it->second.msgBoxCallback);
                mImpl->closeAndCleanupUI(player);
                cb(player, 1, std::nullopt);
            }
        }
    );
    ctx.subscriptions.push_back(std::move(sub1));

    auto sub2 = sync->listen(
        DDUI_GLOBAL_NAMESPACE,
        MSG_BOX_PROP,
        "button2.onClick",
        [this, &player](cereal::DynamicValue const* val) {
            auto it = mImpl->contexts.find(&player);
            if (it != mImpl->contexts.end() && it->second.msgBoxCallback) {
                auto cb = std::move(it->second.msgBoxCallback);
                mImpl->closeAndCleanupUI(player);
                cb(player, 2, std::nullopt);
            }
        }
    );
    ctx.subscriptions.push_back(std::move(sub2));

    mImpl->flushAndShowPacket(*sp, formId, MSG_BOX_SCREEN_ID);
    return true;
}

bool UIManager::showCustomForm(
    Player&                    player,
    const CustomForm&          form,
    bool                       isUpdate,
    CustomForm::SubmitCallback callback
) {
    auto* sp   = static_cast<ServerPlayer*>(&player);
    auto* sync = sp->mDataStoreSync.get();
    if (!sync) return false;

    mImpl->closeAndCleanupUI(player);

    uint           formId    = ll::form::FormIdManager::genFormId();
    nlohmann::json layoutObj = nlohmann::json::object();
    auto const&    elements  = form.getElements();

    for (size_t i = 0; i < elements.size(); ++i) {
        nlohmann::json item;
        std::string    type = elements[i]->type;

        item["visible"]         = true;
        item[type + "_visible"] = true;

        if (type == "header" || type == "label") {
            item["text"] = elements[i]->text.getValue();
        } else if (type == "toggle") {
            item["label"]       = elements[i]->label.getValue();
            item["description"] = elements[i]->description.getValue();
            item["disabled"]    = false;
            auto obs            = std::static_pointer_cast<ObservableBoolean>(elements[i]->obsValue);
            item["toggled"]     = obs ? obs->get() : false;
        } else if (type == "slider") {
            item["label"]       = elements[i]->label.getValue();
            item["description"] = elements[i]->description.getValue();
            item["minValue"]    = elements[i]->min;
            item["maxValue"]    = elements[i]->max;
            item["step"]        = elements[i]->step;
            item["disabled"]    = false;
            auto obs            = std::static_pointer_cast<ObservableNumber>(elements[i]->obsValue);
            item["value"]       = obs ? obs->get() : elements[i]->min;
        } else if (type == "dropdown") {
            item["label"]       = elements[i]->label.getValue();
            item["description"] = elements[i]->description.getValue();
            item["disabled"]    = false;

            nlohmann::json itemsObj = nlohmann::json::object();
            for (size_t j = 0; j < elements[i]->dropdownItems.size(); ++j) {
                itemsObj[std::to_string(j)] = {
                    {"label",       elements[i]->dropdownItems[j].label      },
                    {"value",       elements[i]->dropdownItems[j].value      },
                    {"description", elements[i]->dropdownItems[j].description}
                };
            }
            itemsObj["length"] = elements[i]->dropdownItems.size();
            item["items"]      = itemsObj;

            auto obs      = std::static_pointer_cast<ObservableNumber>(elements[i]->obsValue);
            item["value"] = obs ? obs->get() : 0.0;
        } else if (type == "textfield") {
            item["label"]       = elements[i]->label.getValue();
            item["description"] = elements[i]->description.getValue();
            item["disabled"]    = false;
            auto obs            = std::static_pointer_cast<ObservableString>(elements[i]->obsValue);
            item["text"]        = obs ? obs->get() : "";
        } else if (type == "button") {
            item["label"]    = elements[i]->label.getValue();
            item["tooltip"]  = elements[i]->tooltip.getValue();
            item["disabled"] = false;
            item["onClick"]  = 0;
        }

        layoutObj[std::to_string(i)] = item;
    }
    layoutObj["length"] = elements.size();

    nlohmann::json root = {
        {"title",       form.getTitle().getValue()                                    },
        {"closeButton", {{"button_visible", true}, {"label", "Close"}, {"onClick", 0}}},
        {"layout",      layoutObj                                                     }
    };

    applyJsonToDataStore(*sync, CSTM_FORM_PROP, root);
    sync->set(DDUI_GLOBAL_NAMESPACE, DDUI_ACTIVE_FLAG, getBooleanDynamicValue(true), true);

    auto& ctx          = mImpl->contexts[&player];
    ctx.currentFormId  = formId;
    ctx.currentProp    = CSTM_FORM_PROP;
    ctx.submitCallback = std::move(callback);

    mImpl->bindObservableProperty(ctx, sp, sync, CSTM_FORM_PROP, "title", form.getTitle());

    for (size_t i = 0; i < elements.size(); ++i) {
        std::string bp   = "layout[" + std::to_string(i) + "]";
        std::string type = elements[i]->type;

        if (type == "header" || type == "label") {
            mImpl->bindObservableProperty(ctx, sp, sync, CSTM_FORM_PROP, bp + ".text", elements[i]->text);
        } else if (type == "button") {
            mImpl->bindObservableProperty(ctx, sp, sync, CSTM_FORM_PROP, bp + ".label", elements[i]->label);
            mImpl->bindObservableProperty(ctx, sp, sync, CSTM_FORM_PROP, bp + ".tooltip", elements[i]->tooltip);

            std::string path = bp + ".onClick"; // 修正 onClick 事件绑定
            sync->setPropertyUpdateAllowed(DDUI_GLOBAL_NAMESPACE, CSTM_FORM_PROP, path, true);

            auto sub = sync->listen(
                DDUI_GLOBAL_NAMESPACE,
                CSTM_FORM_PROP,
                path,
                [&player, btnCb = elements[i]->btnCallback](cereal::DynamicValue const* val) {
                    if (btnCb) btnCb(player);
                }
            );
            ctx.subscriptions.push_back(std::move(sub));
        } else if (type == "toggle") {
            mImpl->bindObservableProperty(ctx, sp, sync, CSTM_FORM_PROP, bp + ".label", elements[i]->label);
            mImpl->bindObservableProperty(ctx, sp, sync, CSTM_FORM_PROP, bp + ".description", elements[i]->description);

            std::string path = bp + ".toggled";
            sync->setPropertyUpdateAllowed(DDUI_GLOBAL_NAMESPACE, CSTM_FORM_PROP, path, true);

            if (auto obs = std::static_pointer_cast<ObservableBoolean>(elements[i]->obsValue)) {
                auto sub =
                    sync->listen(DDUI_GLOBAL_NAMESPACE, CSTM_FORM_PROP, path, [obs](cereal::DynamicValue const* val) {
                        if (val && val->mType == cereal::DynamicValue::Type::Boolean) {
                            obs->_updateFromClient(val->mStorage.mBool);
                        }
                    });
                ctx.subscriptions.push_back(std::move(sub));
            }
        } else if (type == "slider" || type == "dropdown") {
            mImpl->bindObservableProperty(ctx, sp, sync, CSTM_FORM_PROP, bp + ".label", elements[i]->label);
            mImpl->bindObservableProperty(ctx, sp, sync, CSTM_FORM_PROP, bp + ".description", elements[i]->description);

            std::string path = bp + ".value";
            sync->setPropertyUpdateAllowed(DDUI_GLOBAL_NAMESPACE, CSTM_FORM_PROP, path, true);

            if (auto obs = std::static_pointer_cast<ObservableNumber>(elements[i]->obsValue)) {
                auto sub =
                    sync->listen(DDUI_GLOBAL_NAMESPACE, CSTM_FORM_PROP, path, [obs](cereal::DynamicValue const* val) {
                        if (val && val->mType == cereal::DynamicValue::Type::Number) {
                            obs->_updateFromClient(val->mStorage.mDouble);
                        } else if (val && val->mType == cereal::DynamicValue::Type::Integer) {
                            obs->_updateFromClient(static_cast<double>(val->mStorage.mInt));
                        }
                    });
                ctx.subscriptions.push_back(std::move(sub));
            }
        } else if (type == "textfield") {
            mImpl->bindObservableProperty(ctx, sp, sync, CSTM_FORM_PROP, bp + ".label", elements[i]->label);
            mImpl->bindObservableProperty(ctx, sp, sync, CSTM_FORM_PROP, bp + ".description", elements[i]->description);

            std::string path = bp + ".text";
            sync->setPropertyUpdateAllowed(DDUI_GLOBAL_NAMESPACE, CSTM_FORM_PROP, path, true);

            if (auto obs = std::static_pointer_cast<ObservableString>(elements[i]->obsValue)) {
                auto sub =
                    sync->listen(DDUI_GLOBAL_NAMESPACE, CSTM_FORM_PROP, path, [obs](cereal::DynamicValue const* val) {
                        if (val && val->mType == cereal::DynamicValue::Type::String) {
                            obs->_updateFromClient(val->mStorage.mString);
                        }
                    });
                ctx.subscriptions.push_back(std::move(sub));
            }
        }
    }

    sync->setPropertyUpdateAllowed(DDUI_GLOBAL_NAMESPACE, CSTM_FORM_PROP, "closeButton.onClick", true);
    auto closeSub = sync->listen(
        DDUI_GLOBAL_NAMESPACE,
        CSTM_FORM_PROP,
        "closeButton.onClick",
        [this, &player](cereal::DynamicValue const* val) {
            auto it = mImpl->contexts.find(&player);
            if (it != mImpl->contexts.end() && it->second.submitCallback) {
                auto cb = std::move(it->second.submitCallback);
                mImpl->closeAndCleanupUI(player);
                cb(player, FormCancelReason::UserClosed);
            }
        }
    );
    ctx.subscriptions.push_back(std::move(closeSub));

    mImpl->flushAndShowPacket(*sp, formId, CSTM_FORM_SCREEN_ID);
    return true;
}

} // namespace ddui