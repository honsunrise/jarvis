//
// Created by zhsyourai on 4/27/17.
//

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

static std::vector<Voice::voice_dev> record_dev_list;

static std::vector<Voice::voice_dev> player_dev_list;

static void free_name_desc(char **name_or_desc) {
    if (nullptr == name_or_desc)
        return;
    while (*name_or_desc != nullptr) {
        free(*name_or_desc);
        *name_or_desc = nullptr;
        name_or_desc++;
    }
}

static size_t list_pcm(snd_pcm_stream_t stream, char ***name_out, char ***desc_out) {
    void **hints, **n;
    char **names, **descr;
    char *io;
    const char *filter;
    size_t cnt = 0;
    int i = 0;
    *name_out = nullptr;
    *desc_out = nullptr;

    if (snd_device_name_hint(-1, "pcm", &hints) < 0)
        return 0;
    n = hints;

    filter = stream == SND_PCM_STREAM_CAPTURE ? "Input" : "Output";

    while (*n != nullptr) {
        char *name;
        io = snd_device_name_get_hint(*n, "IOID");
        name = snd_device_name_get_hint(*n, "NAME");
        if (name && (io == nullptr || strcmp(io, filter) == 0))
            cnt++;
        if (io != nullptr)
            free(io);
        if (name != nullptr)
            free(name);
        n++;
    }

    if (!cnt) {
        goto fail;
    }

    *name_out = (char **) malloc((1 + cnt) * sizeof(char *));
    if (*name_out == nullptr)
        goto fail;
    *desc_out = (char **) malloc((1 + cnt) * sizeof(char *));
    if (*desc_out == nullptr)
        goto fail;

    /* the last one is a flag, nullptr */
    names = *name_out;
    descr = *desc_out;
    names[cnt] = nullptr;
    descr[cnt] = nullptr;

    n = hints;
    while (*n != nullptr && i < cnt) {
        *names = snd_device_name_get_hint(*n, "NAME");
        *descr = snd_device_name_get_hint(*n, "DESC");
        io = snd_device_name_get_hint(*n, "IOID");
        if (io != nullptr && strcmp(io, filter) != 0) {
            if (*names) free(*names);
            if (*descr) free(*descr);
        } else {
            if (*names == nullptr) {
                *names = (char *) malloc(4);
                memset(*names, 0, 4);
            }
            if (*descr == nullptr) {
                *descr = (char *) malloc(4);
                memset(*descr, 0, 4);
            }
            names++;
            descr++;
            i++;
        }
        if (io != nullptr)
            free(io);
        n++;
    }
    snd_device_name_free_hint(hints);
    return cnt;
    fail:
    free_name_desc(*name_out);
    free_name_desc(*desc_out);
    snd_device_name_free_hint(hints);
    return 0;
}

static void _prepare_capture_device_list() {
    char **name_array;
    char **desc_array;
    size_t count = list_pcm(SND_PCM_STREAM_CAPTURE, &name_array, &desc_array);
    for (int i = 0; i < count; ++i) {
        Voice::voice_dev _dev;
        _dev.name = name_array[i];
        _dev.desc = desc_array[i];
        _dev.id = i;
        record_dev_list.push_back(_dev);
    }
    free_name_desc(name_array);
    free_name_desc(desc_array);
}

static void _prepare_playback_device_list() {
    char **name_array;
    char **desc_array;
    size_t count = list_pcm(SND_PCM_STREAM_PLAYBACK, &name_array, &desc_array);
    for (int i = 0; i < count; ++i) {
        Voice::voice_dev _dev;
        _dev.name = name_array[i];
        _dev.desc = desc_array[i];
        _dev.id = i;
        player_dev_list.push_back(_dev);
    }
    free_name_desc(name_array);
    free_name_desc(desc_array);
}

static class __Init__ {
public:
    __Init__() {
        _prepare_capture_device_list();
        _prepare_playback_device_list();
    }
} __init__;

std::vector<Voice::voice_dev> Voice::list_capture_devices() {
    return record_dev_list;
}

std::vector<Voice::voice_dev> Voice::list_playback_devices() {
    return player_dev_list;
}

