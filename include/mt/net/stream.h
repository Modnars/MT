/*
 * @Author: modnarshen
 * @Date: 2023.01.11 15:06:03
 * @Note: Copyrights (c) 2023 modnarshen. All rights reserved.
 */
#ifndef _MT_NET_STREAM_H
#define _MT_NET_STREAM_H 1

#include <netdb.h>
#include <unistd.h>
#include <cctype>
#include <utility>
#include <vector>

#include <mt/task.h>
#include <mt/util/macros.h>
#include <mt/util/noncopyable.h>

namespace mt {
struct Stream : NonCopyable {
public:
    using buffer_type = std::vector<char>;

public:
    Stream(int fd) : fd_(fd) {
        if (fd_ > 0) {
            ::socklen_t addrlen = sizeof(sock_info_);
            ::getsockname(fd_, reinterpret_cast<::sockaddr *>(&sock_info_), &addrlen);
        }
    }
    Stream(int fd, const ::sockaddr_storage &sock_info) : fd_(fd), sock_info_(sock_info) { }
    Stream(Stream &&other) : fd_(std::exchange(other.fd_, -1)), sock_info_(other.sock_info_) { }

    ~Stream() { close(); }

    void close() {
        COND_EXP(fd_ > 0, ::close(fd_));
        fd_ = -1;
    }

    Task<buffer_type> read(ssize_t sz = -1) {
        COND_EXP(sz < 0, co_return co_await read_until_eof());

        buffer_type result(sz, 0);
        Event event{.fd = fd_, .events = EPOLLIN};
        co_await get_event_loop().wait_event(event);
        sz = ::read(fd_, result.data(), result.size());
        COND_EXP(sz == -1, throw std::system_error(std::make_error_code(static_cast<std::errc>(errno))));
        result.resize(sz);
        co_return result;
    }

    Task<> write(const buffer_type &buffer) {
        Event event{.fd = fd_, .events = EPOLLOUT};
        auto &loop = get_event_loop();
        ssize_t total_write = 0;
        while (total_write < buffer.size()) {
            co_await loop.wait_event(event);
            ssize_t sz = ::write(fd_, buffer.data() + total_write, buffer.size() - total_write);
            COND_EXP(sz == -1, throw std::system_error(std::make_error_code(static_cast<std::errc>(errno))));
            total_write += sz;
        }
    }

    const sockaddr_storage &get_sock_info() const { return sock_info_; }

private:
    Task<buffer_type> read_until_eof() {
        auto &loop = get_event_loop();

        buffer_type result(chunk_size, 0);
        Event event{.fd = fd_, .events = EPOLLIN};
        int current_read = 0;
        int total_read = 0;
        do {
            co_await loop.wait_event(event);
            current_read = ::read(fd_, result.data() + total_read, chunk_size);
            COND_EXP(current_read == -1, throw std::system_error(std::make_error_code(static_cast<std::errc>(errno))));
            COND_EXP(current_read < chunk_size, result.resize(total_read + current_read));
            total_read += current_read;
            result.resize(total_read + chunk_size);
        } while (current_read > 0);
        co_return result;
    }

private:
    int fd_{-1};
    ::sockaddr_storage sock_info_{};
    constexpr static std::size_t chunk_size = 4096UL;
};

inline const void *get_in_addr(const ::sockaddr *sa) {
    COND_RET(sa->sa_family == AF_INET, &reinterpret_cast<const ::sockaddr_in *>(sa)->sin_addr);
    return &reinterpret_cast<const ::sockaddr_in6 *>(sa)->sin6_addr;
}

std::uint16_t get_in_port(const ::sockaddr *sa) {
    COND_RET(sa->sa_family == AF_INET, ::ntohs(reinterpret_cast<const ::sockaddr_in *>(sa)->sin_port));
    return ::ntohs(reinterpret_cast<const ::sockaddr_in6 *>(sa)->sin6_port);
}

}  // namespace mt

#endif  // _MT_NET_STREAM_H
