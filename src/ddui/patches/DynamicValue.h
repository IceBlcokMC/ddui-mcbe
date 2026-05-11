#pragma once
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>


namespace cereal {

struct NullType {};

class DynamicValue {
public:
    enum class Type : int {
        Null    = 0,
        Boolean = 1,
        Integer = 2,
        Number  = 3,
        String  = 4,
        Array   = 5,
        Object  = 6,
    };

    union Storage {
        NullType                                      mNull;
        bool                                          mBool;
        int64_t                                       mInt;
        double                                        mDouble;
        std::string                                   mString;
        std::vector<DynamicValue>                     mArray;
        std::unordered_map<std::string, DynamicValue> mObject;

        Storage() : mNull() {}
        ~Storage() {}
    } mStorage;

    Type mType;
    int  _pad;

public:
    DynamicValue() : mType(Type::Null) {}
    ~DynamicValue() { _cleanup(); }

    DynamicValue(DynamicValue&& other) noexcept : mType(Type::Null) { *this = std::move(other); }

    DynamicValue& operator=(DynamicValue&& other) noexcept {
        if (this != &other) {
            _cleanup();
            mType = other.mType;
            switch (mType) {
            case Type::Boolean:
                mStorage.mBool = other.mStorage.mBool;
                break;
            case Type::Integer:
                mStorage.mInt = other.mStorage.mInt;
                break;
            case Type::Number:
                mStorage.mDouble = other.mStorage.mDouble;
                break;
            case Type::String:
                new (&mStorage.mString) std::string(std::move(other.mStorage.mString));
                break;
            case Type::Array:
                new (&mStorage.mArray) std::vector<DynamicValue>(std::move(other.mStorage.mArray));
                break;
            case Type::Object:
                new (&mStorage.mObject)
                    std::unordered_map<std::string, DynamicValue>(std::move(other.mStorage.mObject));
                break;
            default:
                break;
            }
            other.mType = Type::Null;
        }
        return *this;
    }

    bool isNull() const { return mType == Type::Null; }
    Type getType() const { return mType; }

    bool& asBool() {
        if (mType != Type::Boolean) throw std::runtime_error("bad variant access");
        return mStorage.mBool;
    }

    int64_t& asInteger() {
        if (mType != Type::Integer) throw std::runtime_error("bad variant access");
        return mStorage.mInt;
    }

    double& asNumber() {
        if (mType != Type::Number) throw std::runtime_error("bad variant access");
        return mStorage.mDouble;
    }

    std::string& asString() {
        if (mType != Type::String) throw std::runtime_error("bad variant access");
        return mStorage.mString;
    }

    std::vector<DynamicValue>& asArray() {
        if (mType != Type::Array) throw std::runtime_error("bad variant access");
        return mStorage.mArray;
    }

    std::unordered_map<std::string, DynamicValue>& asObject() {
        if (mType != Type::Object) throw std::runtime_error("bad variant access");
        return mStorage.mObject;
    }

private:
    void _cleanup() {
        switch (mType) {
        case Type::String:
            mStorage.mString.~basic_string();
            break;
        case Type::Array:
            mStorage.mArray.~vector();
            break;
        case Type::Object:
            mStorage.mObject.~unordered_map();
            break;
        default:
            break;
        }
        mType = Type::Null;
    }
};

static_assert(sizeof(DynamicValue) == 72, "DynamicValue size mismatch with Bedrock Server!");

} // namespace cereal