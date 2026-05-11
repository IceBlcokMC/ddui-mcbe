#pragma once
#include "FormBase.h"
#include "Observable.h"
#include "ddui/Export.h"

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

class Player;

namespace ddui {

struct DropdownItem {
    std::string label;
    int         value;
    std::string description;
};

class CustomForm : public Form {
public:
    using ButtonCallback = std::function<void(Player&)>;
    using SubmitCallback = std::function<void(Player&, std::optional<FormCancelReason>)>;

    DDUI_API explicit CustomForm(ObservableValue<std::string> title);
    DDUI_API ~CustomForm() override;
    DDUI_API             CustomForm(CustomForm&&) noexcept;
    DDUI_API CustomForm& operator=(CustomForm&&) noexcept;

    DDUI_API CustomForm& appendHeader(ObservableValue<std::string> text);
    DDUI_API CustomForm& appendLabel(ObservableValue<std::string> text);
    DDUI_API CustomForm& appendDivider();
    DDUI_API CustomForm& appendSpacer();

    DDUI_API CustomForm& appendToggle(
        ObservableValue<std::string>       label,
        std::shared_ptr<ObservableBoolean> obsValue,
        ObservableValue<std::string>       description = std::string("")
    );

    DDUI_API CustomForm& appendSlider(
        ObservableValue<std::string>      label,
        std::shared_ptr<ObservableNumber> obsValue,
        double                            min,
        double                            max,
        double                            step        = 1.0,
        ObservableValue<std::string>      description = std::string("")
    );

    DDUI_API CustomForm& appendDropdown(
        ObservableValue<std::string>      label,
        std::shared_ptr<ObservableNumber> obsValue,
        std::vector<DropdownItem>         items,
        ObservableValue<std::string>      description = std::string("")
    );

    DDUI_API CustomForm& appendTextField(
        ObservableValue<std::string>      label,
        std::shared_ptr<ObservableString> obsValue,
        ObservableValue<std::string>      description = std::string("")
    );

    DDUI_API CustomForm& appendButton(
        ObservableValue<std::string> label,
        ButtonCallback               onClick = nullptr,
        ObservableValue<std::string> tooltip = std::string("")
    );

    DDUI_API bool sendTo(Player& player, SubmitCallback callback = {});
    DDUI_API bool sendUpdate(Player& player, SubmitCallback callback = {});

private:
    friend class UIManager;

    struct Element {
        std::string                  type;
        ObservableValue<std::string> label{std::string()};
        ObservableValue<std::string> description{std::string()};
        ObservableValue<std::string> tooltip{std::string()};
        ObservableValue<std::string> text{std::string()};

        double min  = 0.0;
        double max  = 100.0;
        double step = 1.0;

        std::vector<DropdownItem> dropdownItems;

        std::shared_ptr<ObservableBase> obsValue;
        ButtonCallback                  btnCallback;
    };

    struct Impl;
    std::unique_ptr<Impl> mImpl;

    const std::vector<std::shared_ptr<Element>>& getElements() const;
    const ObservableValue<std::string>&          getTitle() const;
};

} // namespace ddui