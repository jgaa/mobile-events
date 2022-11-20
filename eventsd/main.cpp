
#include <iostream>
#include <filesystem>
#include <boost/program_options.hpp>

#include "lib/eventsd.h"
#include "lib/logging.h"
#include "lib/Engine.h"

using namespace std;
using namespace eventsd;

int main(int argc, char* argv[]) {

    try {
        locale loc("");
    } catch (const std::exception&) {
        cout << "Locales in Linux are fundamentally broken. Never worked. Never will. Overriding the current mess with LC_ALL=C" << endl;
        setenv("LC_ALL", "C", 1);
    }


    Config config;
    std::string log_level = "info";

    namespace po = boost::program_options;
    po::options_description general("Options");
    po::positional_options_description positionalDescription;

    general.add_options()
        ("help,h", "Print help and exit")
        ("version", "print version string and exit")
//        ("db-path,d",
//            po::value<string>(&config.db_path)->default_value(config.db_path),
//            "Definition file to deploy")
        ("log-level,l",
             po::value<string>(&log_level)->default_value(log_level),
             "Log-level to use; one of 'info', 'debug', 'trace'")
    ;
    po::options_description http("HTTP/API server");
    http.add_options()
        ("http-endpoint,H",
            po::value<string>(&config.http_endpoint)->default_value(config.http_endpoint),
            "HTTP endpoint. For example [::] to listen to all interfaces")
        ("http-port",
            po::value<string>(&config.http_port)->default_value(config.http_port),
            "HTTP port to listen to. Not required when using port 80 or 443")
        ("http-tls-key",
            po::value<string>(&config.http_tls_key)->default_value(config.http_tls_key),
            "TLS key for the embedded HTTP server")
        ("http-tls-cert",
            po::value<string>(&config.http_tls_cert)->default_value(config.http_tls_cert),
            "TLS cert for the embedded HTTP server")
        ("http-num-threads",
            po::value<size_t>(&config.num_http_threads)->default_value(config.num_http_threads),
            "Threads for the embedded HTTP server")

        ;

    po::options_description cmdline_options;
    cmdline_options.add(general).add(http);
    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(cmdline_options).run(), vm);
    po::notify(vm);
    if (vm.count("help")) {
        std::cout << filesystem::path(argv[0]).stem().string() << " [options]";
        std::cout << cmdline_options << std::endl;
        return -1;
    }

    if (vm.count("version")) {
        std::cout << filesystem::path(argv[0]).stem().string() << ' '  << EVENTSD_VERSION << endl;
        return -2;
    }

    auto llevel = logfault::LogLevel::INFO;
    if (log_level == "debug") {
        llevel = logfault::LogLevel::DEBUGGING;
    } else if (log_level == "trace") {
        llevel = logfault::LogLevel::TRACE;
    } else if (log_level == "info") {
        ;  // Do nothing
    } else {
        std::cerr << "Unknown log-level: " << log_level << endl;
        return -1;
    }

    logfault::LogManager::Instance().AddHandler(
                make_unique<logfault::StreamHandler>(clog, llevel));

    LOG_INFO << filesystem::path(argv[0]).stem().string() << ' ' << EVENTSD_VERSION  " starting up. Log level: " << log_level;

    try {
        eventsd::lib::Engine engine{config};
        engine.init();
        engine.run();
    } catch (const exception& ex) {
        LOG_ERROR << "Caught exception from engine: " << ex.what();
    }
} // mail
