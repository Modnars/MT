/*
 * @Author: modnarshen
 * @Date: 2023.06.09 15:54:05
 * @Note: Copyrights (c) 2023 modnarshen. All rights reserved.
 */
#include <algorithm>
#include <atomic>
#include <chrono>
#include <coroutine>
#include <cstdint>
#include <functional>
#include <iostream>
#include <random>
#include <set>
#include <unordered_map>
#include <vector>

#include <fmt/core.h>
#include <fmt/format.h>

#include <mt/runner.h>
#include <mt/task.h>
#include <mt/util/singleton.h>

template <typename _Tp>
void print(const std::vector<_Tp> &vec) {
    fmt::print("{}", fmt::join(vec, ", "));
    fmt::print("\n");
}

void test_ranges_oper() {
    auto seed = std::chrono::steady_clock::now().time_since_epoch().count();
    std::default_random_engine engine{static_cast<uint64_t>(seed)};
    std::vector<int> seq;
    static constexpr std::size_t data_num = 100UL;
    for (std::size_t i = 0UL; i != data_num; ++i) {
        seq.emplace_back(engine() % 100);
        std::ranges::push_heap(seq, std::ranges::greater{});
    }
    print(seq);
    std::vector<int> order(seq.size());
    for (std::size_t i = 0UL; i != data_num; ++i) {
        std::ranges::pop_heap(seq, std::ranges::greater{});
        order[i] = seq.back();
        seq.pop_back();
    }
    print(order);
}

struct A : public mt::Singleton<A> {
public:
    ~A() { std::cout << "call A destructor" << std::endl; }

    void foo() const { std::cout << "Hello" << std::endl; }
};

void test_single_pattern_destructor() {
    A::GetInst().foo();
}

mt::Task<> *resume_ptr = nullptr;

mt::Task<> g() {
    fmt::print("suspend coro|g\n");
    co_await std::suspend_always{};
    fmt::print("coro resume|g\n");
    co_return;
}

mt::Task<> func() {
    auto tt = g();
    resume_ptr = &tt;
    co_await tt;
    fmt::print("coro resume|func\n");
    co_return;
}

void test_coro() {
    mt::run(func());
    fmt::print("resume_ptr: {}\n", static_cast<const void *>(resume_ptr));
    if (resume_ptr)
        mt::run(*resume_ptr);
}

std::unordered_map<std::uint64_t, mt::Task<>> global_tasks;
std::set<std::uint64_t> done_tasks;

static constexpr std::uint64_t TEST_NUM = 10UL;
static constexpr std::uint64_t STEP = 50UL;

mt::Task<> call(std::uint64_t task_id) {
    fmt::print("task is call|task_id:{}\n", task_id);
    co_await std::suspend_always{};
    fmt::print("task is resume|task_id:{}\n", task_id);
    co_return;
}

void test_suspend() {
    for (std::uint64_t i = 0; done_tasks.size() != TEST_NUM; ++i) {
        std::uint64_t task_id = i % TEST_NUM;
        COND_EXP(auto iter = done_tasks.find(task_id); iter != done_tasks.end(), continue);
        if (auto iter = global_tasks.find(task_id); iter != global_tasks.end()) {
            mt::run(iter->second);
            global_tasks.erase(iter);
            done_tasks.insert(task_id);
        } else {
            auto t = call(task_id);
            mt::run(t);
            global_tasks.insert({task_id, std::move(t)});
        }
    }
}

using call_back = std::function<mt::Task<>(int)>;

mt::Task<> mainloop(call_back func) {
    int i = 0;
    while (true) {
        ++i;
        mt::run(func(i));
        COND_EXP(i >= 10, break);
    }
    co_return;
}

mt::Task<> coro(int val) {
    fmt::print("call task|val = {}\n", val);
    co_await std::suspend_always{};
    if (auto iter = global_tasks.find(9999); iter != global_tasks.end())
        mt::run(iter->second);
    co_return;
}

void test_transfer() {
    global_tasks.insert({9999, mainloop(coro)});
    if (auto iter = global_tasks.find(9999); iter != global_tasks.end())
        mt::run(iter->second);
}

void test_task_done() {
    auto func = []() -> mt::Task<int> {
        fmt::print("task start\n");
        co_await std::suspend_always{};
        fmt::print("task resume\n");
        co_return 100;
    };
    auto t = func();
    fmt::print("[1]|valid:{}|done:{}\n", t.valid(), t.valid() && t.done());
    mt::run(t);
    fmt::print("[2]|valid:{}|done:{}\n", t.valid(), t.valid() && t.done());
    mt::run(t);
    fmt::print("[3]|valid:{}|done:{}\n", t.valid(), t.valid() && t.done());
}

