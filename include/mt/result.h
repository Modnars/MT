/*
 * @Author: modnarshen
 * @Date: 2023.01.06 15:09:01
 * @Note: Copyrights (c) 2022 modnarshen. All rights reserved.
 */
#ifndef _MT_RESULT_H
#define _MT_RESULT_H 1

#include <optional>
#include <variant>

#include <mt/exception.h>

namespace mt {

template <typename _Tp>
struct Result {
public:
    constexpr bool has_value() const noexcept { return std::get_if<std::monostate>(&result_) == nullptr; }

    template <typename _Value>
    constexpr void set_value(_Value&& value) noexcept {
        // TODO modnarshen 学习一下这种写法
        result_.template emplace<_Tp>(std::forward<_Value>(value));
    }

    template <typename _Value>
    constexpr void return_value(_Value&& value) noexcept {
        return set_value(std::forward<_Value>(value));
    }

    constexpr _Tp result() & {
        if (auto* exception = std::get_if<std::exception_ptr>(&result_)) {
            std::rethrow_exception(*exception);
        }
        if (auto* res = std::get_if<_Tp>(&result_)) {
            return *res;
        }
        throw ExceptionNoResult{};
    }
    constexpr _Tp result() && {
        if (auto* exception = std::get_if<std::exception_ptr>(&result_)) {
            std::rethrow_exception(*exception);
        }
        if (auto* res = std::get_if<_Tp>(&result_)) {
            return std::move(*res);
        }
        throw ExceptionNoResult{};
    }

    void set_exception(std::exception_ptr exception) noexcept { result_ = exception; }
    void unhandled_exception() noexcept { result_ = std::current_exception(); }

private:
    std::variant<std::monostate, _Tp, std::exception_ptr> result_;
};

template <>
struct Result<void> {
    constexpr bool has_value() const noexcept { return result_.has_value(); }

    void return_void() noexcept { result_.emplace(nullptr); }

    void result() {
        if (result_.has_value() && *result_ != nullptr) {
            std::rethrow_exception(*result_);
        }
    }

    void set_exception(std::exception_ptr exception) noexcept { result_ = exception; }
    void unhandled_exception() noexcept { result_ = std::current_exception(); }

private:
    std::optional<std::exception_ptr> result_;
};

}  // namespace mt

#endif  // _MT_RESULT_H
