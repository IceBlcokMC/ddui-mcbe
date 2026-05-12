#pragma once

#include "ll/api/mod/NativeMod.h"

namespace capture {

class PacketCapture {
public:
    static PacketCapture& getInstance();

    PacketCapture();

    [[nodiscard]] ll::mod::NativeMod& getSelf();

    bool load();
    bool enable();
    bool disable();

private:
    ll::mod::NativeMod& mSelf;
};

} // namespace ddui
