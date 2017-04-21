//
// Created by zhsyourai on 4/12/17.
//

#include <boost/log/trivial.hpp>
#include "VoiceRecord.h"

VoiceRecord::VoiceRecord(std::function<void(char *, size_t, void *)> data_callback,
                         void *user_parm)
        : _data_callback(data_callback), _user_parm(user_parm), _pcm(nullptr) {
    _state = RECORD_STATE_CREATED;
}

VoiceRecord::~VoiceRecord() {
    _state = RECORD_STATE_CLOSING;
}

int VoiceRecord::open(record_dev_id dev, wave_format fmt) {
    if (_state != RECORD_STATE_CREATED) {
        return -1;
    }
    if (_state == RECORD_STATE_READY) {
        return 0;
    }
    int err = 0;
    err = snd_pcm_open(&_pcm, dev.name, SND_PCM_STREAM_CAPTURE, 0);
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

    _thread_handle = new std::thread(std::bind(&VoiceRecord::record_thread, this));

    return 0;

    fail:
    if (_pcm) {
        snd_pcm_close(_pcm);
        _pcm = nullptr;
    }
    _free_rec_buffer();
    return err;
}

int VoiceRecord::close() {
    if (!(_state == RECORD_STATE_READY || _state == RECORD_STATE_RECORDING))
        return -RECORD_ERR_NOT_READY;

    if (_state == RECORD_STATE_RECORDING)
        stop();

    _state = RECORD_STATE_CLOSING;

    _thread_handle->join();

    if (_pcm) {
        snd_pcm_close(_pcm);
        _pcm = NULL;
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

    ret = snd_pcm_start(_pcm);
    if (ret == 0)
        _state = RECORD_STATE_RECORDING;
    return ret;
}

int VoiceRecord::stop() {
    int ret;
    if (_state != RECORD_STATE_RECORDING)
        return -RECORD_ERR_INVAL;

    _state = RECORD_STATE_STOPPING;
    ret = snd_pcm_drop(_pcm);
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
    err = snd_pcm_hw_params_any(_pcm, params);
    if (err < 0) {
        BOOST_LOG_TRIVIAL(error) << "Broken configuration for this PCM";
        return err;
    }
    err = snd_pcm_hw_params_set_access(_pcm, params,
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
    err = snd_pcm_hw_params_set_format(_pcm, params, format);
    if (err < 0) {
        BOOST_LOG_TRIVIAL(error) << "Sample format non available";
        return err;
    }
    err = snd_pcm_hw_params_set_channels(_pcm, params, _fmt.channels);
    if (err < 0) {
        BOOST_LOG_TRIVIAL(error) << "Channels count non available";
        return err;
    }

    rate = _fmt.samples_per_sec;
    err = snd_pcm_hw_params_set_rate_near(_pcm, params, &rate, 0);
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
    err = snd_pcm_hw_params_set_period_time_near(_pcm, params, &period_time, 0);
    if (err < 0) {
        BOOST_LOG_TRIVIAL(error) << "set period time fail";
        return err;
    }
    err = snd_pcm_hw_params_set_buffer_time_near(_pcm, params, &buffer_time, 0);
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
    err = snd_pcm_hw_params(_pcm, params);
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
    err = snd_pcm_sw_params_current(_pcm, swparams);
    if (err < 0) {
        BOOST_LOG_TRIVIAL(error) << "get current sw para fail";
        return err;
    }

    err = snd_pcm_sw_params_set_avail_min(_pcm, swparams,
                                          period_frames);
    if (err < 0) {
        BOOST_LOG_TRIVIAL(error) << "set avail min failed";
        return err;
    }
    /* set a value bigger than the buffer frames to prevent the auto start.
     * we use the snd_pcm_start to explicit start the pcm */
    err = snd_pcm_sw_params_set_start_threshold(_pcm, swparams,
                                                buffer_frames * 2);
    if (err < 0) {
        BOOST_LOG_TRIVIAL(error) << "set start threshold fail";
        return err;
    }

    if ((err = snd_pcm_sw_params(_pcm, swparams)) < 0) {
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
        _audio_buf = NULL;
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

void VoiceRecord::list_record_dev() {
}

static void free_name_desc(char **name_or_desc) {
    char **ss;
    ss = name_or_desc;
    if (NULL == name_or_desc)
        return;
    while (*name_or_desc) {
        free(*name_or_desc);
        *name_or_desc = NULL;
        name_or_desc++;
    }
    free(ss);
}

size_t VoiceRecord::get_pcm_device_cnt(snd_pcm_stream_t stream) {
    void **hints, **n;
    char *io, *name;
    const char *filter;
    size_t cnt = 0;

    if (snd_device_name_hint(-1, "pcm", &hints) < 0)
        return 0;
    n = hints;
    filter = stream == SND_PCM_STREAM_CAPTURE ? "Input" : "Output";
    while (*n != NULL) {
        io = snd_device_name_get_hint(*n, "IOID");
        name = snd_device_name_get_hint(*n, "NAME");
        if (name && (io == NULL || strcmp(io, filter) == 0))
            cnt++;
        if (io != NULL)
            free(io);
        if (name != NULL)
            free(name);
        n++;
    }
    snd_device_name_free_hint(hints);
    return cnt;
}

size_t VoiceRecord::list_pcm(snd_pcm_stream_t stream, char ***name_out, char ***desc_out) {
    void **hints, **n;
    char **name, **descr;
    char *io;
    const char *filter;
    size_t cnt = 0;
    int i = 0;

    if (snd_device_name_hint(-1, "pcm", &hints) < 0)
        return 0;
    n = hints;
    cnt = get_pcm_device_cnt(stream);
    if (!cnt) {
        goto fail;
    }

    *name_out = (char **) calloc(sizeof(char *), (1 + cnt));
    if (*name_out == NULL)
        goto fail;
    *desc_out = (char **) calloc(sizeof(char *), (1 + cnt));
    if (*desc_out == NULL)
        goto fail;

    /* the last one is a flag, NULL */
    name_out[cnt] = NULL;
    desc_out[cnt] = NULL;
    name = *name_out;
    descr = *desc_out;

    filter = stream == SND_PCM_STREAM_CAPTURE ? "Input" : "Output";
    while (*n != NULL && i < cnt) {
        *name = snd_device_name_get_hint(*n, "NAME");
        *descr = snd_device_name_get_hint(*n, "DESC");
        io = snd_device_name_get_hint(*n, "IOID");
        if (io != NULL && strcmp(io, filter) != 0) {
            if (*name) free(*name);
            if (*descr) free(*descr);
        } else {
            if (*descr == NULL) {
                *descr = (char *) malloc(4);
                memset(*descr, 0, 4);
            }
            name++;
            descr++;
            i++;
        }
        if (io != NULL)
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

static int xrun_recovery(snd_pcm_t *pcm, int err) {
    if (err == -EPIPE) {    /* over-run */
        if (true)
            printf("!!!!!!overrun happend!!!!!!");

        err = snd_pcm_prepare(pcm);
        if (err < 0) {
            if (true)
                printf("Can't recovery from overrun,"
                               "prepare failed: %s\n", snd_strerror(err));
            return err;
        }
        return 0;
    } else if (err == -ESTRPIPE) {
        while ((err = snd_pcm_resume(pcm)) == -EAGAIN)
            usleep(200000);    /* wait until the suspend flag is released */
        if (err < 0) {
            err = snd_pcm_prepare(pcm);
            if (err < 0) {
                if (true)
                    printf("Can't recovery from suspend,"
                                   "prepare failed: %s\n", snd_strerror(err));
                return err;
            }
        }
        return 0;
    }
    return err;
}

ssize_t VoiceRecord::pcm_read(size_t rcount) {
    ssize_t r;
    size_t count = rcount;
    char *data;
    if (_pcm == nullptr)
        return -RECORD_ERR_INVAL;

    data = _audio_buf;
    while (count > 0) {
        r = snd_pcm_readi(_pcm, data, count);
        if (r == -EAGAIN || (r >= 0 && (size_t) r < count)) {
            snd_pcm_wait(_pcm, 100);
        } else if (r < 0) {
            if (xrun_recovery(_pcm, (int) r) < 0) {
                return -1;
            }
        }

        if (r > 0) {
            count -= r;
            data += r * bits_per_frame / 8;
        }
    }
    return rcount;
}

void VoiceRecord::record_thread() {
    size_t frames, bytes;
    sigset_t mask, oldmask;

    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGTERM);
    pthread_sigmask(SIG_BLOCK, &mask, &oldmask);

    while (1) {
        frames = period_frames;
        bytes = frames * bits_per_frame / 8;

        /* closing, exit the thread */
        if (_state == RECORD_STATE_CLOSING)
            break;

        if (_state != RECORD_STATE_RECORDING)
            usleep(1000);

        if (pcm_read(frames) != frames) {
            return;
        }

        _data_callback(_audio_buf, bytes, _user_parm);
    }
}

std::vector<record_dev_id> VoiceRecord::list() {
    std::vector<record_dev_id> dev_list;
    char **name_array;
    char **desc_array;
    size_t count = list_pcm(SND_PCM_STREAM_CAPTURE, &name_array, &desc_array);
    for (int i = 0; i < count; ++i) {
        record_dev_id _id;
        _id.name = name_array[i];
        dev_list.push_back(_id);
    }
    // free_name_desc(&name_array);
    // free_name_desc(&desc_array);
    return dev_list;
}
