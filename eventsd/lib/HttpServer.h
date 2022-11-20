#pragma once

#include <map>
#include <functional>
#include <filesystem>
#include <string_view>
#include <future>

#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/uuid/uuid.hpp>

#include "eventsd.h"

namespace eventsd::lib {

boost::uuids::uuid generateUuid();

struct Request {
    enum class Type {
        GET,
        PUT,
        PATCH,
        POST,
        DELETE
    };

    boost::asio::yield_context& yield;
    std::string target;
    std::string_view route; // The part of the target that was matched by the chosen route.
    std::string auth; // from Authorization header
    std::string owner;
    std::string body;
    Type type = Type::GET;
    boost::uuids::uuid uuid = generateUuid();

    /*! Send one SSE event to the client.
     *
     *  @param sseEvent Complete and correctly formatted SSE event.
     *
     *  @exception std::exception if the operation fails.
     */
    std::function<void(std::string_view sseEvent)> sse_send;

    /*! Check if the connection is still open to the client.
     */
    std::function<bool()> probe_connection_ok;

    /*! Can be set by a handler to allow the HTTP server to notify
     *  directly when a connection is closed.
     *
     *  Muteable because Requests in general are const
     */
    mutable std::function<void()> notify_connection_closed;
};

struct Response {    
    int code = 200;
    std::string reason = "OK";
    std::string body;
    std::string target; // The actual target
    std::string_view mime_type;
    std::string_view mimeType() const;
    bool close = false;

    bool ok() const noexcept {
        return code / 100 == 2;
    }
};

class RequestHandler {
public:
    virtual ~RequestHandler() = default;

    virtual Response onReqest(const Request& req) = 0;
};

template <typename T>
class EmbeddedHandler : public RequestHandler {
public:
    EmbeddedHandler(const T& content, std::string prefix)
        : content_{content}, prefix_{std::move(prefix)} {}

    Response onReqest(const Request& req) override {
        // Remove prefix
        auto t = std::string_view{req.target};
        if (t.size() < prefix_.size()) {
            throw std::runtime_error{"Invalid targert. Cannot be shorted than prefix!"};
        }

        t = t.substr(prefix_.size());

        while(!t.empty() && t.front() == '/') {
            t = t.substr(1);
        }

        if (t.empty()) {
            t = {"index.html"};
        }

        if (auto it = content_.find(std::string{t}) ; it != content_.end()) {
            std::filesystem::path served = prefix_;
            served /= t;

            return {200, "OK", std::string{it->second}, served.string()};
        }
        return {404, "Document not found"};
    }

private:
    const T& content_;
    const std::string prefix_;
};

// Very general HTTP server so we can easily swap it out with something better later...
class HttpServer
{
public:
    using handler_t = std::shared_ptr<RequestHandler>;//std::function<Response (const Request& req)>;


    HttpServer(const Config& config);

    std::future<void> start();
    void stop();

    void addRoute(std::string_view target, handler_t handler);

    std::pair<bool, std::string_view /* user name */> Authenticate(const std::string_view& authHeader);

    // Called by the HTTP server implementation template
    Response onRequest(Request& req) noexcept;

    // Serve a directory.
    // handles `index.html` by default. Lists the directory if there is no index.html.
    class FileHandler : public RequestHandler {
    public:
        FileHandler(std::filesystem::path root);

        Response onReqest(const Request &req) override;

        std::filesystem::path resolve(std::string_view target);
    private:
        Response readFile(const std::filesystem::path& path);
        Response handleDir(const std::filesystem::path& path);
        Response listDir(const std::filesystem::path& path);
        std::string getMimeType(const std::filesystem::path& path);

        const std::filesystem::path root_;
    };

    auto& getCtx() {
        return ctx_;
    }

private:
    void startWorkers();

    const Config& config_;
    std::map<std::string, handler_t> routes_;
    boost::asio::io_context ctx_;
    std::vector<std::thread> workers_;
    std::promise<void> promise_;
};

}
