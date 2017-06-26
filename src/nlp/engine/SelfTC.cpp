//
// Created by zhsyourai on 6/26/17.
//

#include <boost/regex.hpp>

#include <utils/AsyncHttpClient.h>
#include "SelfTC.h"
SelfTC::SelfTC(const std::function<void(bool)> &on_result,
               const std::function<void()> &on_speech_begin,
               const std::function<void()> &on_speech_end,
               const std::function<void(int)> &on_error) : TextClassify(on_result,
                                                                        on_speech_begin,
                                                                        on_speech_end,
                                                                        on_error) {}

SelfTC::SelfTC(const std::function<void(bool)> &on_result, const std::function<void(int)> &on_error) : TextClassify(
    on_result,
    on_error) {}

int SelfTC::initialize() {
  io_service = new boost::asio::io_service;
  return 0;
}

int SelfTC::uninitialize() {
  delete io_service;
  io_service = nullptr;
  return 0;
}

int SelfTC::start() {
  if (_on_begin)
    _on_begin();
  return 0;
}

int SelfTC::process(std::string data) {
  std::stringstream ss;
  ss << data;
  AsyncHttpClient c(*io_service, "106.75.72.75", AsyncHttpClient::POST, "/service", ss.str(),
                    AsyncHttpClient::URLENCODE,
                    [&](std::string error) {
                      _on_error(-1);
                    },
                    [&](unsigned int code, std::vector<std::string> headers, std::string content) {
                      if (code == 200) {
                        boost::smatch mat;
                        boost::regex reg("\\[\\(d+)\\]");
                        boost::regex_match(content, mat, reg);
                        if (mat[1].matched)
                          _on_result(mat[1].str() == "true");
                      } else {
                        _on_error(-1);
                      }
                    });
  io_service->run();
  return 0;
}

int SelfTC::end() {
  if (_on_end)
    _on_end();
  return 0;
}
