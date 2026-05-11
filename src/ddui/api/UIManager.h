#pragma once
#include "CustomForm.h"
#include "MessageBox.h"
#include "ddui/Export.h"

#include <memory>

class Player;

namespace ddui {

class UIManager {
public:
    DDUI_API static UIManager& getInstance();

    bool showMessageBox(Player& player, const MessageBox& form, bool isUpdate, MessageBox::Callback callback);
    bool showCustomForm(Player& player, const CustomForm& form, bool isUpdate, CustomForm::SubmitCallback callback);

    void handleScreenClosed(Player& player);

private:
    UIManager();
    ~UIManager();
    UIManager(const UIManager&)            = delete;
    UIManager& operator=(const UIManager&) = delete;

    struct Impl;
    std::unique_ptr<Impl> mImpl;
};

} // namespace ddui