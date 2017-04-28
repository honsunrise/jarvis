//
// Created by zhsyourai on 4/13/17.
//

#ifndef JARVIS_IFLYTEKRECOGNIZER_H
#define JARVIS_IFLYTEKRECOGNIZER_H


#include "../SpeechRecognizer.h"

enum {
    IFLYTEK_STATE_INIT,
    IFLYTEK_STATE_STARTED
};

#define E_SR_NOACTIVEDEVICE		1
#define E_SR_NOMEM				2
#define E_SR_INVAL				3
#define E_SR_RECORDFAIL			4
#define E_SR_ALREADY			5
#define E_SR_NOTSTART			6

class IflytekRecognizer: public SpeechRecognizer {

protected:
    int initialize() override;

    int uninitialize() override;

public:
    IflytekRecognizer(const std::function<void(char *, bool)> &on_result,
                      const std::function<void()> &on_speech_begin, const std::function<void()> &on_speech_end,
                      const std::function<void(int)> &on_error);

    IflytekRecognizer(const std::function<void(char *, bool)> &on_result,
                      const std::function<void(int)> &on_error);

    int start() override;

    int listen(char *data, size_t len) override;

    int end() override;

private:
    void _error_happen(int errcode);

    int audio_status;
    const char *session_begin_params;
    const char* session_id;
    int state;
};


#endif //JARVIS_IFLYTEKRECOGNIZER_H
