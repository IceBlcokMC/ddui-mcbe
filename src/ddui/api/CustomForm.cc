#include "CustomForm.h"
#include "UIManager.h"

namespace ddui {

struct CustomForm::Impl {
    ObservableValue<std::string>          title{std::string()};
    std::vector<std::shared_ptr<Element>> elements;
};

CustomForm::CustomForm(ObservableValue<std::string> title) : mImpl(std::make_unique<Impl>()) {
    mImpl->title = std::move(title);
}

CustomForm::~CustomForm()                                = default;
CustomForm::CustomForm(CustomForm&&) noexcept            = default;
CustomForm& CustomForm::operator=(CustomForm&&) noexcept = default;

CustomForm& CustomForm::appendHeader(ObservableValue<std::string> text) {
    auto el  = std::make_shared<Element>();
    el->type = "header";
    el->text = std::move(text);
    mImpl->elements.push_back(std::move(el));
    return *this;
}

CustomForm& CustomForm::appendLabel(ObservableValue<std::string> text) {
    auto el  = std::make_shared<Element>();
    el->type = "label";
    el->text = std::move(text);
    mImpl->elements.push_back(std::move(el));
    return *this;
}

CustomForm& CustomForm::appendDivider() {
    auto el  = std::make_shared<Element>();
    el->type = "divider";
    mImpl->elements.push_back(std::move(el));
    return *this;
}

CustomForm& CustomForm::appendSpacer() {
    auto el  = std::make_shared<Element>();
    el->type = "spacer";
    mImpl->elements.push_back(std::move(el));
    return *this;
}

CustomForm& CustomForm::appendToggle(
    ObservableValue<std::string>       label,
    std::shared_ptr<ObservableBoolean> obsValue,
    ObservableValue<std::string>       desc
) {
    auto el         = std::make_shared<Element>();
    el->type        = "toggle";
    el->label       = std::move(label);
    el->description = std::move(desc);
    el->obsValue    = std::move(obsValue);
    mImpl->elements.push_back(std::move(el));
    return *this;
}

CustomForm& CustomForm::appendSlider(
    ObservableValue<std::string>      label,
    std::shared_ptr<ObservableNumber> obsValue,
    double                            min,
    double                            max,
    double                            step,
    ObservableValue<std::string>      desc
) {
    auto el         = std::make_shared<Element>();
    el->type        = "slider";
    el->label       = std::move(label);
    el->min         = min;
    el->max         = max;
    el->step        = step;
    el->description = std::move(desc);
    el->obsValue    = std::move(obsValue);
    mImpl->elements.push_back(std::move(el));
    return *this;
}

CustomForm& CustomForm::appendDropdown(
    ObservableValue<std::string>      label,
    std::shared_ptr<ObservableNumber> obsValue,
    std::vector<DropdownItem>         items,
    ObservableValue<std::string>      description
) {
    auto el           = std::make_shared<Element>();
    el->type          = "dropdown";
    el->label         = std::move(label);
    el->obsValue      = std::move(obsValue);
    el->dropdownItems = std::move(items);
    el->description   = std::move(description);
    mImpl->elements.push_back(std::move(el));
    return *this;
}

CustomForm& CustomForm::appendTextField(
    ObservableValue<std::string>      label,
    std::shared_ptr<ObservableString> obsValue,
    ObservableValue<std::string>      desc
) {
    auto el         = std::make_shared<Element>();
    el->type        = "textfield";
    el->label       = std::move(label);
    el->description = std::move(desc);
    el->obsValue    = std::move(obsValue);
    mImpl->elements.push_back(std::move(el));
    return *this;
}

CustomForm& CustomForm::appendButton(
    ObservableValue<std::string> label,
    ButtonCallback               onClick,
    ObservableValue<std::string> tooltip
) {
    auto el         = std::make_shared<Element>();
    el->type        = "button";
    el->label       = std::move(label);
    el->tooltip     = std::move(tooltip);
    el->btnCallback = std::move(onClick);
    mImpl->elements.push_back(std::move(el));
    return *this;
}

bool CustomForm::sendTo(Player& player, SubmitCallback callback) {
    return UIManager::getInstance().showCustomForm(player, *this, false, std::move(callback));
}

bool CustomForm::sendUpdate(Player& player, SubmitCallback callback) {
    return UIManager::getInstance().showCustomForm(player, *this, true, std::move(callback));
}

const std::vector<std::shared_ptr<CustomForm::Element>>& CustomForm::getElements() const { return mImpl->elements; }
const ObservableValue<std::string>&                      CustomForm::getTitle() const { return mImpl->title; }

} // namespace ddui