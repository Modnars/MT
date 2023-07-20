/*
 * @Author: modnarshen
 * @Date: 2023.07.11 11:35:40
 * @Note: Copyrights (c) 2023 modnarshen. All rights reserved.
 */
#pragma once

#include <mt/task.h>
#include <mt/util/singleton.h>

#include "error_code.pb.h"

static const std::string SERVER_LLOG_CONF_PATH = "../../config/server_log.cfg";

struct RpcServer : public mt::Singleton<RpcServer> {
public:
    int Init() {
        stop_ = false;
        return 0;
    }

    void Stop() { stop_ = true; }

    mt::Task<> serve();

private:
    bool stop_ = true;
};
