/*
 * @Author: modnarshen
 * @Date: 2023.01.11 17:10:17
 * @Note: Copyrights (c) 2023 modnarshen. All rights reserved.
 */
#ifndef _MT_NET_CONNECT_H
#define _MT_NET_CONNECT_H 1

#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <exception>
#include <system_error>

#include <mt/net/addrinfo_guard.h>
#include <mt/net/stream.h>
#include <mt/selector/event.h>
#include <mt/task.h>

namespace mt {
namespace detail {

Task<bool> do_connect(int fd, const ::sockaddr *addr, ::socklen_t len) noexcept {
    int ret = ::connect(fd, addr, len);
    COND_EXP(ret == 0, co_return true);
    COND_EXP(ret < 0 && errno != EINPROGRESS,
             throw std::system_error(std::make_error_code(static_cast<std::errc>(errno))));

    Event event{.fd = fd, .events = EPOLLOUT};
    auto &loop = get_event_loop();
    co_await loop.wait_event(event);

    int result = 0;
    ::socklen_t res_len = sizeof(result);
    // error, fail somehow, close socket
    COND_EXP(::getsockopt(fd, SOL_SOCKET, SO_ERROR, &result, &res_len) < 0, co_return false);
    co_return result == 0;
}

}  // namespace detail

Task<Stream> connect(std::string_view ip, std::uint16_t port) {
    ::addrinfo hints{.ai_family = AF_UNSPEC, .ai_socktype = SOCK_STREAM};
    ::addrinfo *server_info = nullptr;
    auto service = std::to_string(port);
    // TODO modnarshen getaddrinfo is a blocking api
    COND_EXP(int ret = ::getaddrinfo(ip.data(), service.c_str(), &hints, &server_info);
             ret != 0, throw std::system_error(std::make_error_code(std::errc::address_not_available)));

    AddrinfoGuard guard(server_info);

    int sockfd = -1;
    for (auto p = server_info; p != nullptr; p = p->ai_next) {
        COND_EXP((sockfd = ::socket(p->ai_family, p->ai_socktype | SOCK_NONBLOCK, p->ai_protocol)) == -1, continue);
        COND_EXP(co_await detail::do_connect(sockfd, p->ai_addr, p->ai_addrlen), break);
        ::close(sockfd);
        sockfd = -1;
    }
    COND_EXP(sockfd == -1, throw std::system_error(std::make_error_code(std::errc::address_not_available)));

    co_return Stream{sockfd};
}

}  // namespace mt

#endif  // _MT_NET_CONNECT_H
