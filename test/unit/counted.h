/*
 * @Author: modnarshen
 * @Date: 2023.01.08 15:13:57
 * @Note: Copyrights (c) 2023 modnarshen. All rights reserved.
 */
//
// Created by netcan on 2021/11/24.
//
#ifndef _MT_TEST_UNIT_COUNTED_H
#define _MT_TEST_UNIT_COUNTED_H 1

struct CountedPolicy {
    bool move_constructable = true;
    bool copy_constructable = true;
    bool move_assignable = true;
    bool copy_assignable = true;
};

inline constexpr CountedPolicy default_counted_policy;

template <CountedPolicy _policy = default_counted_policy>
struct Counted {
    static void reset_count() {
        move_construct_counts = 0;
        copy_construct_counts = 0;
        default_construct_counts = 0;
        copy_assign_counts = 0;
        move_assign_counts = 0;
        destruction_counts = 0;
    }

    Counted() : id(default_construct_counts++) { }

    Counted(const Counted& other) requires(_policy.copy_constructable) : id(other.id) { ++copy_construct_counts; }

    Counted(Counted&& other) requires(_policy.move_constructable) : id(other.id) {
        ++move_construct_counts;
        other.id = -1;
    }

    Counted& operator=(const Counted&) requires(_policy.copy_assignable) {
        ++copy_assign_counts;
        return *this;
    }

    Counted& operator=(Counted&& other) requires(_policy.move_assignable) {
        ++move_assign_counts;
        other.id = -1;
        return *this;
    }

    ~Counted() { ++destruction_counts; }

    static int construct_counts() { return move_construct_counts + copy_construct_counts + default_construct_counts; }
    static int alive_counts() { return construct_counts() - destruction_counts; }

    int id;
    static int move_construct_counts;
    static int copy_construct_counts;
    static int copy_assign_counts;
    static int move_assign_counts;
    static int default_construct_counts;
    static int destruction_counts;
};

template <CountedPolicy _policy>
inline int Counted<_policy>::move_construct_counts = 0;
template <CountedPolicy _policy>
inline int Counted<_policy>::copy_construct_counts = 0;
template <CountedPolicy _policy>
inline int Counted<_policy>::move_assign_counts = 0;
template <CountedPolicy _policy>
inline int Counted<_policy>::copy_assign_counts = 0;
template <CountedPolicy _policy>
inline int Counted<_policy>::default_construct_counts = 0;
template <CountedPolicy _policy>
inline int Counted<_policy>::destruction_counts = 0;

#endif  // _MT_TEST_UNIT_COUNTED_H
