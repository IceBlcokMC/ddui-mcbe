#include "PacketCapture.h"

#include "ll/api/Expected.h"
#include "ll/api/mod/NativeMod.h"
#include "ll/api/mod/RegisterHelper.h"

#include "fmt/base.h"
#include "nlohmann/json.hpp"
#include "nlohmann/json_fwd.hpp"
#include "nlohmann/ordered_map.hpp"

#include "ll/api/memory/Hook.h"
#include "ll/api/reflection/Serialization.h"
#include "ll/api/service/Bedrock.h"

#include "mc/deps/core/utility/pub_sub/thread_model/SingleThreaded.h"
#include "mc/network/MinecraftPacketIds.h"
#include "mc/network/NetworkSystem.h"
#include "mc/network/Packet.h"
#include "mc/network/packet/ClientboundDataDrivenUICloseScreenPacket.h"
#include "mc/network/packet/ClientboundDataDrivenUIReloadPacket.h"
#include "mc/network/packet/ClientboundDataDrivenUIShowScreenPacket.h"
#include "mc/network/packet/ClientboundDataStorePacket.h"
#include "mc/network/packet/ServerboundDataDrivenScreenClosedPacket.h"
#include "mc/network/packet/ServerboundDataStorePacket.h"
#include "mc/scripting/data_sync/DDUI.h"
#include "mc/scripting/data_sync/DataStoreRemoval.h"
#include "mc/scripting/data_sync/DataStoreSyncServer.h"
#include "mc/scripting/data_sync/DataStoreUpdate.h"
#include "mc/server/ServerPlayer.h"
#include "mc/world/level/Level.h"

#include "../src/ddui/patches/DataStoreChange.h"
#include "../src/ddui/patches/DynamicValue.h"

#include <type_traits>
#include <variant>


#ifdef LL_PLAT_S
#include "mc/network/ServerNetworkHandler.h"
#endif

#ifdef LL_PLAT_C
#include "mc/client/network/ClientNetworkHandler.h"
#endif

namespace cereal {

template <class J, class T>
inline ll::Expected<J> serialize_impl(T&& val, ll::meta::PriorityTag<5>)
    requires(std::is_same_v<std::remove_cvref_t<T>, cereal::DynamicValue>)
{
    using Type = cereal::DynamicValue::Type;
    auto& v    = const_cast<std::remove_cvref_t<T>&>(val);

    try {
        switch (v.getType()) {
        case Type::Null:
            return J();

        case Type::Boolean:
            return J(v.asBool());

        case Type::Integer:
            return J(v.asInteger());

        case Type::Number:
            return J(v.asNumber());

        case Type::String:
            return J(v.asString());

        case Type::Array: {
            auto res = J::array();
            for (auto& item : v.asArray()) {
                auto sub = ll::reflection::serialize<J>(item);
                if (!sub) return ll::forwardError(sub.error());
                res.push_back(std::move(*sub));
            }
            return res;
        }

        case Type::Object: {
            auto res = J::object();
            for (auto& [key, value] : v.asObject()) {
                auto sub = ll::reflection::serialize<J>(value);
                if (!sub) return ll::forwardError(sub.error());
                res[key] = std::move(*sub);
            }
            return res;
        }
        default:
            return J();
        }
    } catch (...) {
        return ll::makeExceptionError();
    }
}

} // namespace cereal

namespace Bedrock::DDUI {

template <typename>
inline constexpr bool is_variant_v = false;

template <typename... Ts>
inline constexpr bool is_variant_v<std::variant<Ts...>> = true;

template <class J, class... Ts>
inline ll::Expected<J> serialize_impl(std::variant<Ts...>&& val, ll::meta::PriorityTag<5>) {
    using type = std::variant<Ts...>;
    return std::visit(
        [](auto&& arg) -> ll::Expected<J> { return ll::reflection::serialize<J>(arg); },
        std::forward<type>(val)
    );
}
template <class J, class T>
inline ll::Expected<J> serialize_impl(T&& val, ll::meta::PriorityTag<5>)
    requires(is_variant_v<std::remove_cvref_t<T>>)
{
    return std::visit(
        [](auto&& arg) -> ll::Expected<J> { return ll::reflection::serialize<J>(arg); },
        std::forward<T>(val)
    );
}

template <class J, class T>
inline ll::Expected<J> serialize_impl(T&& val, ll::meta::PriorityTag<5>)
    requires(std::is_same_v<std::remove_cvref_t<T>, Bedrock::DDUI::DataStoreUpdate>)
{
    auto& unConst = const_cast<Bedrock::DDUI::DataStoreUpdate&>(val);

    auto obj                    = J::object();
    obj["mDataStoreName"]       = unConst.mDataStoreName.get();
    obj["mProperty"]            = unConst.mProperty.get();
    obj["mPropertyUpdateCount"] = unConst.mPropertyUpdateCount;
    obj["mPathUpdateCount"]     = unConst.mPathUpdateCount;
    obj["mPath"]                = unConst.mPath.get();

    auto data = serialize_impl<J>(unConst.mData.get(), ll::meta::PriorityTag<5>{});
    if (!data) return ll::forwardError(data.error());
    obj["mData"] = std::move(*data);

    return obj;
}
template <class J, class T>
inline ll::Expected<J> serialize_impl(T&& val, ll::meta::PriorityTag<5>)
    requires(std::is_same_v<std::remove_cvref_t<T>, Bedrock::DDUI::DataStoreChange>)
{
    auto& unConst = const_cast<Bedrock::DDUI::DataStoreChange&>(val);

    auto obj              = J::object();
    obj["mDataStoreName"] = unConst.mDataStoreName.get();
    obj["mProperty"]      = unConst.mProperty.get();
    obj["mUpdateCount"]   = unConst.mUpdateCount;

    auto data = ll::reflection::serialize<J>(unConst.mNewData.get());
    if (!data) return ll::forwardError(data.error());
    obj["mNewData"] = std::move(*data);

    return obj;
}
template <class J, class T>
inline ll::Expected<J> serialize_impl(T&& val, ll::meta::PriorityTag<5>)
    requires(std::is_same_v<std::remove_cvref_t<T>, Bedrock::DDUI::DataStoreRemoval>)
{
    auto& unConst = const_cast<Bedrock::DDUI::DataStoreRemoval&>(val);

    auto obj              = J::object();
    obj["mDataStoreName"] = unConst.mDataStoreName.get();

    return obj;
}

template <class J, class T>
inline ll::Expected<J> serialize_impl(T&& val, ll::meta::PriorityTag<5>)
    requires(std::is_same_v<std::remove_cvref_t<T>, Bedrock::DDUI::DataStoreSync::PropertyChangePublisher>)
{
    return J("Publisher<Fn, ThreadModel, 0>"); // 由于 Publisher 是模板类，布局极其复杂，这里直接返回一个字符串
}

} // namespace Bedrock::DDUI

