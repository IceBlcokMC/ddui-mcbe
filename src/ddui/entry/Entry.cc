#include "Entry.h"

#include "ll/api/mod/NativeMod.h"
#include "ll/api/mod/RegisterHelper.h"

#ifdef DDUI_WITH_TESTS
#include "test.h"
#endif

namespace ddui {

Entry::Entry() : mSelf(*ll::mod::NativeMod::current()) {}

ll::mod::NativeMod& Entry::getSelf() { return mSelf; }

Entry& Entry::getInstance() {
    static Entry instance;
    return instance;
}

bool Entry::load() { return true; }
bool Entry::enable() {
#ifdef DDUI_WITH_TESTS
    DDUI_TEST::init();
    getSelf().getLogger().warn("DDUI TEST ENABLED");
#endif
    return true;
}
bool Entry::disable() { return true; }


} // namespace ddui

LL_REGISTER_MOD(ddui::Entry, ddui::Entry::getInstance())