//
// Created by zhsyourai on 4/17/17.
//

#include "LPTProcessor.h"
#include "json.hpp"

// for convenience
using json = nlohmann::json;

LPTProcessor::LPTProcessor(const std::function<void(int reason)> &on_result,
                           const std::function<void()> &on_speech_begin, const std::function<void()> &on_speech_end,
                           const std::function<void(int)> &on_error) : NLP(on_result, on_speech_begin, on_speech_end,
                                                                           on_error) {}


LPTProcessor::LPTProcessor(const std::function<void(int reason)> &on_result,
                           const std::function<void(int)> &on_error) : NLP(on_result, on_error) {}

int LPTProcessor::initialize() {
    api_key = "81o4m9i1s5X5B2u5w1r8cb1jljqgfkpjqQKXypmj";
    io_service = new boost::asio::io_service;
    return 0;
}

int LPTProcessor::uninitialize() {
    api_key = "";
    delete io_service;
    io_service = nullptr;
    return 0;
}

int LPTProcessor::start() {
    if (_on_begin)
        _on_begin();
    return 0;
}

int LPTProcessor::process(std::string data) {
    std::stringstream ss;
    ss << "pattern=dp&format=json";
    ss << "&api_key=" << api_key;
    ss << "&text=" << data;
    AsyncHttpClient c(*io_service, "ltpapi.voicecloud.cn", AsyncHttpClient::POST, "/analysis/", ss.str(),
                      [&](std::string error) {
                          _on_error(-1);
                      },
                      [&](unsigned int code, std::vector<std::string> headers, std::string content) {
                          auto j = json::parse(content);
                          _on_result(0);
                      });
    io_service->run();
    return 0;
}

int LPTProcessor::end() {
    if (_on_end)
        _on_end();
    return 0;
}