void inner_func() {
    auto coro = []() -> mt::Task<> {
        fmt::print("coro start\n");
        co_await std::suspend_always{};
        fmt::print("coro resume\n");
        co_return;
    };
}

std::set<int> ids_1;

using data_pair = std::pair<void *, std::string *>;
std::unordered_map<std::uint64_t, data_pair> suspend_coros;
void *global_main_addr_ = nullptr;

struct CoroAwaiter {
public:
    CoroAwaiter(std::uint64_t id, std::string *msg) : id_(id), msg_(msg) { }
    CoroAwaiter() : CoroAwaiter(0UL, nullptr) { }
    bool await_ready() const noexcept { return false; }

    bool await_suspend(std::coroutine_handle<> handle) {
        // continuation_ = handle.address();
        if (id_ == 0 && msg_ == nullptr) {
            global_main_addr_ = handle.address();
            return false;
        }
        suspend_coros.insert({id_, {handle.address(), msg_}});
        return true;
    }

    void *await_resume() noexcept { return continuation_; }

private:
    void *continuation_;
    std::uint64_t id_ = 0UL;
    std::string *msg_ = nullptr;
};

struct CurrCoroAwaiter {
public:
    bool await_ready() const noexcept { return false; }

    bool await_suspend(std::coroutine_handle<> handle) {
        continuation_ = handle.address();
        return false;
    }

    void *await_resume() noexcept { return continuation_; }

private:
    void *continuation_;
};

mt::Task<> subcoro(int id, std::string m) {
    fmt::print("enter call|id:{}|msg:{}\n", id, m);
    std::string msg = m;
    co_await CoroAwaiter{static_cast<std::uint64_t>(id), &msg};
    // std::coroutine_handle<>::from_address(global_main_addr_).resume();
    // co_await std::suspend_always{};
    fmt::print("resume coro|id:{}|msg:{}\n", id, msg);
    co_return;
}

mt::Task<> mainloop() {
    // global_main_addr_ = co_await CurrCoroAwaiter{};
    co_await CoroAwaiter{};
    fmt::print("main_addr:{}\n", global_main_addr_);
    auto t = subcoro(1, "Hello");
    mt::run(t);
    // co_await t;
    fmt::print("resume to mainloop\n");
    for (const auto &kv : suspend_coros) {
        fmt::print("{} {} {}\n", kv.first, kv.second.first, *kv.second.second);
    }
    fmt::print("valid:{} done:{}\n", t.valid(), t.done());
    // if (t.valid() && !t.done()) {
    //     // mt::run(t);
    //     co_await t;
    // }
    std::coroutine_handle<mt::Task<>::promise_type>::from_address(suspend_coros[1].first).resume();
    co_return;
}

void test_task_resume() {
    mt::run(mainloop());
}

void *point_1 = nullptr;
void *ptr_main_1 = nullptr;

struct CtxAwaiter {
    bool await_ready() const noexcept { return false; }

    bool await_suspend(std::coroutine_handle<> handle) {
        point_1 = handle.address();
        fmt::print("point1:{}\n", point_1);
        return true;
    }

    void *await_resume() noexcept { return continuation_; }

private:
    void *continuation_;
};

mt::Task<> func1() {
    fmt::print("func1\n");
    co_await CtxAwaiter{};
    fmt::print("func1 resume\n");
    co_return;
}

mt::Task<> func2() {
    fmt::print("func2\n");
    co_await func1();
    fmt::print("func2 resume\n");
    co_return;
}

mt::Task<> test_inner_call_suspend(int) {
    ptr_main_1 = co_await CurrCoroAwaiter{};
    mt::run(func2());
    std::coroutine_handle<>::from_address(point_1).resume();
    co_return;
}

void test_inner_call_suspend() {
    mt::run(test_inner_call_suspend(1));
}

// TODO modnarshen 测试挂起点

int main() {
    fmt::print(">>> TEST BEGIN <<<\n");
    // test_ranges_oper();
    // test_single_pattern_destructor();
    // test_coro();
    // test_suspend();
    // test_transfer();
    // test_task_done();
    // test_task_resume();
    test_inner_call_suspend();
    fmt::print(">>> TEST END <<<\n");
    return 0;
}
