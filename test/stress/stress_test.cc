/*
 * @Author: modnarshen
 * @Date: 2023.01.12 16:38:00
 * @Note: Copyrights (c) 2023 modnarshen. All rights reserved.
 */
#include <nanobench.h>
#include <catch2/catch_test_macros.hpp>

#include <mt/runner.h>
#include <mt/task.h>

SCENARIO("lots of synchronous completions") {
    auto completes_synchronously = []() -> mt::Task<int> { co_return 1; };

    auto main = [&]() -> mt::Task<> {
        int sum = 0;
        for (int i = 0; i < 1'000'000; ++i) {
            sum += co_await completes_synchronously();
        }
        REQUIRE(sum == 1'000'000);
    };

    ankerl::nanobench::Bench().epochs(20).run("lots of synchronous completions", [&] { mt::run(main()); });
}

SCENARIO("sched simple test") {
    auto main = [&]() -> mt::Task<int> { co_return 1; };

    ankerl::nanobench::Bench().epochs(1'000).run("sched simple test", [&] { mt::run(main()); });
}
