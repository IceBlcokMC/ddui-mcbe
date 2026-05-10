#pragma once
#include "ddui/Export.h"

#include <functional>
#include <memory>
#include <string>

class Player;

namespace ddui {

class MessageBox {
public:
    using Callback = std::function<void(Player&, int selection)>;

    DDUI_API explicit MessageBox(std::string title);
    DDUI_API ~MessageBox();
    DDUI_API             MessageBox(MessageBox&&) noexcept;
    DDUI_API MessageBox& operator=(MessageBox&&) noexcept;

    DDUI_API MessageBox& body(std::string b);
    DDUI_API MessageBox& button1(std::string b);
    DDUI_API MessageBox& button2(std::string b);

    DDUI_API MessageBox& then(Callback cb);

    // Getters for UIManager internal use
    DDUI_API const std::string& getTitle() const;
    DDUI_API const std::string& getBody() const;
    DDUI_API const std::string& getButton1() const;
    DDUI_API const std::string& getButton2() const;
    DDUI_API const Callback&    getCallback() const;

private:
    struct Impl;
    std::unique_ptr<Impl> mImpl;
};

} // namespace ddui