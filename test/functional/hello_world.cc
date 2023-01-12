/*
 * @Author: modnarshen
 * @Date: 2023.01.12 15:24:59
 * @Note: Copyrights (c) 2023 modnarshen. All rights reserved.
 */
#include <string>
#include <string_view>

#include <fmt/core.h>

#include <mt/runner.h>
#include <mt/task.h>

mt::Task<std::string_view> hello() {
    co_return "hello";
}

mt::Task<std::string_view> world() {
    co_return "world";
}

mt::Task<std::string> hello_world() {
    co_return fmt::format("{} {}", co_await hello(), co_await world());
}

int main(int argc, char *argv[]) {
    fmt::print("RESULT: {}\n", mt::run(hello_world()));
    return 0;
}
