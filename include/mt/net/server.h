/*
 * @Author: modnarshen
 * @Date: 2023.01.11 17:28:24
 * @Note: Copyrights (c) 2023 modnarshen. All rights reserved.
 */
#ifndef _MT_NET_SERVER_H
#define _MT_NET_SERVER_H 1

#include <sys/types.h>
#include <list>
#include <utility>

#include <mt/concept/awaitable.h>
#include <mt/net/addrinfo_guard.h>
#include <mt/net/stream.h>
#include <mt/scheduled_task.h>
#include <mt/selector/event.h>
#include <mt/util/macros.h>
#include <mt/util/noncopyable.h>

namespace mt {
namespace concepts {
template <typename _ConnectCallBack>
concept ConnectCallBack = requires(_ConnectCallBack call_back) {
                              { call_back(std::declval<Stream>()) } -> concepts::Awaiter;
                          };

}  // namespace concepts

constexpr static std::size_t MAX_CONNECT_COUNT = 16UL;

template <concepts::ConnectCallBack _ConnectCallBack>
struct Server : NonCopyable {
public:
    Server(_ConnectCallBack call_back, int fd) : connect_call_back_(call_back), fd_(fd) { }
    Server(Server &&other) : connect_call_back_(other.connect_call_back_), fd_(std::exchange(other.fd_, -1)) { }
    ~Server() { close(); }

    Task<void> serve() {
        Event event{.fd = fd_, .events = EPOLLIN};
        auto &loop = get_event_loop();
        std::list<ScheduledTask<Task<>>> connected;
        while (true) {
            co_await loop.wait_event(event);
            ::sockaddr_storage remote_addr{};
            ::socklen_t addrlen = sizeof(remote_addr);
            int client_fd = ::accept(fd_, reinterpret_cast<::sockaddr *>(&remote_addr), &addrlen);
            COND_EXP(client_fd == -1, continue);
            connected.emplace_back(scheduled_task(connect_call_back_(Stream{client_fd, remote_addr})));
            // garbage collect
            clean_connected(connected);
        }
    }

private:
    void clean_connected(std::list<ScheduledTask<Task<>>> &connected) {
        COND_LIKELY_RET(connected.size() < 100);
        for (auto iter = connected.begin(); iter != connected.end();) {
            if (iter->done()) {
                iter = connected.erase(iter);
            } else {
                ++iter;
            }
        }
    }

private:
    void close() {
        COND_EXP(fd_ > 0, ::close(fd_));
        fd_ = -1;
    }

private:
    [[no_unique_address]] _ConnectCallBack connect_call_back_;
    int fd_{-1};
};

template <concepts::ConnectCallBack _ConnectCallBack>
Task<Server<_ConnectCallBack>> start_server(_ConnectCallBack call_back, std::string_view ip, std::uint16_t port) {
    ::addrinfo hints{.ai_family = AF_UNSPEC, .ai_socktype = SOCK_STREAM};
    ::addrinfo *server_info = nullptr;
    auto service = std::to_string(port);
    // TODO modnarshen getaddrinfo is a blocking api
    COND_EXP(int ret = ::getaddrinfo(ip.data(), service.c_str(), &hints, &server_info);
             ret != 0, throw std::system_error(std::make_error_code(std::errc::address_not_available)));
    AddrinfoGuard guard(server_info);

    int svr_fd = -1;
    for (auto p = server_info; p != nullptr; p = p->ai_next) {
        COND_EXP((svr_fd = ::socket(p->ai_family, p->ai_socktype | SOCK_NONBLOCK, p->ai_protocol)) == -1, continue);
        int succ = 1;
        // lose the pesky "address already in use" error message
        ::setsockopt(svr_fd, SOL_SOCKET, SO_REUSEADDR, &succ, sizeof(succ));
        COND_EXP(::bind(svr_fd, p->ai_addr, p->ai_addrlen) == 0, break);
        ::close(svr_fd);
        svr_fd = -1;
    }
    COND_EXP(svr_fd == -1, throw std::system_error(std::make_error_code(std::errc::address_not_available)));
    COND_EXP(::listen(svr_fd, MAX_CONNECT_COUNT) == -1,
             throw std::system_error(std::make_error_code(static_cast<std::errc>(errno))));

    co_return Server{call_back, svr_fd};
}

}  // namespace mt

#endif  // _MT_NET_SERVER_H
