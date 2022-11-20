#pragma once

#include <array>
#include <deque>
#include <functional>
#include <string_view>
#include <optional>
#include <mutex>
#include <chrono>

#include <boost/asio.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/functional/hash.hpp>

#include "eventsd.h"

namespace eventsd::lib {

struct Event {
    std::string origin; // Component sendng the event
    std::optional<std::string> msg; // Message
    std::string category = "info"; // short word like: test, debug, info, warning, error, priority
    boost::uuids::uuid uuid;
    std::chrono::system_clock::time_point when = std::chrono::system_clock::now();
    uint64_t id = 0;
};

class EventsMgr
{
public:
    using subscr_id_t = int64_t;
    using onevent_cb_t = std::function<void(const std::shared_ptr<Event>& event)>;
    using events_list_t = std::deque<std::shared_ptr<Event>>;

    struct Subscriber {
        std::string owner;
        events_list_t events;
        onevent_cb_t cb;
    };

     using subscribers_t = std::unordered_map<subscr_id_t /* sibscription id */,
        std::shared_ptr<Subscriber>>;

    struct Owner {
        int64_t eid = 0; // First event will get id #1
        events_list_t events;
        std::set<std::shared_ptr<Subscriber>> subscriptions;
    };

    using event_queues_t = std::unordered_map<std::string /* owner */, Owner>;

    EventsMgr(const Config& cfg);

    void addEvent(Event && event, const std::string& owner);

    /*! Register a callback for events for an owner
     *
     *  One owner can use multiple online subscriptions, for example for different
     *  devices.
     *
     *  @param owner Unique owner identifier
     *  @param fn Callback that is called when an event is received for
     *      the owner. The callback must return quickly and never call
     *      back into EventsMgr from the same trhread, as a lock is held
     *      when the  relevant callbacks are called.
     */
    subscr_id_t subscribe(const std::string& owner, onevent_cb_t && fn);

    /*! Unsubscribe a subscription. */
    void unsubscribe(subscr_id_t id);

private:
    const Config& config_;

    event_queues_t events_;
    subscribers_t subscribers_;
    std::mutex mutex_;
};

} // ns

