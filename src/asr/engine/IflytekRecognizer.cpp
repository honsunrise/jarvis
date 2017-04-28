//
// Created by zhsyourai on 4/13/17.
//

#include <cstring>
#include <msp_types.h>
#include <qisr.h>
#include <msp_errors.h>
#include <boost/log/trivial.hpp>
#include <thread>
#include "IflytekRecognizer.h"

IflytekRecognizer::IflytekRecognizer(const std::function<void(char *, bool)> &on_result,
                                     const std::function<void()> &on_speech_begin,
                                     const std::function<void()> &on_speech_end,
                                     const std::function<void(int)> &on_error) : SpeechRecognizer(on_result,
                                                                                                  on_speech_begin,
                                                                                                  on_speech_end,
                                                                                                  on_error) {
}

IflytekRecognizer::IflytekRecognizer(const std::function<void(char *, bool)> &on_result,
                                     const std::function<void(int)> &on_error) : SpeechRecognizer(on_result,
                                                                                                  on_error) {

}

void IflytekRecognizer::_error_happen(int errcode) {
    if (session_id) {
        if (_on_error)
            _on_error(errcode);

        QISRSessionEnd(session_id, "err");
        session_id = NULL;
    }
    state = IFLYTEK_STATE_INIT;
}

int IflytekRecognizer::initialize() {
    session_begin_params =
            "sub = iat, domain = iat, language = zh_cn, "
                    "accent = mandarin, sample_rate = 16000, "
                    "result_type = plain, result_encoding = utf8";

    audio_status = MSP_AUDIO_SAMPLE_FIRST;

    return 0;
}

int IflytekRecognizer::uninitialize() {
    return 0;
}

int IflytekRecognizer::start() {
    int errcode = MSP_SUCCESS;

    if (state == IFLYTEK_STATE_STARTED) {
        BOOST_LOG_TRIVIAL(error) << "already STARTED.";
        return -E_SR_ALREADY;
    }

    session_id = QISRSessionBegin(NULL, session_begin_params, &errcode);
    if (MSP_SUCCESS != errcode) {
        BOOST_LOG_TRIVIAL(error) << "QISRSessionBegin failed! error code: " << errcode;
        return errcode;
    }

    audio_status = MSP_AUDIO_SAMPLE_FIRST;
    state = IFLYTEK_STATE_STARTED;

    if (_on_begin)
        _on_begin();
    return 0;
}

int IflytekRecognizer::listen(char *data, size_t len) {
    const char *rslt = NULL;
    int ep_stat = MSP_EP_LOOKING_FOR_SPEECH;
    int rec_stat = MSP_REC_STATUS_SUCCESS;
    int ret = 0;
    if (!data || !len)
        return 0;

    ret = QISRAudioWrite(session_id, data, len, audio_status, &ep_stat, &rec_stat);
    if (ret) {
        _error_happen(ret);
        return ret;
    }
    audio_status = MSP_AUDIO_SAMPLE_CONTINUE;

    if (MSP_REC_STATUS_SUCCESS == rec_stat) {
        rslt = QISRGetResult(session_id, &rec_stat, 0, &ret);
        if (MSP_SUCCESS != ret) {
            BOOST_LOG_TRIVIAL(error) << "QISRGetResult failed! error code: " << ret;
            _error_happen(ret);
            return ret;
        }
        if (NULL != rslt && _on_result) {
            char *str = new char[strlen(rslt) + 1];
            strcpy(str, rslt);
            std::thread([this, str, rec_stat](){_on_result(str, rec_stat == MSP_REC_STATUS_COMPLETE);}).detach();
        }
    }

    if (MSP_EP_AFTER_SPEECH == ep_stat) {
        int errcode;
        while (rec_stat != MSP_REC_STATUS_COMPLETE) {
            rslt = QISRGetResult(session_id, &rec_stat, 0, &errcode);
            if (rslt && _on_result) {
                char *str = new char[strlen(rslt) + 1];
                strcpy(str, rslt);
                std::thread([this, str, rec_stat](){_on_result(str, rec_stat == MSP_REC_STATUS_COMPLETE);}).detach();
            }

            usleep(500); /* for cpu occupy, should sleep here */
        }

        if (session_id) {
            if (_on_end)
                _on_end();
            QISRSessionEnd(session_id, "VAD Normal");
            session_id = NULL;
        }
        state = IFLYTEK_STATE_INIT;
    }

    return 0;
}

int IflytekRecognizer::end() {
    int ret = 0;
    const char *rslt = NULL;
    int ep_stat;
    int rec_stat;

    if (state != IFLYTEK_STATE_STARTED) {
        BOOST_LOG_TRIVIAL(error) << "Not started or already stopped.";
        return -E_SR_NOTSTART;
    }

    state = IFLYTEK_STATE_INIT;
    ret = QISRAudioWrite(session_id, NULL, 0, MSP_AUDIO_SAMPLE_LAST, &ep_stat, &rec_stat);
    if (ret != 0) {
        BOOST_LOG_TRIVIAL(error) << "write LAST_SAMPLE failed: " << ret;
        QISRSessionEnd(session_id, "write err");
        return ret;
    }

    while (rec_stat != MSP_REC_STATUS_COMPLETE) {
        rslt = QISRGetResult(session_id, &rec_stat, 0, &ret);
        if (MSP_SUCCESS != ret) {
            BOOST_LOG_TRIVIAL(error) << "QISRGetResult failed! error code: " << ret;
            _error_happen(ret);
            return ret;
        }
        if (rslt && _on_result) {
            char *str = new char[strlen(rslt) + 1];
            strcpy(str, rslt);
            std::thread([this, str, rec_stat](){_on_result(str, rec_stat == MSP_REC_STATUS_COMPLETE);}).detach();
        }
        usleep(500);
    }

    QISRSessionEnd(session_id, "normal");
    session_id = NULL;
    return 0;
}
