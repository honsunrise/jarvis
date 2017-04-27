//
// Created by zhsyourai on 4/12/17.
//

#ifndef JARVIS_VOICERECORD_H
#define JARVIS_VOICERECORD_H

#include <functional>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <alsa/asoundlib.h>
#include <signal.h>
#include <sys/stat.h>
#include <vector>
#include <thread>
#include "Voice.h"

extern "C" {
#include <filter_audio.h>
};

namespace Voice {

    class VoiceRecord {
        enum {
            RECORD_ERR_BASE = 0,
            RECORD_ERR_GENERAL,
            RECORD_ERR_MEMFAIL,
            RECORD_ERR_INVAL,
            RECORD_ERR_NOT_READY
        };

        typedef enum {
            RECORD_STATE_CREATED,    /* Init		*/
            RECORD_STATE_CLOSING,    /* Closing */
            RECORD_STATE_READY,        /* Opened	*/
            RECORD_STATE_STOPPING,    /* During Stop	*/
            RECORD_STATE_RECORDING,    /* Started	*/
        } RECORD_STATE;

    public:
        VoiceRecord(std::function<void(char *, size_t, void *)> data_callback,
                    std::function<void()> vad_callback, void *user_para);

        virtual ~VoiceRecord();

        int open(const voice_dev &dev, wave_format fmt);

        int close();

        int start();

        int stop();

        RECORD_STATE state();

    private:
        int setup();

        int _set_hwparams();

        int _set_swparams();

        int _prepare_rec_buffer();

        void _free_rec_buffer();

        ssize_t pcm_read(size_t r_count);

        void record_thread();

        unsigned int sample_rate;
        unsigned int sample_bit;
        unsigned int sample_cnt;
        unsigned int buffer_time = 0;
        unsigned int period_time = 0;
        unsigned long buffer_frames = 0;
        unsigned long period_frames = 0;
        unsigned int bits_per_frame = 0;

        char *_audio_buf;
        std::thread *_thread_handle;
        std::function<void(char *data, size_t len, void *user_para)> _data_callback;
        std::function<void()> _vad_callback;
        void *_user_parm;
        RECORD_STATE _state;
        snd_pcm_t *_handle;
        wave_format _fmt;
        Filter_Audio *filteraudio;
    };
}

#endif //JARVIS_VOICERECORD_H
