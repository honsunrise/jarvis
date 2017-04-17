//
// Created by zhsyourai on 4/17/17.
//

#include "LPTProcessor.h"
#include "utils/AsyncHttpClient.h"

LPTProcessor::LPTProcessor(const std::function<void(const char *, char)> &on_result,
                           const std::function<void()> &on_speech_begin, const std::function<void()> &on_speech_end,
                           const std::function<void(int)> &on_error) : NLP(on_result, on_speech_begin, on_speech_end,
                                                                           on_error) {}


LPTProcessor::LPTProcessor(const std::function<void(const char *, char)> &on_result,
                           const std::function<void(int)> &on_error) : NLP(on_result, on_error) {}

int LPTProcessor::initialize() {
    return 0;
}

int LPTProcessor::uninitialize() {
    return 0;
}

int LPTProcessor::start() {
    return 0;
}

int LPTProcessor::process(std::string data) {
    return 0;
}

int LPTProcessor::end() {
    return 0;
}

