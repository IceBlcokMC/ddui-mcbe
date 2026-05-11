#include "test.h"
#include "ddui/api/FormBase.h"

#include <cstddef>
#include <iostream>

#ifdef DDUI_WITH_CAPTURE_PACKET
#include "fmt/base.h"
#include "ila/event/minecraft/server/SendMultiplePacketEvent.h"
#include "ila/event/minecraft/server/SendPacketEvent.h"
#include "ll/api/event/EventBus.h"
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
                || id == MinecraftPacketIds::ClientboundDataStore
                || id == MinecraftPacketIds::ServerboundDataDrivenScreenClosed
                || id == MinecraftPacketIds::ServerboundDataStore) {
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

#include "ll/api/command/CommandHandle.h"
#include "ll/api/command/CommandRegistrar.h"
#include "ll/api/command/Overload.h"

#include "mc/world/actor/player/Player.h"

void DDUI_TEST::init() {
    std::cout << "DDUI TEST INIT CALLED" << std::endl;

    auto& ov = ll::command::CommandRegistrar::getInstance(false).getOrCreateCommand("dduit");

    ov.overload().text("message_box").execute([](CommandOrigin const& origin, CommandOutput& output) {
        auto& player = static_cast<Player&>(*origin.getEntity());

        ddui::MessageBox msg{"Test DDUI MessageBox"};
        msg.setContent("This is the highly reactive DDUI MessageBox.")
            .appendButton1("Upper", "UpperTip")
            .appendButton2("Lower", "LowerTip")
            .sendTo(player, [](Player& player, std::optional<int> idx, std::optional<ddui::FormCancelReason>) {
                fmt::print("Player {} clicked MessageBox button {}\n", player.getRealName(), idx.value_or(-1));
            });
    });

    ov.overload().text("custom_form").execute([](CommandOrigin const& origin, CommandOutput& output) {
        auto& player = static_cast<Player&>(*origin.getEntity());

        static auto obsTitle    = std::make_shared<ddui::ObservableString>("Test CustomForm");
        static auto obsInput    = std::make_shared<ddui::ObservableString>("Default text");
        static auto obsBool     = std::make_shared<ddui::ObservableBoolean>(true);
        static auto obsNumber   = std::make_shared<ddui::ObservableNumber>(5.0);
        static auto obsDropdown = std::make_shared<ddui::ObservableNumber>(1.0);

        ddui::CustomForm form{obsTitle};         // 使用 ObservableTitle
        form.appendHeader("Dynamic Form Header") // 字面量
            .appendLabel("Please enter your configuration below:")
            .appendDivider()
            .appendSpacer()
            .appendToggle("Enable feature", obsBool, "Toggle it on or off")
            .appendSlider("Intensity", obsNumber, 0.0, 10.0, 1.0, "Slide to change value")
            .appendDropdown(
                "Select Profile",
                obsDropdown,
                {
                    {"Option 1", 0, "Description 1"},
                    {"Option 2", 1, "Description 2"},
                    {"Option 3", 2, "Description 3"}
        },
                "Select a predefined profile"
            )
            .appendTextField("Input text", obsInput, "Type something here")
            .appendButton(
                "Random Update Server-Side",
                [&](Player& /* player */) {
                    obsTitle->set(fmt::format("Random Updated! {}", std::rand() % 100));
                    obsInput->set(fmt::format("Random Value: {}", std::rand()));
                    obsBool->set(!obsBool->get());
                    obsNumber->set(std::rand() % 10);
                    obsDropdown->set(std::rand() % 3);
                },
                "Click me to trigger an observable flush from the server"
            )
            .sendTo(player, [](Player&, std::optional<ddui::FormCancelReason>) {
                fmt::print("Custom form closed/submitted.\n");
            });
    });
}