//
// Created by zhsyourai on 4/28/17.
//

#include "TulingOTT.h"
#include "json.hpp"
#include "utils/AsyncHttpClient.h"

// for convenience
using json = nlohmann::json;

TulingOTT::TulingOTT(const std::function<void(std::string)> &on_result, const std::function<void()> &on_speech_begin,
                     const std::function<void()> &on_speech_end, const std::function<void(int)> &on_error) : chat(
        on_result, on_speech_begin, on_speech_end, on_error) {}

TulingOTT::TulingOTT(const std::function<void(std::string)> &on_result, const std::function<void(int)> &on_error)
        : chat(on_result, on_error) {}

int TulingOTT::initialize() {
    api_key = "a181576d3edd4ed4800be287a11a5ae3";
    io_service = new boost::asio::io_service;
    return 0;
}

int TulingOTT::uninitialize() {
    api_key = "";
    delete io_service;
    io_service = nullptr;
    return 0;
}

int TulingOTT::start() {
    if (_on_begin)
        _on_begin();
    return 0;
}

int TulingOTT::process(std::string data) {
    json request;
    request["key"] = api_key;
    request["loc"] = "北京市中关村";
    request["userid"] = "100001";
    request["info"] = data;

    AsyncHttpClient c(*io_service, "www.tuling123.com", AsyncHttpClient::POST, "/openapi/api", request.dump(4),
                      AsyncHttpClient::JSON,
                      [&](std::string error) {
                          _on_error(-1);
                      },
                      [&](unsigned int code, std::vector<std::string> headers, std::string content) {
                          if (code == 200) {
                              auto j = json::parse(content);
                              int ret_code = j["code"];
                              if (ret_code == 100000) {
                                  _on_result(j["text"]);
                              }
                          } else {
                              _on_error(-1);
                          }
                      });
    io_service->run();
    return 0;
}

int TulingOTT::end() {
    if (_on_end)
        _on_end();
    return 0;
}
