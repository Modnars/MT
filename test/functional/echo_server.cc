/*
 * @Author: modnarshen
 * @Date: 2023.01.12 15:29:24
 * @Note: Copyrights (c) 2023 modnarshen. All rights reserved.
 */
#include <arpa/inet.h>

#include <mt/net/server.h>
#include <mt/runner.h>

mt::Task<> handle_echo(mt::Stream stream) {
    auto &sockinfo = stream.get_sock_info();
    auto sa = reinterpret_cast<const ::sockaddr *>(&sockinfo);
    char addr[INET_ADDRSTRLEN]{};

    auto data = co_await stream.read(100);
    fmt::print("Received: '{}' from '{}:{}'\n", data.data(),
               ::inet_ntop(sockinfo.ss_family, mt::get_in_addr(sa), addr, sizeof addr), mt::get_in_port(sa));
    fmt::print("Send: '{}'\n", data.data());
    co_await stream.write(data);

    fmt::print("Close the connection.\n");
    stream.close();
}

mt::Task<void> process() {
    auto server = co_await mt::start_server(handle_echo, "127.0.0.1", 8888);

    fmt::print("Serving on 127.0.0.1:8888\n");

    co_await server.serve();
}

int main(int argc, char *argv[]) {
    mt::run(process());
    return 0;
}
