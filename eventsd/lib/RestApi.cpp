
#include <chrono>
#include <optional>
#include <boost/fusion/adapted.hpp>
#include <boost/scope_exit.hpp>

#include "Engine.h"
#include "RestApi.h"
#include "EventsMgr.h"

#include "eventsd.h"
#include "logging.h"

#include "restc-cpp/SerializeJson.h"

using namespace std;
using namespace std::string_literals;
using namespace std::chrono_literals;

BOOST_FUSION_ADAPT_STRUCT(eventsd::lib::Event,
    (string, origin)
    (std::optional<string>, msg)
    (string, category)
    );

namespace eventsd::lib {

RestApi::RestApi(const Config &config)
    : config_{config}
{

}

Response RestApi::onReqest(const Request &req)
{
    if (req.target == "/api/v1/event") {
        if (req.type == Request::Type::POST) {
            return onEvent(req);
        }

        if (req.type == Request::Type::GET) {
            return onSubscribe(req);
        }


        return {400, "Expected a POST or GET request"};
    }

    return {404, "Unknown api endpoint"};
}

Response RestApi::onEvent(const Request &req)
{

    Event event;
    event.uuid = req.uuid;

    try {
        restc_cpp::serialize_properties_t p;
        p.SetMaxMemoryConsumption(1024);
        p.ignore_unknown_properties = false;
        restc_cpp::SerializeFromJson(event, req.body, p);

        if (!event.msg) {
            return {400, "Expected an Event encoded as json. The \"msg\" property is required."};
        }
    } catch (const runtime_error& ex) {
        LOG_DEBUG << "Failed to parse json from: " << req.body;
        return {400, "Invalid payload. Expected an Event encoded as json"};
    }

    LOG_TRACE << "Received event: origin='" << event.origin
              << "', msg='" << *event.msg
              << "', category='" << event.category << "'";

    // Deliver for processing
    Engine::instance().eventsMgr().addEvent(move(event), req.owner);

    return {202, "Event accepted for delivery"};
}

Response RestApi::onSubscribe(const Request &req)
{
    uint64_t newest_seen = 0;
    deque<shared_ptr<Event>> events;
    auto timer = make_shared<boost::asio::deadline_timer>(Engine::instance().getCtx());
    mutex mx;

    Response done{400, "SSE session ended"};
    done.close = true;

    weak_ptr<boost::asio::deadline_timer> wtimer = timer;
    req.notify_connection_closed = [wtimer] {
        LOG_TRACE << "RestApi::onSubscribe - in notify_connection_closed cb";
        if (auto timer = wtimer.lock()) {
            timer->cancel();
            LOG_TRACE << "RestApi::onSubscribe - in notify_connection_closed timer cancelled";
        }
    };

    // TODO: Extract "from" query argument

    // Subscribe to new events
    int64_t sid = -1;
    sid = Engine::instance().eventsMgr().subscribe(
                req.owner,
                [&](const shared_ptr<Event>& event) {

        // Potential race condition for `sid` if the callback is caklled before
        // `subscribe` returns and assigns the appropriate value to `sid`.
        LOG_TRACE << "RestApi::onSubscribe/cb - subscription " << sid
                  << " called with event " << event->id;

        boost::system::error_code ec;

        {
            lock_guard<mutex> lock{mx};
            events.emplace_back(event);
        }

        timer->cancel_one(ec);

        if (ec) {
            LOG_DEBUG << "RestApi::onSubscribe/cb - cancel_one failed: " << ec;
        }
    });

    BOOST_SCOPE_EXIT(sid) {
        Engine::instance().eventsMgr().unsubscribe(sid);
    } BOOST_SCOPE_EXIT_END

    // Reply to client so they know we are good with SSE
    try {
        req.sse_send({});
    } catch (std::exception& ex) {
        LOG_DEBUG << "RestApi::onSubscribe failed to initiate SSE: " << ex.what();
        return done;
    }

    // TODO: (re)play the existing events

    // Wait for new events to emerge
    while(!Engine::instance().getCtx().stopped()) {
        timer->expires_from_now(boost::posix_time::seconds(10));

        boost::system::error_code ec;
        timer->async_wait(req.yield[ec]);
        if (ec && ec != boost::asio::error::operation_aborted) {
            LOG_DEBUG << "RestApi::onSubscribe/cb - async_wait failed: " << ec;
        }

        while(true) {
            assert(req.probe_connection_ok);
            if (!req.probe_connection_ok()) {
                LOG_DEBUG << "RestApi::onSubscribe/cb " << req.uuid << " SSE session ended.";
                return done;
            }

            shared_ptr<Event> event;
            {
                lock_guard<mutex> lock{mx};
                if (events.empty()) {
                    break;
                }

                event = events.front();
                events.pop_front();

                if (newest_seen >= event->id) {
                    continue; // Already sent this
                }
                newest_seen = event->id;
            }

            assert(event);
            ostringstream json;
            restc_cpp::SerializeToJson(*event, json);
            const auto sse =
                    "event: "s + event->category
                    + "\nid: " + to_string(event->id)
                    + "\ndata: "s + json.str()
                    + "\n\n";
            assert(req.sse_send);

            try {
                req.sse_send(sse);
            } catch (std::exception& ex) {
                LOG_DEBUG << "RestApi::onSubscribe failed to send sse: " << ex.what();
                return done;
            }
        }
    }

    // At this point we are shutting down with no active asio context
    return done;
}

} // ns
