#pragma once

#include <array>
#include <map>
#include <deque>
#include <functional>
#include <filesystem>
#include <string_view>
#include <future>

#include <boost/asio.hpp>
#include <boost/asio/buffer.hpp>

#include "eventsd.h"
#include "HttpServer.h"
#include "EventsMgr.h"

namespace eventsd::lib {

class Engine
{
public:
    Engine(const Config& config);
    ~Engine();

    void init();
    void run();

    static Engine& instance();

    EventsMgr& eventsMgr() {
        return events_;
    }

    auto& getCtx() {
        return http_.getCtx();
    }

private:
    void startWorkers();

    const Config& config_;

    // The HTTP server contains the asio context and worker threads
    static Engine *self_;
    HttpServer http_;
    std::future<void> http_future_;
    EventsMgr events_;
};


} //
