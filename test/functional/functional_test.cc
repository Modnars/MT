/*
 * @Author: modnarshen
 * @Date: 2023.06.09 15:54:05
 * @Note: Copyrights (c) 2023 modnarshen. All rights reserved.
 */
#include <algorithm>
#include <chrono>
#include <coroutine>
#include <functional>
#include <iostream>
#include <random>
#include <vector>

#include <fmt/core.h>
#include <fmt/format.h>

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

int main() {
    fmt::print(">>> TEST BEGIN <<<\n");
    test_ranges_oper();
    test_single_pattern_destructor();
    fmt::print(">>> TEST END <<<\n");
    return 0;
}
