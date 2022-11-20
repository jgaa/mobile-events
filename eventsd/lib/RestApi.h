#pragma once

//#include "gtest/gtest_prod.h"
#include "HttpServer.h"
#include "logging.h"

namespace eventsd::lib {

class RestApi : public RequestHandler
{
public:
    RestApi(const Config& config);

    Response onReqest(const Request &req) override;

private:
    Response onEvent(const Request &req);
    Response onSubscribe(const Request &req);
    const Config& config_;
};

} // ns
