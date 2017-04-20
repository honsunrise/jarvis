//
// Created by zhsyourai on 4/17/17.
//

#ifndef JARVIS_LPTPROCESSOR_H
#define JARVIS_LPTPROCESSOR_H

#include "../NLP.h"

class LPTProcessor : public NLP {
public:
    LPTProcessor(const std::function<void(int reason)> &on_result, const std::function<void()> &on_speech_begin,
                 const std::function<void()> &on_speech_end, const std::function<void(int)> &on_error);

    LPTProcessor(const std::function<void(int reason)> &on_result, const std::function<void(int)> &on_error);

    int initialize() override;

    int uninitialize() override;

    int start() override;

    int process(std::string data) override;

    int end() override;
};


#endif //JARVIS_LPTPROCESSOR_H
