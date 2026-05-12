#include "PacketCapture.h"

#include "ll/api/mod/NativeMod.h"
#include "ll/api/mod/RegisterHelper.h"

#include "fmt/base.h"

#include "ll/api/memory/Hook.h"

#include "mc/network/MinecraftPacketIds.h"
#include "mc/network/NetworkSystem.h"
#include "mc/network/Packet.h"
#include "mc/network/packet/ClientboundDataDrivenUICloseScreenPacket.h"
#include "mc/network/packet/ClientboundDataDrivenUIReloadPacket.h"
#include "mc/network/packet/ClientboundDataDrivenUIShowScreenPacket.h"
#include "mc/network/packet/ClientboundDataStorePacket.h"
#include "mc/network/packet/ServerboundDataDrivenScreenClosedPacket.h"
#include "mc/network/packet/ServerboundDataStorePacket.h"

#ifdef LL_PLAT_S
#include "mc/network/ServerNetworkHandler.h"
#endif

#ifdef LL_PLAT_C
#include "mc/client/network/ClientNetworkHandler.h"
#endif

bool filterPacketId(Packet const& pkt) {
    auto id = pkt.getId();
    return id == MinecraftPacketIds::ClientboundDataDrivenUIShowScreen  //
        || id == MinecraftPacketIds::ClientboundDataDrivenUICloseScreen //
        || id == MinecraftPacketIds::ClientboundDataDrivenUIReload      //
        || id == MinecraftPacketIds::ClientboundDataStore               //
        || id == MinecraftPacketIds::ServerboundDataDrivenScreenClosed  //
        || id == MinecraftPacketIds::ServerboundDataStore;              //
}

long long timestamp() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
        .count();
}

// C/S send packet hook
LL_TYPE_INSTANCE_HOOK(
    NetworkSystemSendHook,
    ll::memory::HookPriority::Normal,
    NetworkSystem,
    &NetworkSystem::send,
    void,
    ::NetworkIdentifier const& id,
    ::Packet const&            packet,
    ::SubClientId              recipientSubId
) {
    if (filterPacketId(packet)) {
        fmt::print("[{}][Send] {}：{}\n", timestamp(), packet.getName(), packet.toString());
    }
    origin(id, packet, recipientSubId);
}

// specify side packet hook
#define CAPTURE_RECEIVE_PACKET(HANDER, PACKET)                                                                         \
    LL_TYPE_INSTANCE_HOOK(                                                                                             \
        PACKET##ReceiveHook,                                                                                           \
        ll::memory::HookPriority::Normal,                                                                              \
        HANDER,                                                                                                        \
        &HANDER::$handle,                                                                                              \
        void,                                                                                                          \
        ::NetworkIdentifier const& source,                                                                             \
        ::PACKET const&            packet                                                                              \
    ) {                                                                                                                \
        fmt::print("[{}][Receive] {}：{}\n", timestamp(), packet.getName(), packet.toString());                        \
        origin(source, packet);                                                                                        \
    }

// Server side packet hook
#ifdef LL_PLAT_S
#define CAPTURE_SERVER_RECEIVE_PACKET(PACKET) CAPTURE_RECEIVE_PACKET(ServerNetworkHandler, PACKET)

CAPTURE_SERVER_RECEIVE_PACKET(ServerboundDataStorePacket);
CAPTURE_SERVER_RECEIVE_PACKET(ServerboundDataDrivenScreenClosedPacket);
#endif

// Client side packet hook
#ifdef LL_PLAT_C
#define CAPTURE_CLIENT_RECEIVE_PACKET(PACKET) CAPTURE_RECEIVE_PACKET(ClientNetworkHandler, PACKET)

CAPTURE_CLIENT_RECEIVE_PACKET(ClientboundDataStorePacket);
CAPTURE_CLIENT_RECEIVE_PACKET(ClientboundDataDrivenUICloseScreenPacket);
CAPTURE_CLIENT_RECEIVE_PACKET(ClientboundDataDrivenUIReloadPacket);
CAPTURE_CLIENT_RECEIVE_PACKET(ClientboundDataDrivenUIShowScreenPacket);
#endif

#define ENABLE_PACKET_CAPTURE(PACKET)  PACKET##ReceiveHook::hook();
#define DISABLE_PACKET_CAPTURE(PACKET) PACKET##ReceiveHook::unhook();

namespace capture {

PacketCapture::PacketCapture() : mSelf(*ll::mod::NativeMod::current()) {}

ll::mod::NativeMod& PacketCapture::getSelf() { return mSelf; }

PacketCapture& PacketCapture::getInstance() {
    static PacketCapture instance;
    return instance;
}

bool PacketCapture::load() { return true; }
bool PacketCapture::enable() {
    fmt::print("DDUI TEST CAPTURE PACKET CALLED\n");

    fmt::print("Registering packet capture hook...\n");
    NetworkSystemSendHook::hook();

#ifdef LL_PLAT_S
    fmt::print("Registering server packet capture hook...\n");
    ENABLE_PACKET_CAPTURE(ServerboundDataStorePacket);
    ENABLE_PACKET_CAPTURE(ServerboundDataDrivenScreenClosedPacket);
#endif

#ifdef LL_PLAT_C
    fmt::print("Registering client packet capture hook...\n");
    ENABLE_PACKET_CAPTURE(ClientboundDataStorePacket);
    ENABLE_PACKET_CAPTURE(ClientboundDataDrivenUICloseScreenPacket);
    ENABLE_PACKET_CAPTURE(ClientboundDataDrivenUIReloadPacket);
    ENABLE_PACKET_CAPTURE(ClientboundDataDrivenUIShowScreenPacket);
#endif
    return true;
}
bool PacketCapture::disable() { return true; }


} // namespace capture

LL_REGISTER_MOD(capture::PacketCapture, capture::PacketCapture::getInstance())