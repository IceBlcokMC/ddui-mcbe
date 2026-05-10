#include "Observable.h"

#include "mc/network/ServerNetworkSystem.h"
#include "mc/scripting/data_sync/DDUI.h"
#include "mc/scripting/data_sync/DataStoreSyncServer.h"
#include "mc/scripting/data_sync/PathQueryError.h"
#include "mc/scripting/data_sync/PathUtility.h"
#include "mc/server/ServerPlayer.h"

#include <variant>


namespace ddui {

struct ObservableBase::Impl {
    std::string   ds;
    std::string   prop;
    std::string   path;
    ServerPlayer* player = nullptr;

    void updateInternal(const std::variant<double, bool, std::string>& val) {
        if (player && player->mDataStoreSync) {
            // addToOutgoing = true, allowStringToObjectUpdates = true
            player->mDataStoreSync->setPath(ds, prop, path, val, true, true);
        }
    }
};

ObservableBase::ObservableBase(std::string ds, std::string prop, std::string path) : mImpl(std::make_unique<Impl>()) {
    mImpl->ds   = std::move(ds);
    mImpl->prop = std::move(prop);
    mImpl->path = std::move(path);
}

ObservableBase::~ObservableBase() = default;

void ObservableBase::bind(ServerPlayer& player) { mImpl->player = &player; }

void ObservableBase::forceSync(ServerPlayer& player) {
    if (player.mDataStoreSync) {
        Bedrock::DDUI::sendDataStorePacketsToClient(*player.mDataStoreSync, player.mPacketSender, nullptr);
    }
}

void ObservableBase::syncValue(double val) { mImpl->updateInternal(val); }
void ObservableBase::syncValue(bool val) { mImpl->updateInternal(val); }
void ObservableBase::syncValue(const std::string& val) { mImpl->updateInternal(val); }

} // namespace ddui