#include "MessageBox.h"
#include "UIManager.h"

namespace ddui {

struct MessageBox::Impl {
    ObservableValue<std::string> title{std::string()};
    ObservableValue<std::string> content{std::string()};
    ObservableValue<std::string> btn1Label{std::string("OK")};
    ObservableValue<std::string> btn1Tooltip{std::string("")};
    ObservableValue<std::string> btn2Label{std::string("Cancel")};
    ObservableValue<std::string> btn2Tooltip{std::string("")};
};

MessageBox::MessageBox(ObservableValue<std::string> title) : mImpl(std::make_unique<Impl>()) {
    mImpl->title = std::move(title);
}

MessageBox::~MessageBox()                                = default;
MessageBox::MessageBox(MessageBox&&) noexcept            = default;
MessageBox& MessageBox::operator=(MessageBox&&) noexcept = default;

MessageBox& MessageBox::setContent(ObservableValue<std::string> content) {
    mImpl->content = std::move(content);
    return *this;
}

MessageBox& MessageBox::appendButton1(ObservableValue<std::string> label, ObservableValue<std::string> tooltip) {
    mImpl->btn1Label   = std::move(label);
    mImpl->btn1Tooltip = std::move(tooltip);
    return *this;
}

MessageBox& MessageBox::appendButton2(ObservableValue<std::string> label, ObservableValue<std::string> tooltip) {
    mImpl->btn2Label   = std::move(label);
    mImpl->btn2Tooltip = std::move(tooltip);
    return *this;
}

bool MessageBox::sendTo(Player& player, Callback callback) {
    return UIManager::getInstance().showMessageBox(player, *this, false, std::move(callback));
}

bool MessageBox::sendUpdate(Player& player, Callback callback) {
    return UIManager::getInstance().showMessageBox(player, *this, true, std::move(callback));
}

const ObservableValue<std::string>& MessageBox::getTitle() const { return mImpl->title; }
const ObservableValue<std::string>& MessageBox::getBody() const { return mImpl->content; }
const ObservableValue<std::string>& MessageBox::getBtn1Label() const { return mImpl->btn1Label; }
const ObservableValue<std::string>& MessageBox::getBtn1Tooltip() const { return mImpl->btn1Tooltip; }
const ObservableValue<std::string>& MessageBox::getBtn2Label() const { return mImpl->btn2Label; }
const ObservableValue<std::string>& MessageBox::getBtn2Tooltip() const { return mImpl->btn2Tooltip; }

} // namespace ddui