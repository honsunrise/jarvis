//
// Created by zhsyourai on 4/12/17.
//

#include <boost/log/trivial.hpp>
#include "VoiceRecord.h"

VoiceRecord::VoiceRecord(std::function<void(char *, size_t, void *)> data_callback,
                         void *user_parm)
        : _data_callback(data_callback), _user_parm(user_parm), _handle(nullptr) {
    _state = RECORD_STATE_CREATED;
    _prepare_device_list();
}

VoiceRecord::~VoiceRecord() {
    _state = RECORD_STATE_CLOSING;
}

int VoiceRecord::open(const voice_record_dev &dev, wave_format fmt) {
    if (_state != RECORD_STATE_CREATED) {
        return -1;
    }
    if (_state == RECORD_STATE_READY) {
        return 0;
    }
    int err = 0;
    err = snd_pcm_open(&_handle, dev.name.c_str(), SND_PCM_STREAM_CAPTURE, 0);
    _state = RECORD_STATE_READY;
    if (err < 0)
        goto fail;

    _fmt = fmt;
    err = setup();
    if (err)
        goto fail;

    err = _prepare_rec_buffer();
    if (err)
        goto fail;

    return 0;

    fail:
    if (_handle) {
        snd_pcm_close(_handle);
        _handle = nullptr;
    }
    _free_rec_buffer();
    return err;
}

int VoiceRecord::close() {
    if (!(_state == RECORD_STATE_READY || _state == RECORD_STATE_RECORDING || _state == RECORD_STATE_STOPPING))
        return -RECORD_ERR_NOT_READY;

    if (_state == RECORD_STATE_RECORDING)
        stop();

    _state = RECORD_STATE_CLOSING;

    if (_handle) {
        snd_pcm_drain(_handle);
        snd_pcm_close(_handle);
        _handle = nullptr;
    }
    _free_rec_buffer();
    _state = RECORD_STATE_CREATED;
    return 0;
}

int VoiceRecord::start() {
    int ret;
    if (_state != RECORD_STATE_READY)
        return -RECORD_ERR_NOT_READY;
    if (_state == RECORD_STATE_RECORDING)
        return 0;

    ret = snd_pcm_prepare(_handle);
    ret = snd_pcm_start(_handle);
    if (ret == 0)
        _state = RECORD_STATE_RECORDING;

    _thread_handle = new std::thread(std::bind(&VoiceRecord::record_thread, this));
    return ret;
}

int VoiceRecord::stop() {
    int ret;
    if (_state != RECORD_STATE_RECORDING)
        return -RECORD_ERR_INVAL;

    _state = RECORD_STATE_STOPPING;

    _thread_handle->join();

    ret = snd_pcm_drop(_handle);
    if (ret == 0) {
        _state = RECORD_STATE_READY;
    }
    return ret;
}

RECORD_STATE VoiceRecord::state() {
    return _state;
}

int VoiceRecord::format_ms_to_alsa(const wave_format *fmt, snd_pcm_format_t *format) {
    snd_pcm_format_t tmp;
    tmp = snd_pcm_build_linear_format(fmt->bits_per_sample, fmt->bits_per_sample,
                                      fmt->bits_per_sample == 8 ? 1 : 0, 0);
    if (tmp == SND_PCM_FORMAT_UNKNOWN)
        return -EINVAL;
    *format = tmp;
    return 0;
}

int VoiceRecord::_set_hwparams() {
    snd_pcm_hw_params_t *params;
    int err;
    unsigned int rate;
    snd_pcm_format_t format;
    snd_pcm_uframes_t size;

    snd_pcm_hw_params_alloca(&params);
    err = snd_pcm_hw_params_any(_handle, params);
    if (err < 0) {
        BOOST_LOG_TRIVIAL(error) << "Broken configuration for this PCM";
        return err;
    }
    err = snd_pcm_hw_params_set_access(_handle, params,
                                       SND_PCM_ACCESS_RW_INTERLEAVED);
    if (err < 0) {
        BOOST_LOG_TRIVIAL(error) << "Access type not available";
        return err;
    }
    err = format_ms_to_alsa(&_fmt, &format);
    if (err) {
        BOOST_LOG_TRIVIAL(error) << "Invalid format";
        return -EINVAL;
    }
    err = snd_pcm_hw_params_set_format(_handle, params, format);
    if (err < 0) {
        BOOST_LOG_TRIVIAL(error) << "Sample format non available";
        return err;
    }
    err = snd_pcm_hw_params_set_channels(_handle, params, _fmt.channels);
    if (err < 0) {
        BOOST_LOG_TRIVIAL(error) << "Channels count non available";
        return err;
    }

    rate = _fmt.samples_per_sec;
    err = snd_pcm_hw_params_set_rate_near(_handle, params, &rate, 0);
    if (err < 0) {
        BOOST_LOG_TRIVIAL(error) << "Set rate failed";
        return err;
    }
    if (rate != _fmt.samples_per_sec) {
        BOOST_LOG_TRIVIAL(error) << "Rate mismatch";
        return -EINVAL;
    }
    if (buffer_time == 0 || period_time == 0) {
        err = snd_pcm_hw_params_get_buffer_time_max(params,
                                                    &buffer_time, 0);
        assert(err >= 0);
        if (buffer_time > 500000)
            buffer_time = 500000;
        period_time = buffer_time / 4;
    }
    err = snd_pcm_hw_params_set_period_time_near(_handle, params, &period_time, 0);
    if (err < 0) {
        BOOST_LOG_TRIVIAL(error) << "set period time fail";
        return err;
    }
    err = snd_pcm_hw_params_set_buffer_time_near(_handle, params, &buffer_time, 0);
    if (err < 0) {
        BOOST_LOG_TRIVIAL(error) << "set buffer time failed";
        return err;
    }
    err = snd_pcm_hw_params_get_period_size(params, &size, 0);
    if (err < 0) {
        BOOST_LOG_TRIVIAL(error) << "get period size fail";
        return err;
    }
    period_frames = size;
    err = snd_pcm_hw_params_get_buffer_size(params, &size);
    if (size == period_frames) {
        BOOST_LOG_TRIVIAL(error) << "Can't use period equal to buffer size " << size << "==" << period_frames;
        return -EINVAL;
    }
    buffer_frames = size;
    bits_per_frame = _fmt.bits_per_sample;

    /* set to driver */
    err = snd_pcm_hw_params(_handle, params);
    if (err < 0) {
        BOOST_LOG_TRIVIAL(error) << "Unable to install hw params:";
        return err;
    }
    return 0;
}

