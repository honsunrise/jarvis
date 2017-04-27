//
// Created by zhsyourai on 4/27/17.
//

#include <boost/log/trivial.hpp>
#include "VoicePlayer.h"

Voice::VoicePlayer::VoicePlayer() {
    _state = PLAYER_STATE_CREATED;
}

int Voice::VoicePlayer::open(const Voice::voice_dev &dev, Voice::wave_format fmt) {
    if (_state != PLAYER_STATE_CREATED) {
        return -1;
    }
    if (_state == PLAYER_STATE_READY) {
        return 0;
    }
    int err = 0;
    err = snd_pcm_open(&_handle, dev.name.c_str(), SND_PCM_STREAM_PLAYBACK, 0);
    _state = PLAYER_STATE_READY;
    if (err < 0)
        goto fail;

    _fmt = fmt;
    err = setup();
    if (err)
        goto fail;
    return 0;
    fail:
    if (_handle) {
        snd_pcm_close(_handle);
        _handle = nullptr;
    }
    return err;
}

int Voice::VoicePlayer::close() {
    if (!(_state == PLAYER_STATE_READY || _state == PLAYER_STATE_PLAYERING || _state == PLAYER_STATE_STOPPING))
        return -PLAYER_ERR_NOT_READY;

    if (_state == PLAYER_STATE_PLAYERING)
        stop();

    _state = PLAYER_STATE_CLOSING;

    if (_handle) {
        snd_pcm_drain(_handle);
        snd_pcm_close(_handle);
        _handle = nullptr;
    }

    _state = PLAYER_STATE_CREATED;
    return 0;
}

int Voice::VoicePlayer::start() {
    int ret = 0;
    if (_state != PLAYER_STATE_READY)
        return -PLAYER_ERR_NOT_READY;
    if (_state == PLAYER_STATE_PLAYERING)
        return 0;
    _state = PLAYER_STATE_PLAYERING;
    return ret;
}

int Voice::VoicePlayer::stop() {
    int ret = 0;
    if (_state != PLAYER_STATE_PLAYERING)
        return -PLAYER_ERR_INVAL;

    _state = PLAYER_STATE_STOPPING;
    snd_pcm_drain(_handle);
    _state = PLAYER_STATE_READY;
    return ret;
}

int Voice::VoicePlayer::setup() {
    int err = 0;
    snd_pcm_format_t format;
    err = format_ms_to_alsa(&_fmt, &format);
    if (err) {
        BOOST_LOG_TRIVIAL(error) << "Invalid format";
        return -EINVAL;
    }
    // Set the audio card's hardware parameters (sample rate, bit resolution, etc)
    if ((err = snd_pcm_set_params(_handle, format, SND_PCM_ACCESS_RW_INTERLEAVED, _fmt.channels, _fmt.samples_per_sec, 1,
                                  500000)) < 0)
        BOOST_LOG_TRIVIAL(error) << "Can't set sound parameters:" << snd_strerror(err);

    return err;
}

void Voice::VoicePlayer::play(const char *data, unsigned int len) {
    register snd_pcm_sframes_t count, ret,  frames = len / (_fmt.bits_per_sample / 8);
    // Output the wave data
    count = 0;
    do {
        ret = snd_pcm_writei(_handle, data + count * 2, frames - count);

        // If an error, try to recover from it
        if (ret < 0)
            ret = snd_pcm_recover(_handle, (int) ret, 0);
        if (ret < 0) {
            BOOST_LOG_TRIVIAL(error) << "Error playing wave:" << snd_strerror((int) ret);
            break;
        }

        // Update our pointer
        count += ret;
    } while (count < frames);
}
