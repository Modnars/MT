/*
 * @Author: modnarshen
 * @Date: 2023.01.05 15:58:07
 * @Note: Copyrights (c) 2023 modnarshen. All rights reserved.
 */
#ifndef _MT_SELECTOR_SELECTOR_EPOLL_H
#define _MT_SELECTOR_SELECTOR_EPOLL_H 1

#include <sys/epoll.h>
#include <unistd.h>
#include <vector>
#include <iostream>

#include <mt/selector/event.h>
#include <mt/util/macros.h>

namespace mt {

struct SelectorEpoll {
public:
    SelectorEpoll() : epfd_(::epoll_create1(0)) {
        if (epfd_ < 0) {
            ::perror("epoll_create1");
            throw;
        }
    }
    ~SelectorEpoll() {
        if (epfd_ > 0) {
            ::close(epfd_);
        }
    }

    std::vector<Event> select(int time_out /* ms */) {
        errno = 0;
        std::vector<::epoll_event> events;
        events.resize(registered_event_count_);
        int ndfs = ::epoll_wait(epfd_, events.data(), registered_event_count_, time_out);
        std::vector<Event> result;
        if (ndfs == -1) [[unlikely]] {
            std::cerr << "error occured|errno:" << errno << std::endl;
            return result;
        }
        for (std::size_t i = 0; i < ndfs; ++i) {
            result.emplace_back(Event{.handle_info = *reinterpret_cast<HandleInfo *>(events[i].data.ptr)});
        }
        return result;
    }

    bool is_stop() { return registered_event_count_ == 1; }

    void register_event(const Event &event) {
        ::epoll_event ev{.events = event.events, .data{.ptr = const_cast<HandleInfo *>(&event.handle_info)}};
        COND_RET(::epoll_ctl(epfd_, EPOLL_CTL_ADD, event.fd, &ev) != 0);
        ++registered_event_count_;
    }

    void remove_event(const Event &event) {
        ::epoll_event ev{.events = event.events};
        COND_RET(::epoll_ctl(epfd_, EPOLL_CTL_DEL, event.fd, &ev) != 0);
        --registered_event_count_;
    }

private:
    int epfd_;
    int registered_event_count_{1};
};

}  // namespace mt

#endif  // _MT_SELECTOR_SELECTOR_EPOLL_H
