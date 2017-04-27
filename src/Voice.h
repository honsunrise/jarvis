//
// Created by zhsyourai on 4/27/17.
//

#ifndef JARVIS_VOICE_H_H
#define JARVIS_VOICE_H_H

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

namespace Voice {

    typedef struct _voice_record_dev {
        int id;
        std::string name;
        std::string desc;
    } voice_dev;

#define WAVE_FORMAT_PCM  1

    typedef struct _wave_format_ {
        unsigned short format_tag;
        unsigned short channels;
        unsigned int samples_per_sec;
        unsigned int avg_bytes_per_sec;
        unsigned short block_align;
        unsigned short bits_per_sample;
        unsigned short cb_size;
    } wave_format;

#define DEFAULT_FORMAT        \
{\
    WAVE_FORMAT_PCM,    \
    1,            \
    16000,            \
    32000,            \
    2,            \
    16,            \
    sizeof(struct Voice::_wave_format_)    \
}

    inline int format_ms_to_alsa(const wave_format *fmt, snd_pcm_format_t *format) {
        snd_pcm_format_t tmp;
        tmp = snd_pcm_build_linear_format(fmt->bits_per_sample, fmt->bits_per_sample,
                                          fmt->bits_per_sample == 8 ? 1 : 0, 0);
        if (tmp == SND_PCM_FORMAT_UNKNOWN)
            return -EINVAL;
        *format = tmp;
        return 0;
    }

    std::vector<voice_dev> list_capture_devices();

    std::vector<voice_dev> list_playback_devices();
}

#endif //JARVIS_VOICE_H_H
