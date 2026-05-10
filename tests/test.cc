#include "test.h"

#include "ll/api/event/EventBus.h"
#include "ll/api/event/player/PlayerJoinEvent.h"

#include "ddui/api/MessageBox.h"
#include "ddui/api/Observable.h"
#include "ddui/api/UIManager.h"

void DDUI_TEST::init() {
    std::cout << "DDUI TEST INIT CALLED" << std::endl;

    ll::event::EventBus::getInstance().emplaceListener<ll::event::PlayerJoinEvent>(
        [](ll::event::PlayerJoinEvent& event) {
            auto& player = event.self();

            static ddui::ObservableString dynamicProgress("script_ui", "message_box", "body", "正在初始化资源 0%...");

            player.sendMessage("test form sending...");

            ddui::MessageBox msgBox{"热更新 DDUI 测试界面"};
            msgBox.body("初始化中...").button1("确认").button2("取消").then([](Player& player, int i) {});
            ddui::UIManager::getInstance().show(player, msgBox);
        }
    );
}