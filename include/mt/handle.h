/*
 * @Author: modnarshen
 * @Date: 2023.01.05 11:02:35
 * @Note: Copyrights (c) 2022 modnarshen. All rights reserved.
 */
#ifndef _MT_HANDLE_H
#define _MT_HANDLE_H 1

#include <cstdint>
#include <source_location>

#include <fmt/core.h>

namespace mt {

using HandleId = std::uint64_t;

struct Handle {
public:
    enum class STATE : uint8_t {
        UNSCHEDULED,
        SCHEDULED,
        SUSPENDED,
    };

public:
    Handle() noexcept : handle_id_(++handle_id_generator_ == 0 ? ++handle_id_generator_ : handle_id_generator_) { }
    virtual ~Handle() = default;

    virtual void run() = 0;
    void set_state(STATE state) { state_ = state; }
    HandleId handle_id() { return handle_id_; }

protected:
    STATE state_{Handle::STATE::UNSCHEDULED};

private:
    HandleId handle_id_;
    static HandleId handle_id_generator_;
};

// handle maybe destroyed, using the increasing id to track the lifetime of handle.
// don't directly using a raw pointer to track coroutine lifetime,
// because a destroyed coroutine may has the same address as a new ready coroutine has created.
struct HandleInfo {
    HandleId id;
    Handle* handle;
};

struct CoroHandle : Handle {
public:
    std::string frame_name() const {
        const auto& frame_info = get_frame_info();
        return fmt::format("{}[line:{}]|{}", frame_info.file_name(), frame_info.line(), frame_info.function_name());
    }

    virtual void dump_backtrace(std::size_t depth = 0) const {};
    void schedule();
    void cancel();

private:
    virtual const std::source_location& get_frame_info() const;
};

}  // namespace mt

#endif  // _MT_HANDLE_H
