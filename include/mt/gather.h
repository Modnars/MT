/*
 * @Author: modnarshen
 * @Date: 2023.01.09 20:59:27
 * @Note: Copyrights (c) 2023 modnarshen. All rights reserved.
 */
#ifndef _MT_GATHER_H
#define _MT_GATHER_H 1

#include <stdexcept>
#include <tuple>
#include <variant>

#include <mt/concept/awaitable.h>
#include <mt/task.h>
#include <mt/util/noncopyable.h>
#include <mt/void_value.h>

namespace mt {
namespace detail {

template <typename... _Results>
struct GatherAwaiter : NonCopyable {
public:
    using result_types = std::tuple<GetTypeIfVoid_t<_Results>...>;

public:
    constexpr bool await_ready() noexcept { return is_finished(); }

    constexpr auto await_resume() const {
        if (auto *exception = std::get_if<std::exception_ptr>(&result_)) {
            std::rethrow_exception(*exception);
        }
        if (auto *res = std::get_if<result_types>(&result_)) {
            return *res;
        }
        throw std::runtime_error("result is unset");
    }

    template <typename _Promise>
    void await_suspend(std::coroutine_handle<_Promise> continuation) noexcept {
        continuation_ = &continuation.promise();
        // set continuation_ to SUSPEND, don't schedule anymore, until it resume continuation_
        continuation_->set_state(Handle::STATE::SUSPENDED);
    }

    template <concepts::Awaiter... _Futures>
    explicit GatherAwaiter(_Futures &&...futures)
        : GatherAwaiter(std::make_index_sequence<sizeof...(_Futures)>{}, std::forward<_Futures>(futures)...) { }

private:
    template <concepts::Awaiter... _Futures, std::size_t... _idxSeq>
    explicit GatherAwaiter(std::index_sequence<_idxSeq...>, _Futures &&...futures)
        : tasks_{std::make_tuple(
              collect_result<_idxSeq>(no_wait_at_initial_suspend, std::forward<_Futures>(futures))...)} { }

    template <std::size_t _idx, concepts::Awaiter _Future>
    Task<> collect_result(NoWaitAtInitialSuspend, _Future &&future) {
        try {
            auto &results = std::get<result_types>(result_);
            if constexpr (std::is_void_v<AwaitResult<_Future>>) {
                co_await std::forward<_Future>(future);
            } else {
                std::get<_idx>(results) = std::move(co_await std::forward<_Future>(future));
            }
            ++count_;
        } catch (...) {
            result_ = std::current_exception();
        }
        if (is_finished()) {
            get_event_loop().call_soon(*continuation_);
        }
    }

private:
    bool is_finished() {
        return (count_ == sizeof...(_Results) || std::get_if<std::exception_ptr>(&result_) != nullptr);
    }

private:
    std::variant<result_types, std::exception_ptr> result_;
    std::tuple<Task<std::void_t<_Results>>...> tasks_;
    CoroHandle *continuation_{};
    int count_{0};
};

template <concepts::Awaiter... _Futures>
GatherAwaiter(_Futures &&...) -> GatherAwaiter<AwaitResult<_Futures>...>;

template <concepts::Awaiter... _Futures>
struct GatherAwaiterRepository {
    explicit GatherAwaiterRepository(_Futures &&...futures) : futures_(std::forward<_Futures>(futures)...) { }

    auto operator co_await() && {
        return std::apply([]<concepts::Awaiter... _Futs>(
                              _Futs &&..._futures) { return GatherAwaiter{std::forward<_Futs>(_futures)...}; },
                          std::move(futures_));
    }

private:
    // futures_ to lift Future's lifetime
    // 1. if Future is rvalue(Fut&&), then move it to tuple(Fut)
    // 2. if Future is xvalue(Fut&&), then move it to tuple(Fut)
    // 3. if Future is lvalue(Fut&), then store as lvalue-ref(Fut&)
    std::tuple<_Futures...> futures_;
};

template <concepts::Awaiter... _Futures>  // need deduction guide to deduce future type
GatherAwaiterRepository(_Futures &&...) -> GatherAwaiterRepository<_Futures...>;

template <concepts::Awaiter... _Futures>
auto gather(NoWaitAtInitialSuspend, _Futures &&...futures)
    -> Task<std::tuple<GetTypeIfVoid_t<AwaitResult<_Futures>>...>> {
    co_return co_await GatherAwaiterRepository{std::forward<_Futures>(futures)...};
}

}  // namespace detail

template <concepts::Awaiter... _Futures>
[[nodiscard("discard gather doesn't make sense")]] auto gather(_Futures &&...futures) {
    return detail::gather(no_wait_at_initial_suspend, std::forward<_Futures>(futures)...);
}

}  // namespace mt

#endif  // _MT_GATHER_H
