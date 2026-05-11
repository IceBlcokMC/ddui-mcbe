#pragma once
#include "FormBase.h"
#include "Observable.h"
#include "ddui/Export.h"

#include <functional>
#include <memory>
#include <optional>
#include <string>

class Player;

namespace ddui {

class MessageBox : public Form {
public:
    using Callback =
        std::function<void(Player&, std::optional<int> selection, std::optional<FormCancelReason> cancelReason)>;

    DDUI_API explicit MessageBox(ObservableValue<std::string> title);
    DDUI_API ~MessageBox() override;
    DDUI_API             MessageBox(MessageBox&&) noexcept;
    DDUI_API MessageBox& operator=(MessageBox&&) noexcept;

    DDUI_API MessageBox& setContent(ObservableValue<std::string> content);
    DDUI_API MessageBox&
    appendButton1(ObservableValue<std::string> label, ObservableValue<std::string> tooltip = std::string(""));
    DDUI_API MessageBox&
    appendButton2(ObservableValue<std::string> label, ObservableValue<std::string> tooltip = std::string(""));

    DDUI_API bool sendTo(Player& player, Callback callback = {});
    DDUI_API bool sendUpdate(Player& player, Callback callback = {});

private:
    friend class UIManager;
    struct Impl;
    std::unique_ptr<Impl> mImpl;

    const ObservableValue<std::string>& getTitle() const;
    const ObservableValue<std::string>& getBody() const;
    const ObservableValue<std::string>& getBtn1Label() const;
    const ObservableValue<std::string>& getBtn1Tooltip() const;
    const ObservableValue<std::string>& getBtn2Label() const;
    const ObservableValue<std::string>& getBtn2Tooltip() const;
};

} // namespace ddui