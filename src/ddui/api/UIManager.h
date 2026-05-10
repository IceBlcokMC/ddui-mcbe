#pragma once
#include "ddui/Export.h"
#include <memory>

class ServerPlayer;

namespace ddui {

class MessageBox;
class CustomForm;

class UIManager {
public:
    DDUI_API static UIManager& getInstance();

    // 核心接口：渲染并弹窗
    DDUI_API void show(ServerPlayer& player, const MessageBox& form);
    DDUI_API void show(ServerPlayer& player, const CustomForm& form);

    // 强制把 DataStore 当前状态同步给客户端
    DDUI_API void flush(ServerPlayer& player);

private:
    DDUI_API UIManager();
    DDUI_API ~UIManager();
    UIManager(const UIManager&)            = delete;
    UIManager& operator=(const UIManager&) = delete;

    struct Impl;
    std::unique_ptr<Impl> mImpl;
};

} // namespace ddui