bool filterPacketId(Packet const& pkt) {
    auto id = pkt.getId();
    return id == MinecraftPacketIds::ClientboundDataDrivenUIShowScreen  //
        || id == MinecraftPacketIds::ClientboundDataDrivenUICloseScreen //
        || id == MinecraftPacketIds::ClientboundDataDrivenUIReload      //
        || id == MinecraftPacketIds::ClientboundDataStore               //
        || id == MinecraftPacketIds::ServerboundDataDrivenScreenClosed  //
        || id == MinecraftPacketIds::ServerboundDataStore;              //
}

std::string toDebugFriendlyJSON(std::string const& json) try {
    return nlohmann::json::parse(json).dump(2);
} catch (...) {
    return json;
}

void dump_data_store_sync(std::string_view idf) {
    auto player = ll::service::getLevel()->getPlayer("engsr6982");
    if (player) {
        auto spl  = static_cast<ServerPlayer*>(player);
        auto sync = spl->mDataStoreSync.get();

        using json_type = nlohmann::ordered_json;

        auto j = json_type::object();

        j["mUpdateableFromClient"] = ll::reflection::serialize<json_type>(sync->mUpdateableFromClient.get()).value();
        j["mDataStores"]           = ll::reflection::serialize<json_type>(sync->mDataStores.get()).value();
        j["mPropertyPathPublishers"] =
            ll::reflection::serialize<json_type>(sync->mPropertyPathPublishers.get()).value();
        j["mPropertyUpdateCount"] = ll::reflection::serialize<json_type>(sync->mPropertyUpdateCount.get()).value();
        j["mPathUpdateCount"]     = ll::reflection::serialize<json_type>(sync->mPathUpdateCount.get()).value();
        j["mOutgoingChanges"]     = ll::reflection::serialize<json_type>(sync->mOutgoingChanges.get()).value();

        fmt::print("[{}] DataStoreSync: {}\n", idf, j.dump(2));
    }
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
        auto pktName = packet.getName();
        fmt::print("[Send] {}：{}\n", pktName, toDebugFriendlyJSON(packet.toString()));

        origin(id, packet, recipientSubId);
        dump_data_store_sync(fmt::format("{}-send-after", pktName));
    } else {
        origin(id, packet, recipientSubId);
    }
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
        fmt::print("[Receive] {}：{}\n", packet.getName(), toDebugFriendlyJSON(packet.toString()));                    \
        origin(source, packet);                                                                                        \
    }

// Server side packet hook
#ifdef LL_PLAT_S
// #define CAPTURE_SERVER_RECEIVE_PACKET(PACKET) CAPTURE_RECEIVE_PACKET(ServerNetworkHandler, PACKET)

// CAPTURE_SERVER_RECEIVE_PACKET(ServerboundDataStorePacket);
// CAPTURE_SERVER_RECEIVE_PACKET(ServerboundDataDrivenScreenClosedPacket);

LL_TYPE_INSTANCE_HOOK(
    ServerboundDataStorePacketReceiveHook,
    ll::memory::HookPriority::Normal,
    ServerNetworkHandler,
    &ServerNetworkHandler::$handle,
    void,
    ::NetworkIdentifier const&          source,
    ::ServerboundDataStorePacket const& packet
) {
    fmt::print("[Receive] {}：{}\n", packet.getName(), toDebugFriendlyJSON(packet.toString()));

    dump_data_store_sync("ServerboundDataStorePacket-receive-before");
    origin(source, packet);
    dump_data_store_sync("ServerboundDataStorePacket-receive-after");
}

LL_TYPE_INSTANCE_HOOK(
    ServerboundDataDrivenScreenClosedPacketReceiveHook,
    ll::memory::HookPriority::Normal,
    ServerNetworkHandler,
    &ServerNetworkHandler::$handle,
    void,
    ::NetworkIdentifier const&                       source,
    ::ServerboundDataDrivenScreenClosedPacket const& packet
) {
    fmt::print("[Receive] {}：{}\n", packet.getName(), toDebugFriendlyJSON(packet.toString()));
    dump_data_store_sync("ServerboundDataDrivenScreenClosedPacket-receive-before");
    origin(source, packet);
    dump_data_store_sync("ServerboundDataDrivenScreenClosedPacket-receive-after");
}

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