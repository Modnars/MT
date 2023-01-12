/*
 * @Author: modnarshen
 * @Date: 2023.01.12 15:29:31
 * @Note: Copyrights (c) 2023 modnarshen. All rights reserved.
 */
#include <string_view>
#include <chrono>

#include <fmt/core.h>

#include <mt/net/connect.h>
#include <mt/net/stream.h>
#include <mt/runner.h>
#include <mt/wait_for.h>

using namespace std::chrono_literals;

mt::Task<> echo_client(std::string_view message) {
    auto stream = co_await mt::connect("127.0.0.1", 8888);

    fmt::print("Send: '{}'\n", message);
    co_await stream.write(mt::Stream::buffer_type(message.begin(), message.end() + 1));

    auto data = co_await mt::wait_for(stream.read(100), 300ms);
    fmt::print("Received: '{}'\n", data.data());

    fmt::print("Close the connection.\n");
    stream.close();
}

int main(int argc, char *argv[]) {
    mt::run(echo_client("Hello, world!"));
    return 0;
}
