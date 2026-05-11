#pragma once
#include "ddui/Export.h"

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>

namespace ddui {

class ObservableBase {
public:
    virtual ~ObservableBase() = default;
};

template <typename T>
class Observable : public ObservableBase {
private:
    static_assert(
        std::is_same_v<T, std::string> || std::is_same_v<T, double> || std::is_same_v<T, bool>,
        "Observable only supports std::string, double and bool"
    );

    T                                                           mValue;
    std::unordered_map<uint64_t, std::function<void(T const&)>> mServerToClientSyncCallbacks;
    uint64_t                                                    mNextId = 0;

    friend class UIManager;

    void _updateFromClient(T const& val) { mValue = val; }

    uint64_t _subscribeSync(std::function<void(T const&)> cb) {
        mServerToClientSyncCallbacks[++mNextId] = std::move(cb);
        return mNextId;
    }

    void _unsubscribeSync(uint64_t id) { mServerToClientSyncCallbacks.erase(id); }

public:
    Observable() : mValue() {}
    explicit Observable(T initialValue) : mValue(std::move(initialValue)) {}

    void set(T const& val) {
        mValue = val;
        for (auto& [id, cb] : mServerToClientSyncCallbacks) {
            cb(mValue);
        }
    }

    [[nodiscard]] T&       get() { return mValue; }
    [[nodiscard]] T const& get() const { return mValue; }

    operator T&() const { return mValue; }
    operator T const&() const { return mValue; }
};

using ObservableString  = Observable<std::string>;
using ObservableNumber  = Observable<double>;
using ObservableBoolean = Observable<bool>;

template <typename T>
class ObservableValue {
    std::variant<T, std::shared_ptr<Observable<T>>> mValue;

public:
    ObservableValue() {
        if constexpr (std::is_same_v<T, std::string>) {
            mValue = std::string("");
        } else if constexpr (std::is_same_v<T, double>) {
            mValue = 0.0;
        } else if constexpr (std::is_same_v<T, bool>) {
            mValue = false;
        }
    }

    ObservableValue(T val) : mValue(std::move(val)) {}
    ObservableValue(std::shared_ptr<Observable<T>> obs) : mValue(std::move(obs)) {}

    template <typename U = T, typename = std::enable_if_t<std::is_same_v<U, std::string>>>
    ObservableValue(const char* val) : mValue(std::string(val)) {}

    bool isObservable() const { return mValue.index() == 1; }

    std::shared_ptr<Observable<T>> getObservable() const {
        if (isObservable()) return std::get<1>(mValue);
        return nullptr;
    }

    T getValue() const {
        if (isObservable()) return std::get<1>(mValue)->get();
        return std::get<0>(mValue);
    }
};

} // namespace ddui