//
// Created by zhsyourai on 4/27/17.
//

#ifndef JARVIS_IFLYTEKTTS_H
#define JARVIS_IFLYTEKTTS_H


#include "../TTS.h"

class IflytekTTS : public TTS {
public:
    IflytekTTS(const std::function<void(const char *, unsigned int)> &on_result,
               const std::function<void()> &on_speech_begin,
               const std::function<void()> &on_speech_end, const std::function<void(int)> &on_error);

    IflytekTTS(const std::function<void(const char *, unsigned int)> &on_result,
               const std::function<void(int)> &on_error);

    int initialize() override;

    int uninitialize() override;

    int start() override;

    int process(std::string text) override;

    int end() override;

private:
    std::string login_params;
    const char *sessionID;
    unsigned int speed = 50;
    unsigned int volume = 50;
    unsigned int pitch = 50;
};


#endif //JARVIS_IFLYTEKTTS_H
