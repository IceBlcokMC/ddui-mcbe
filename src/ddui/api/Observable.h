#pragma once
#include "ddui/Export.h"
#include <memory>
#include <string>


class ServerPlayer;

namespace ddui {

class ObservableBase {
public:
    DDUI_API ObservableBase(std::string dataStore, std::string property, std::string path);
    DDUI_API virtual ~ObservableBase();

    DDUI_API void bind(ServerPlayer& player);
    DDUI_API void forceSync(ServerPlayer& player);

protected:
    DDUI_API void syncValue(double val);
    DDUI_API void syncValue(bool val);
    DDUI_API void syncValue(const std::string& val);

private:
    struct Impl;
    std::unique_ptr<Impl> mImpl;
};

// 模板类，全部在头文件实例化，不需要导出
template <typename T>
class Observable : public ObservableBase {
private:
    T mValue;

public:
    Observable(std::string ds, std::string prop, std::string path, T initial)
    : ObservableBase(std::move(ds), std::move(prop), std::move(path)),
      mValue(std::move(initial)) {}

    void set(T const& val) {
        mValue = val;
        syncValue(mValue); // 内部实现类型分发
    }

    T const& get() const { return mValue; }
};

using ObservableString  = Observable<std::string>;
using ObservableNumber  = Observable<double>;
using ObservableBoolean = Observable<bool>;

} // namespace ddui