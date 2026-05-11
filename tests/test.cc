#include "test.h"

#include "ll/api/event/EventBus.h"
#include "ll/api/event/player/PlayerJoinEvent.h"

#include "ddui/api/MessageBox.h"
#include "ddui/api/Observable.h"
#include "ddui/api/UIManager.h"
#include "ddui/api/CustomForm.h"
#include <cstddef>

void DDUI_TEST::init() {
    std::cout << "DDUI TEST INIT CALLED" << std::endl;

    ll::event::EventBus::getInstance().emplaceListener<ll::event::PlayerJoinEvent>(
        [](ll::event::PlayerJoinEvent& event) {
            auto& player = event.self();

            static ddui::ObservableString dynamicProgress("script_ui", "message_box", "body", "正在初始化资源 0%...");

            player.sendMessage("test form sending...");

            ddui::CustomForm f{"热更新 DDUI 测试界面"};
            f.label("初始化中...").button("确认", NULL).button("取消",nullptr).then([](Player& player) {});
            ddui::UIManager::getInstance().show(player, f);
        }
    );
}