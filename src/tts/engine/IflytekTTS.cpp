//
// Created by zhsyourai on 4/27/17.
//

#include <sstream>
#include <boost/log/trivial.hpp>
#include "qtts.h"
#include "msp_errors.h"
#include "msp_cmn.h"
#include "IflytekTTS.h"

IflytekTTS::IflytekTTS(const std::function<void(const char *, unsigned int)> &on_result,
                       const std::function<void()> &on_speech_begin, const std::function<void()> &on_speech_end,
                       const std::function<void(int)> &on_error) : TTS(on_result, on_speech_begin, on_speech_end,
                                                                       on_error) {}

IflytekTTS::IflytekTTS(const std::function<void(const char *, unsigned int)> &on_result, const std::function<void(int)> &on_error)
        : TTS(on_result, on_error) {}

int IflytekTTS::initialize() {
    login_params = "appid = 58e74906, work_dir = .";
    int ret = MSPLogin(NULL, NULL, login_params.c_str());
    if (MSP_SUCCESS != ret) {
        BOOST_LOG_TRIVIAL(error) << "MSPLogin failed, error code:" << ret;
        MSPLogout();
        return -1;
    }
    return 0;
}

int IflytekTTS::uninitialize() {
    MSPLogout();
    return 0;
}

int IflytekTTS::start() {
    std::stringstream session_begin_params;
    session_begin_params << "voice_name = xiaoqian, text_encoding = utf8, sample_rate = 16000, rdn = 2";
    session_begin_params << ", pitch = " << pitch;
    session_begin_params << ", speed = " << speed;
    session_begin_params << ", volume = " << volume;
    BOOST_LOG_TRIVIAL(info) << session_begin_params.str();
    int ret;
    sessionID = QTTSSessionBegin(session_begin_params.str().c_str(), &ret);
    if (MSP_SUCCESS != ret) {
        BOOST_LOG_TRIVIAL(error) << "QTTSSessionBegin failed, error code:" << ret;
        return ret;
    }
    return ret;
}

int IflytekTTS::process(std::string text) {
    unsigned int audio_len = 0;
    int synth_status = MSP_TTS_FLAG_STILL_HAVE_DATA;
    int ret = QTTSTextPut(sessionID, text.c_str(), (unsigned int) text.length(), NULL);
    if (MSP_SUCCESS != ret) {
        BOOST_LOG_TRIVIAL(error) << "QTTSTextPut failed, error code:" << ret;
        QTTSSessionEnd(sessionID, "TextPutError");
        return ret;
    }
    BOOST_LOG_TRIVIAL(info) << "Start composite (" << text << "):("<< text.length() <<") ...";
    while (1) {
        /* 获取合成音频 */
        const char *data = (const char *) QTTSAudioGet(sessionID, &audio_len, &synth_status, &ret);
        if (MSP_SUCCESS != ret)
            break;
        if (MSP_TTS_FLAG_DATA_END == synth_status)
            break;
        _on_result(data, audio_len);
        usleep(100 * 1000);
    }
    if (MSP_SUCCESS != ret) {
        BOOST_LOG_TRIVIAL(error) << "QTTSAudioGet failed, error code:" << ret;
        QTTSSessionEnd(sessionID, "AudioGetError");
        return ret;
    }
    return ret;
}

int IflytekTTS::end() {
    int ret = QTTSSessionEnd(sessionID, "Normal");
    if (MSP_SUCCESS != ret) {
        BOOST_LOG_TRIVIAL(error) << "QTTSSessionEnd failed, error code:" << ret;
    }
    return ret;
}
