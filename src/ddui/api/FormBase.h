#pragma once
#include "ddui/Export.h"
#include <optional>

namespace ddui {

enum class FormCancelReason { UserClosed, Timeout, Other };

class Form {
protected:
    virtual ~Form() = default;
};

} // namespace ddui