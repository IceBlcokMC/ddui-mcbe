#include "CustomForm.h"
#include <variant>

namespace ddui {

struct CustomForm::Impl {
    std::string                           title;
    std::vector<std::shared_ptr<Control>> controls;
    SubmitCallback                        onSubmit;
};

CustomForm::CustomForm(std::string title) : mImpl(std::make_unique<Impl>()) { mImpl->title = std::move(title); }
CustomForm::~CustomForm()                                = default;
CustomForm::CustomForm(CustomForm&&) noexcept            = default;
CustomForm& CustomForm::operator=(CustomForm&&) noexcept = default;

CustomForm& CustomForm::label(std::string text) {
    mImpl->controls.push_back(std::make_shared<Control>(Control{"label", "", std::move(text), "", nullptr}));
    return *this;
}

CustomForm& CustomForm::toggle(std::string id, std::string text, bool defaultValue) {
    mImpl->controls.push_back(
        std::make_shared<Control>(Control{"toggle", std::move(id), std::move(text), defaultValue, nullptr})
    );
    return *this;
}

CustomForm& CustomForm::slider(std::string id, std::string text, double min, double max, double defaultValue) {
    // 简化处理，实际上应通过对象传递 min, max
    mImpl->controls.push_back(
        std::make_shared<Control>(Control{"slider", std::move(id), std::move(text), defaultValue, nullptr})
    );
    return *this;
}

CustomForm& CustomForm::button(std::string text, ButtonCallback onClick) {
    mImpl->controls.push_back(
        std::make_shared<Control>(Control{"button", "", std::move(text), "", std::move(onClick)})
    );
    return *this;
}

CustomForm& CustomForm::spacer() {
    mImpl->controls.push_back(std::make_shared<Control>(Control{"spacer", "", "", "", nullptr}));
    return *this;
}

CustomForm& CustomForm::divider() {
    mImpl->controls.push_back(std::make_shared<Control>(Control{"divider", "", "", "", nullptr}));
    return *this;
}

CustomForm& CustomForm::then(SubmitCallback cb) {
    mImpl->onSubmit = std::move(cb);
    return *this;
}

const std::string&                                       CustomForm::getTitle() const { return mImpl->title; }
const std::vector<std::shared_ptr<CustomForm::Control>>& CustomForm::getControls() const { return mImpl->controls; }
const CustomForm::SubmitCallback& CustomForm::getSubmitCallback() const { return mImpl->onSubmit; }

} // namespace ddui