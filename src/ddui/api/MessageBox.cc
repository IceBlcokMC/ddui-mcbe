#include "MessageBox.h"

namespace ddui {

struct MessageBox::Impl {
    std::string title;
    std::string body;
    std::string btn1 = "OK";
    std::string btn2 = "Cancel";
    Callback    onResponse;
};

MessageBox::MessageBox(std::string title) : mImpl(std::make_unique<Impl>()) { mImpl->title = std::move(title); }
MessageBox::~MessageBox()                                = default;
MessageBox::MessageBox(MessageBox&&) noexcept            = default;
MessageBox& MessageBox::operator=(MessageBox&&) noexcept = default;

MessageBox& MessageBox::body(std::string b) {
    mImpl->body = std::move(b);
    return *this;
}
MessageBox& MessageBox::button1(std::string b) {
    mImpl->btn1 = std::move(b);
    return *this;
}
MessageBox& MessageBox::button2(std::string b) {
    mImpl->btn2 = std::move(b);
    return *this;
}
MessageBox& MessageBox::then(Callback cb) {
    mImpl->onResponse = std::move(cb);
    return *this;
}

const std::string&          MessageBox::getTitle() const { return mImpl->title; }
const std::string&          MessageBox::getBody() const { return mImpl->body; }
const std::string&          MessageBox::getButton1() const { return mImpl->btn1; }
const std::string&          MessageBox::getButton2() const { return mImpl->btn2; }
const MessageBox::Callback& MessageBox::getCallback() const { return mImpl->onResponse; }

} // namespace ddui