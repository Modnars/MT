/*
 * @Author: modnarshen
 * @Date: 2023.06.09 15:54:05
 * @Note: Copyrights (c) 2023 modnarshen. All rights reserved.
 */
#include <algorithm>
#include <coroutine>
#include <functional>
#include <iostream>
#include <vector>
#include <random>
#include <chrono>

template <typename _Tp>
void print(const std::vector<_Tp> &vec) {
    for (const auto &v: vec)
        std::cout << v << " ";
    std::cout << std::endl;
}

int main() {
    auto seed = std::chrono::steady_clock::now().time_since_epoch().count();
    std::default_random_engine engine{static_cast<uint64_t>(seed)};
    std::vector<int> seq;
    static constexpr std::size_t data_num = 100UL;
    for (std::size_t i = 0UL; i != data_num; ++i) {
        seq.emplace_back(engine() % 100);
        std::ranges::push_heap(seq, std::ranges::greater{});
    }
    print(seq);
    for (std::size_t i = 0UL; i != data_num; ++i) {
        std::ranges::pop_heap(seq, std::ranges::greater{});
        std::cout << seq.back() <<  " ";
        seq.pop_back();
    }
    std::cout << std::endl;
    return 0;
}
