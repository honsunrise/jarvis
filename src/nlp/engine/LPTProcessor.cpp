//
// Created by zhsyourai on 4/17/17.
//

#include "LPTProcessor.h"
#include "json.hpp"

// for convenience
using json = nlohmann::json;

LPTProcessor::LPTProcessor(const std::function<void(std::vector<CONLL> conll)> &on_result,
                           const std::function<void()> &on_speech_begin, const std::function<void()> &on_speech_end,
                           const std::function<void(int)> &on_error) : NLP(on_result, on_speech_begin, on_speech_end,
                                                                           on_error) {}


LPTProcessor::LPTProcessor(const std::function<void(std::vector<CONLL> conll)> &on_result,
                           const std::function<void(int)> &on_error) : NLP(on_result, on_error) {}

int LPTProcessor::initialize() {
    api_key = "O111T730w3BZaUJshXbcFcuQrQuiwSBD5Bjl2cBe";
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
    ss << "pattern=all&format=json";
    ss << "&api_key=" << api_key;
    ss << "&text=" << data;
    AsyncHttpClient c(*io_service, "api.ltp-cloud.com", AsyncHttpClient::POST, "/analysis/", ss.str(),
                      AsyncHttpClient::URLENCODE,
                      [&](std::string error) {
                          _on_error(-1);
                      },
                      [&](unsigned int code, std::vector<std::string> headers, std::string content) {
                          if (code == 200) {
                              auto j = json::parse(content);
                              j = j[0][0];
                              std::vector<CONLL> conlls;
                              for (auto &k : j) {
                                  CONLL conll;
                                  conll.id = k["id"];
                                  conll.pos = k["pos"];
                                  conll.ne = k["ne"];
                                  conll.content = k["cont"];
                                  conll.parent = k["parent"];
                                  conll.relate = k["relate"];
                                  for (auto &l : k["arg"]) {
                                      CONLL_PREDICATE conll_predicate;
                                      conll_predicate.id = l["id"];
                                      conll_predicate.type = l["type"];
                                      conll_predicate.begin = l["beg"];
                                      conll_predicate.end = l["end"];
                                      conll.arg.push_back(conll_predicate);
                                  }
                                  conlls.push_back(conll);
                              }
                              _on_result(conlls);
                          } else {
                              _on_error(-1);
                          }
                      });
    io_service->run();
    return 0;
}

int LPTProcessor::end() {
    if (_on_end)
        _on_end();
    return 0;
}

