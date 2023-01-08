/*
 * @Author: modnarshen
 * @Date: 2023.01.08 16:55:31
 * @Note: Copyrights (c) 2023 modnarshen. All rights reserved.
 */
#include <catch2/catch_test_macros.hpp>

#include <mt/result.h>
#include <mt/runner.h>
#include <mt/task.h>

#include "counted.h"

SCENARIO("test Counted") {
    using TestCounted = Counted<default_counted_policy>;
    TestCounted::reset_count();
    GIVEN("move counted") {
        {
            TestCounted c1;
            TestCounted c2(std::move(c1));
            REQUIRE(TestCounted::construct_counts() == 2);
            REQUIRE(TestCounted::move_construct_counts == 1);
            REQUIRE(TestCounted::alive_counts() == 2);
        }
        REQUIRE(TestCounted::alive_counts() == 0);
    }

    GIVEN("copy counted") {
        {
            TestCounted c1;
            TestCounted c2(c1);
            REQUIRE(TestCounted::construct_counts() == 2);
            REQUIRE(TestCounted::copy_construct_counts == 1);
            REQUIRE(TestCounted::alive_counts() == 2);
        }
        REQUIRE(TestCounted::alive_counts() == 0);
    }

    GIVEN("copy counted") {
        TestCounted c1;
        {
            TestCounted c2(std::move(c1));
            REQUIRE(TestCounted::construct_counts() == 2);
            REQUIRE(TestCounted::move_construct_counts == 1);
            REQUIRE(TestCounted::alive_counts() == 2);
        }
        REQUIRE(TestCounted::alive_counts() == 1);
        REQUIRE(c1.id == -1);
    }
}

SCENARIO("test result T") {
    using TestCounted = Counted<{.move_assignable = false, .copy_assignable = false}>;
    TestCounted::reset_count();
    GIVEN("result set lvalue") {
        mt::Result<TestCounted> res;
        REQUIRE(!res.has_value());
        {
            TestCounted c;
            REQUIRE(TestCounted::construct_counts() == 1);
            REQUIRE(TestCounted::copy_construct_counts == 0);
            res.set_value(c);
            REQUIRE(TestCounted::construct_counts() == 2);
            REQUIRE(TestCounted::copy_construct_counts == 1);
        }
        REQUIRE(TestCounted::alive_counts() == 1);
        REQUIRE(res.has_value());
    }

    GIVEN("result set rvalue") {
        mt::Result<TestCounted> res;
        REQUIRE(!res.has_value());
        {
            TestCounted c;
            REQUIRE(TestCounted::construct_counts() == 1);
            REQUIRE(TestCounted::move_construct_counts == 0);
            res.set_value(std::move(c));
            REQUIRE(TestCounted::construct_counts() == 2);
            REQUIRE(TestCounted::move_construct_counts == 1);
        }
        REQUIRE(TestCounted::alive_counts() == 1);
        REQUIRE(res.has_value());
    }

    GIVEN("lvalue result") {
        mt::Result<TestCounted> res;
        res.set_value(TestCounted{});
        REQUIRE(res.has_value());
        REQUIRE(TestCounted::default_construct_counts == 1);
        REQUIRE(TestCounted::move_construct_counts == 1);
        REQUIRE(TestCounted::copy_construct_counts == 0);
        {
            {
                auto&& r = res.result();
                REQUIRE(TestCounted::default_construct_counts == 1);
                REQUIRE(TestCounted::move_construct_counts == 1);
                REQUIRE(TestCounted::copy_construct_counts == 1);
            }
            {
                auto r = res.result();
                REQUIRE(TestCounted::default_construct_counts == 1);
                REQUIRE(TestCounted::copy_construct_counts == 2);
            }
        }
        REQUIRE(TestCounted::alive_counts() == 1);
    }

    GIVEN("rvalue result") {
        mt::Result<TestCounted> res;
        res.set_value(TestCounted{});
        REQUIRE(res.has_value());
        REQUIRE(TestCounted::default_construct_counts == 1);
        REQUIRE(TestCounted::move_construct_counts == 1);
        {
            auto r = std::move(res).result();
            REQUIRE(TestCounted::move_construct_counts == 2);
            REQUIRE(TestCounted::alive_counts() == 2);
        }
        REQUIRE(TestCounted::alive_counts() == 1);
    }
}

