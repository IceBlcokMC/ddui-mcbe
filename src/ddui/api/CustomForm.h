#pragma once
#include "ddui/Export.h"

#include <functional>
#include <memory>
#include <string>
#include <variant>
#include <vector>


class Player;

namespace ddui {

class CustomForm {
public:
    using ButtonCallback = std::function<void(Player&)>;
    using SubmitCallback = std::function<void(Player&)>;

    DDUI_API explicit CustomForm(std::string title);
    DDUI_API ~CustomForm();
    DDUI_API             CustomForm(CustomForm&&) noexcept;
    DDUI_API CustomForm& operator=(CustomForm&&) noexcept;

    DDUI_API CustomForm& label(std::string text);
    DDUI_API CustomForm& toggle(std::string id, std::string text, bool defaultValue = false);
    DDUI_API CustomForm& slider(std::string id, std::string text, double min, double max, double defaultValue = 0.0);
    DDUI_API CustomForm& button(std::string text, ButtonCallback onClick);
    DDUI_API CustomForm& spacer();
    DDUI_API CustomForm& divider();

    DDUI_API CustomForm& then(SubmitCallback cb);

private:
    struct Control {
        std::string                             type;
        std::string                             id;
        std::string                             text;
        std::variant<double, bool, std::string> value;
        ButtonCallback                          onClick;
    };

    // Getters for UIManager
    const std::string&                           getTitle() const;
    const std::vector<std::shared_ptr<Control>>& getControls() const;
    const SubmitCallback&                        getSubmitCallback() const;

    friend class UIManager;

    struct Impl;
    std::unique_ptr<Impl> mImpl;
};

} // namespace ddui