#include "test.h"

#include <cstddef>
#include <iostream>


#ifdef DDUI_WITH_CAPTURE_PACKET
#include "fmt/base.h"

#include "ila/event/minecraft/server/SendMultiplePacketEvent.h"
#include "ila/event/minecraft/server/SendPacketEvent.h"

#include "ll/api/event/EventBus.h"
#include "ll/api/event/player/PlayerJoinEvent.h"

#include "mc/network/MinecraftPacketIds.h"
#include "mc/network/Packet.h"

void DDUI_TEST::capture_packet() {
    std::cout << "DDUI TEST CAPTURE PACKET CALLED" << std::endl;
    ll::event::EventBus::getInstance().emplaceListener<ila::mc::SendPacketBeforeEvent<>>(
        [](ila::mc::SendPacketBeforeEvent<>& ev) {
            auto id = ev.packet().getId();
            if (id == MinecraftPacketIds::ClientboundDataDrivenUIShowScreen
                || id == MinecraftPacketIds::ClientboundDataDrivenUICloseScreen
                || id == MinecraftPacketIds::ClientboundDataDrivenUIReload
                || id == MinecraftPacketIds::ServerboundDataDrivenScreenClosed) {
                fmt::print("{}：{}\n", ev.packet().getName(), ev.packet().toString());
            }
        }
    );
}
#endif


#include "ddui/api/CustomForm.h"
#include "ddui/api/MessageBox.h"
#include "ddui/api/Observable.h"
#include "ddui/api/UIManager.h"

void DDUI_TEST::init() {
    std::cout << "DDUI TEST INIT CALLED" << std::endl;

    ll::event::EventBus::getInstance().emplaceListener<ll::event::PlayerJoinEvent>(
        [](ll::event::PlayerJoinEvent& event) {
            auto& player = event.self();

            // static ddui::ObservableString dynamicProgress("script_ui", "message_box", "body", "正在初始化资源0%...");

            player.sendMessage("test form sending...");

            ddui::CustomForm f{"热更新 DDUI 测试界面"};
            f.label("初始化中...").button("确认", NULL).button("取消", nullptr).then([](Player& player) {});
            ddui::UIManager::getInstance().show(player, f);
        }
    );
}