SCENARIO("test Counted for Task") {
    using TestCounted = Counted<default_counted_policy>;
    TestCounted::reset_count();

    auto build_count = []() -> mt::Task<TestCounted> { co_return TestCounted{}; };
    bool called{false};

    GIVEN("return a counted") {
        mt::run([&]() -> mt::Task<> {
            auto c = co_await build_count();
            REQUIRE(TestCounted::alive_counts() == 1);
            REQUIRE(TestCounted::move_construct_counts == 2);
            REQUIRE(TestCounted::default_construct_counts == 1);
            REQUIRE(TestCounted::copy_construct_counts == 0);
            called = true;
        }());
        REQUIRE(called);
    }

    GIVEN("return a lvalue counted") {
        mt::run([&]() -> mt::Task<> {
            auto t = build_count();
            {
                auto c = co_await t;
                REQUIRE(TestCounted::alive_counts() == 2);
                REQUIRE(TestCounted::move_construct_counts == 1);
                REQUIRE(TestCounted::default_construct_counts == 1);
                REQUIRE(TestCounted::copy_construct_counts == 1);
            }

            {
                auto c = co_await std::move(t);
                REQUIRE(TestCounted::alive_counts() == 2);
                REQUIRE(TestCounted::move_construct_counts == 2);
                REQUIRE(TestCounted::default_construct_counts == 1);
                REQUIRE(TestCounted::copy_construct_counts == 1);
            }

            called = true;
        }());
        REQUIRE(called);
    }

    GIVEN("rvalue task: get_result") {
        auto c = mt::run(build_count());
        REQUIRE(TestCounted::alive_counts() == 1);
        REQUIRE(TestCounted::move_construct_counts == 2);
        REQUIRE(TestCounted::default_construct_counts == 1);
        REQUIRE(TestCounted::copy_construct_counts == 0);
    }

    GIVEN("lvalue task: get_result") {
        auto t = build_count();
        auto c = mt::run(t);
        REQUIRE(TestCounted::alive_counts() == 2);
        REQUIRE(TestCounted::move_construct_counts == 1);
        REQUIRE(TestCounted::default_construct_counts == 1);
        REQUIRE(TestCounted::copy_construct_counts == 1);
    }
}

SCENARIO("test pass parameters to the coroutine frame") {
    using TestCounted = Counted<{.move_assignable = false, .copy_assignable = false}>;
    TestCounted::reset_count();

    GIVEN("pass by rvalue") {
        auto coro = [](TestCounted count) -> mt::Task<> {
            REQUIRE(count.alive_counts() == 2);
            co_return;
        };
        mt::run(coro(TestCounted{}));
        REQUIRE(TestCounted::default_construct_counts == 1);
        REQUIRE(TestCounted::move_construct_counts == 1);
        REQUIRE(TestCounted::alive_counts() == 0);
    }

    GIVEN("pass by lvalue") {
        auto coro = [](TestCounted count) -> mt::Task<> {
            REQUIRE(TestCounted::copy_construct_counts == 1);
            REQUIRE(TestCounted::move_construct_counts == 1);
            REQUIRE(count.alive_counts() == 3);
            co_return;
        };
        TestCounted count;
        mt::run(coro(count));

        REQUIRE(TestCounted::default_construct_counts == 1);
        REQUIRE(TestCounted::copy_construct_counts == 1);
        REQUIRE(TestCounted::move_construct_counts == 1);
        REQUIRE(TestCounted::alive_counts() == 1);
        REQUIRE(count.id != -1);
    }

    GIVEN("pass by xvalue") {
        auto coro = [](TestCounted count) -> mt::Task<> {
            REQUIRE(TestCounted::copy_construct_counts == 0);
            REQUIRE(TestCounted::move_construct_counts == 2);
            REQUIRE(count.alive_counts() == 3);
            REQUIRE(count.id != -1);
            co_return;
        };
        TestCounted count;
        mt::run(coro(std::move(count)));

        REQUIRE(TestCounted::default_construct_counts == 1);
        REQUIRE(TestCounted::copy_construct_counts == 0);
        REQUIRE(TestCounted::move_construct_counts == 2);
        REQUIRE(TestCounted::alive_counts() == 1);
        REQUIRE(count.id == -1);
    }

    GIVEN("pass by lvalue ref") {
        TestCounted count;
        auto coro = [&](TestCounted& cnt) -> mt::Task<> {
            REQUIRE(cnt.alive_counts() == 1);
            REQUIRE(&cnt == &count);
            co_return;
        };
        mt::run(coro(count));
        REQUIRE(TestCounted::default_construct_counts == 1);
        REQUIRE(TestCounted::construct_counts() == 1);
        REQUIRE(TestCounted::alive_counts() == 1);
    }

    GIVEN("pass by rvalue ref") {
        auto coro = [](TestCounted&& count) -> mt::Task<> {
            REQUIRE(count.alive_counts() == 1);
            co_return;
        };
        mt::run(coro(TestCounted{}));
        REQUIRE(TestCounted::default_construct_counts == 1);
        REQUIRE(TestCounted::construct_counts() == 1);
        REQUIRE(TestCounted::alive_counts() == 0);
    }
}
