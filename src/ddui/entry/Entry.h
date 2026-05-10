#pragma once

#include "ll/api/mod/NativeMod.h"

namespace ddui {

class Entry {
public:
    static Entry& getInstance();

    Entry();

    [[nodiscard]] ll::mod::NativeMod& getSelf();

    bool load();
    bool enable();
    bool disable();

private:
    ll::mod::NativeMod& mSelf;
};

} // namespace ddui
