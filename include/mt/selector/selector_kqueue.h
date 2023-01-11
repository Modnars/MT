/*
 * @Author: modnarshen
 * @Date: 2023.01.05 15:58:18
 * @Note: Copyrights (c) 2023 modnarshen. All rights reserved.
 */
#ifndef _MT_SELECTOR_KQUEUE_H
#define _MT_SELECTOR_KQUEUE_H 1

#include <sys/event.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <cerrno>
#include <cstdio>
#include <vector>

#include <mt/selector/event.h>

namespace mt {

struct SelectorKQueue {
    SelectorKQueue() : kq_(kqueue()) {
        if (kq_ < 0) {
            ::perror("kqueue create");
            throw;
        }
        events_.resize(1UL);
    }
    ~SelectorKQueue() {
        if (kq_ > 0) {
            ::close(kq_);
        }
    }

    std::vector<Event> select(std::size_t time_out /* ms */) {
        errno = 0;
        ::timespec ts{.tv_nsec = static_cast<long>(time_out * 1'000'000)};
        int ndfs = ::kevent(kq_, nullptr, 0, events_.data(), events_.size(), &ts);
        // TODO modnarshen fill the event list
        std::vector<Event> events;
        return events;
    }

private:
    int kq_;
    std::vector<struct kevent> events_;
};

}  // namespace mt

#endif  // _MT_SELECTOR_KQUEUE_H
