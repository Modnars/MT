/*
 * @Author: modnarshen
 * @Date: 2023.01.05 15:58:31
 * @Note: Copyrights (c) 2023 modnarshen. All rights reserved.
 */
#ifndef _MT_SELECTOR_EVENT_H
#define _MT_SELECTOR_EVENT_H 1

#include <mt/handle.h>
#include <cstdint>

namespace mt {

struct Event {
    int fd;
    std::uint32_t events;
    HandleInfo handle_info;
};

}  // namespace mt

#endif  // _MT_SELECTOR_EVENT_H
