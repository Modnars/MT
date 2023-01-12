/*
 * @Author: modnarshen
 * @Date: 2023.01.11 11:45:11
 * @Note: Copyrights (c) 2023 modnarshen. All rights reserved.
 */
#ifndef _MT_NET_ADDRINFO_GUARD_H
#define _MT_NET_ADDRINFO_GUARD_H 1

#include <netdb.h>

namespace mt {

struct AddrinfoGuard {
    AddrinfoGuard(::addrinfo *info) : info_(info) { }
    ~AddrinfoGuard() { ::freeaddrinfo(info_); }

private:
    ::addrinfo *info_{nullptr};
};

}  // namespace mt

#endif  // _MT_NET_ADDRINFO_GUARD_H
