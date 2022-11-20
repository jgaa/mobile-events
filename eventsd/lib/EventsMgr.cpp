
#include <boost/uuid/uuid.hpp>

#include "RestApi.h"

#include "eventsd.h"
#include "logging.h"

#include "EventsMgr.h"

using namespace std;
using namespace std::string_literals;

namespace eventsd::lib {

EventsMgr::EventsMgr(const Config &cfg)
    : config_{cfg}
{
}

void EventsMgr::addEvent(Event && event, const std::string& owner)
{
    {
        lock_guard<mutex> lock{mutex_};

        auto& evo =  events_[owner];

        auto e = make_shared<Event>(event);
        e->id = ++evo.eid;
        evo.events.emplace_back(e);

        LOG_TRACE << "Events[" << owner << "].size() = " << evo.events.size();

        // Todo - notify listeners
        // TODO: Not ideal to call the callbacks while holding the lock
        for(auto& subscr : evo.subscriptions) {
            assert(subscr->cb);
            subscr->cb(e);
        }
    }
}

EventsMgr::subscr_id_t EventsMgr::subscribe(const string &owner, EventsMgr::onevent_cb_t &&fn)
{
    // First subscriber gets sid 0
    static atomic_int64_t sid{-1};

    const auto this_sid = ++sid;

    {
        lock_guard<mutex> lock{mutex_};
        auto s = make_shared<Subscriber>();
        s->owner = owner;
        s->cb = move(fn);
        auto [it, _] = subscribers_.emplace(this_sid, move(s));
        events_[owner].subscriptions.emplace(it->second);
    }

    LOG_DEBUG << "Subsciber[" << owner << "] joined with sid=" << this_sid;
    return this_sid;
}


void EventsMgr::unsubscribe(EventsMgr::subscr_id_t id)
{
    LOG_DEBUG << "Removing subscription " << id;
    {
        lock_guard<mutex> lock{mutex_};

        if (auto it = subscribers_.find(id); it != subscribers_.end()) {
            // Erase the pointer (iterator) to thge subscription stored in events_
            if (auto ot = events_.find(it->second->owner); ot != events_.end()) {
                ot->second.subscriptions.erase(it->second);
            }

            subscribers_.erase(it);
        }
    }
}


} // ns
