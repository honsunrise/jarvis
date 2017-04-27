//
// Created by zhsyourai on 4/12/17.
//

#include <boost/log/trivial.hpp>
#include "VoiceRecord.h"

Voice::VoiceRecord::VoiceRecord(std::function<void(char *, size_t, void *)> data_callback,
                         std::function<void()> vad_callback,
                         void *user_parm)
        : _data_callback(data_callback), _vad_callback(vad_callback), _user_parm(user_parm), _handle(nullptr) {
    _state = RECORD_STATE_CREATED;
}

Voice::VoiceRecord::~VoiceRecord() {
    _state = RECORD_STATE_CLOSING;
}

int Voice::VoiceRecord::open(const voice_dev &dev, wave_format fmt) {
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

    /* Prepare filter_audio */
    filteraudio = new_filter_audio(_fmt.samples_per_sec);
    // enable_disable_filters(filteraudio, 0, 0, 0, 1);
    return 0;

    fail:
    if (_handle) {
        snd_pcm_close(_handle);
        _handle = nullptr;
    }
    _free_rec_buffer();
    return err;
}

int Voice::VoiceRecord::close() {
    if (!(_state == RECORD_STATE_READY || _state == RECORD_STATE_RECORDING || _state == RECORD_STATE_STOPPING))
        return -RECORD_ERR_NOT_READY;

    if (_state == RECORD_STATE_RECORDING)
        stop();

    _state = RECORD_STATE_CLOSING;

    if (_handle) {
        snd_pcm_close(_handle);
        _handle = nullptr;
    }

    kill_filter_audio(filteraudio);

    _free_rec_buffer();
    _state = RECORD_STATE_CREATED;
    return 0;
}

int Voice::VoiceRecord::start() {
    int ret;
    if (_state != RECORD_STATE_READY)
        return -RECORD_ERR_NOT_READY;
    if (_state == RECORD_STATE_RECORDING)
        return 0;

    ret = snd_pcm_prepare(_handle);
    ret = snd_pcm_start(_handle);
    if (ret == 0)
        _state = RECORD_STATE_RECORDING;

    _thread_handle = new std::thread(std::bind(&Voice::VoiceRecord::record_thread, this));
    return ret;
}

int Voice::VoiceRecord::stop() {
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

Voice::VoiceRecord::RECORD_STATE Voice::VoiceRecord::state() {
    return _state;
}

int Voice::VoiceRecord::_set_hwparams() {
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
        if (buffer_time > 400000)
            buffer_time = 400000;
        period_time = buffer_time / 4;
    }
    err = snd_pcm_hw_params_set_period_time_near(_handle, params, &period_time, 0);
    if (err < 0) {
        BOOST_LOG_TRIVIAL(error) << "set period time fail";
        return err;
    }
    BOOST_LOG_TRIVIAL(info) << "set period time " << period_time;
    err = snd_pcm_hw_params_set_buffer_time_near(_handle, params, &buffer_time, 0);
    if (err < 0) {
        BOOST_LOG_TRIVIAL(error) << "set buffer time failed";
        return err;
    }
    BOOST_LOG_TRIVIAL(info) << "set buffer time " << buffer_time;

    err = snd_pcm_hw_params_get_period_size(params, &size, 0);
    if (err < 0) {
        BOOST_LOG_TRIVIAL(error) << "get period size fail";
        return err;
    }

    BOOST_LOG_TRIVIAL(info) << "get period size " << size;
    period_frames = size;

    err = snd_pcm_hw_params_set_period_size_near(_handle, params, &period_frames, 0);
    if (err < 0) {
        BOOST_LOG_TRIVIAL(error) << "set period size fail";
        return err;
    }

    err = snd_pcm_hw_params_get_buffer_size(params, &size);
    if (err < 0) {
        BOOST_LOG_TRIVIAL(error) << "get buffer size fail";
        return err;
    }
    BOOST_LOG_TRIVIAL(info) << "get buffer size " << size;

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

int Voice::VoiceRecord::_set_swparams() {
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

int Voice::VoiceRecord::_prepare_rec_buffer() {
    size_t sz = (period_frames * bits_per_frame / 8);
    _audio_buf = (char *) malloc(sz);
    if (!_audio_buf)
        return -ENOMEM;
    return 0;
}

void Voice::VoiceRecord::_free_rec_buffer() {
    if (_audio_buf) {
        free(_audio_buf);
        _audio_buf = nullptr;
    }
}

int Voice::VoiceRecord::setup() {
    int err = 0;
    err = _set_hwparams();
    if (err < 0)
        return err;
    err = _set_swparams();
    if (err < 0)
        return err;
    return err;
}

ssize_t Voice::VoiceRecord::pcm_read(size_t r_count) {
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

void Voice::VoiceRecord::record_thread() {
    BOOST_LOG_TRIVIAL(info) << "record_thread start!";
    size_t frames, bytes;
    sigset_t mask, old_mask;

    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGTERM);
    pthread_sigmask(SIG_BLOCK, &mask, &old_mask);

    int filter_result = 0;
    int last_result = 0;
    int novoice_count = 0;
    int voice_count = 0;

    frames = period_frames;
    bytes = frames * bits_per_frame / 8;

    char *temp_audio_buf = new char[bytes * 100];
    size_t temp_audio_ptr = 0;
    int interval = 1000 * frames / _fmt.samples_per_sec;

    while (1) {

        if (pcm_read(frames) != frames) {
            return;
        }

        /* closing, exit the thread */
        if (_state == RECORD_STATE_CLOSING || _state == RECORD_STATE_STOPPING)
            break;

        last_result = filter_result;

        filter_result = filter_audio(filteraudio, (int16_t *) _audio_buf, (unsigned int) frames);

        BOOST_LOG_TRIVIAL(info) << "Filter result " << filter_result;
        if (!filter_result) {
            novoice_count++;
        } else {
            memcpy(temp_audio_buf + temp_audio_ptr, _audio_buf, bytes);
            temp_audio_ptr += bytes;
            if (last_result) {
                voice_count++;
            }
        }
        if (interval * novoice_count >= 3 * 1000) {
            novoice_count = 0;
            std::thread([&](){_vad_callback();}).detach();
        }
        if (interval * voice_count > 400) {
            novoice_count = 0;
            voice_count = 0;
            _data_callback(temp_audio_buf, temp_audio_ptr, _user_parm);
            temp_audio_ptr = 0;
        }
    }

    delete[]temp_audio_buf;
}
