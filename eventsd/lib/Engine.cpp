#include <boost/scope_exit.hpp>

#include "Engine.h"
#include "RestApi.h"
#include "logging.h"

namespace eventsd::lib {

using namespace std;

Engine *Engine::self_;

Engine::Engine(const Config &config)
    : config_{config}, http_{config_}, events_{config_}
{
    self_ = this;
}

Engine::~Engine()
{
    assert(self_ == this);
    self_ = {};
}

void Engine::init()
{
    http_.addRoute("/api/v1", make_shared<RestApi>(config_));
}

void Engine::run()
{
    LOG_DEBUG << "Starting the HTTP server...";

    BOOST_SCOPE_EXIT(void) {
        LOG_DEBUG << "Engine is done.";
    } BOOST_SCOPE_EXIT_END

    http_future_ = http_.start();

    // Wait for the thing to stop
    http_future_.get();
}

Engine &Engine::instance()
{
    assert(self_);
    return *self_;
}



} // ns
