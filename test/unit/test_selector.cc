/*
 * @Author: modnarshen
 * @Date: 2023.01.08 20:22:46
 * @Note: Copyrights (c) 2023 modnarshen. All rights reserved.
 */
#include <catch2/catch_test_macros.hpp>

#include <mt/event_loop.h>
#include <mt/selector/selector.h>

SCENARIO("test selector wait") {
    using namespace std::chrono;

    mt::EventLoop loop;
    mt::Selector selector;
    auto before_wait = loop.time();
    selector.select(300);
    auto after_wait = loop.time();
    REQUIRE(after_wait - before_wait >= 300ms);
}