int VoiceRecord::_set_swparams() {
    int err;
    snd_pcm_sw_params_t *swparams;
    /* sw para */
    snd_pcm_sw_params_alloca(&swparams);
    err = snd_pcm_sw_params_current(_handle, swparams);
    if (err < 0) {
        BOOST_LOG_TRIVIAL(error) << "get current sw para fail";
        return err;
    }

    err = snd_pcm_sw_params_set_avail_min(_handle, swparams,
                                          period_frames);
    if (err < 0) {
        BOOST_LOG_TRIVIAL(error) << "set avail min failed";
        return err;
    }
    /* set a value bigger than the buffer frames to prevent the auto start.
     * we use the snd_pcm_start to explicit start the pcm */
    err = snd_pcm_sw_params_set_start_threshold(_handle, swparams,
                                                buffer_frames * 2);
    if (err < 0) {
        BOOST_LOG_TRIVIAL(error) << "set start threshold fail";
        return err;
    }

    if ((err = snd_pcm_sw_params(_handle, swparams)) < 0) {
        BOOST_LOG_TRIVIAL(error) << "unable to install sw params:";
        return err;
    }
    return 0;
}

int VoiceRecord::_prepare_rec_buffer() {
    size_t sz = (period_frames * bits_per_frame / 8);
    _audio_buf = (char *) malloc(sz);
    if (!_audio_buf)
        return -ENOMEM;
    return 0;
}

void VoiceRecord::_free_rec_buffer() {
    if (_audio_buf) {
        free(_audio_buf);
        _audio_buf = nullptr;
    }
}

int VoiceRecord::setup() {
    int err = 0;
    err = _set_hwparams();
    if (err < 0)
        return err;
    err = _set_swparams();
    if (err < 0)
        return err;
    return err;
}

ssize_t VoiceRecord::pcm_read(size_t r_count) {
    snd_pcm_sframes_t frames;
    size_t count = r_count;
    char *data;
    if (_handle == nullptr)
        return -RECORD_ERR_INVAL;

    data = _audio_buf;
    while (count > 0) {
        frames = snd_pcm_readi(_handle, data, count);
        if (frames < 0) {
            frames = snd_pcm_recover(_handle, (int) frames, 0);
        }

        if (frames > 0) {
            count -= frames;
            data += frames * bits_per_frame / 8;
        } else {
            return -RECORD_ERR_INVAL;
        }
    }
    return r_count;
}

void VoiceRecord::record_thread() {
    BOOST_LOG_TRIVIAL(info) << "record_thread start!";
    size_t frames, bytes;
    sigset_t mask, old_mask;

    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGTERM);
    pthread_sigmask(SIG_BLOCK, &mask, &old_mask);

    while (1) {
        frames = period_frames;
        bytes = frames * bits_per_frame / 8;

        if (pcm_read(frames) != frames) {
            return;
        }

        /* closing, exit the thread */
        if (_state == RECORD_STATE_CLOSING || _state == RECORD_STATE_STOPPING)
            break;

        _data_callback(_audio_buf, bytes, _user_parm);
    }
}

static void free_name_desc(char **name_or_desc) {
    if (nullptr == name_or_desc)
        return;
    while (*name_or_desc != nullptr) {
        free(*name_or_desc);
        *name_or_desc = nullptr;
        name_or_desc++;
    }
}

size_t VoiceRecord::list_pcm(snd_pcm_stream_t stream, char ***name_out, char ***desc_out) {
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

std::vector<voice_record_dev> VoiceRecord::list() {
    return record_dev_list;
}

void VoiceRecord::_prepare_device_list() {
    char **name_array;
    char **desc_array;
    size_t count = list_pcm(SND_PCM_STREAM_CAPTURE, &name_array, &desc_array);
    for (int i = 0; i < count; ++i) {
        voice_record_dev _dev;
        _dev.name = name_array[i];
        _dev.desc = desc_array[i];
        _dev.id = i;
        record_dev_list.push_back(_dev);
    }
    free_name_desc(name_array);
    free_name_desc(desc_array);
}
