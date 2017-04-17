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

typedef union {
    char *name;
    int id;
} record_dev_id;

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
    sizeof(struct _wave_format_)    \
}

class VoiceRecord {
public:
    VoiceRecord(std::function<void(char *, size_t, void *)> data_callback, void *user_para);

    virtual ~VoiceRecord();

    int open(record_dev_id dev, wave_format fmt);

    int close();

    int start();

    int stop();

    RECORD_STATE state();

    std::vector<record_dev_id> list();

private:
    int setup();

    int _set_hwparams();

    int _set_swparams();

    int _prepare_rec_buffer();

    void _free_rec_buffer();

    static inline int format_ms_to_alsa(const wave_format *fmt, snd_pcm_format_t *format);

    static size_t get_pcm_device_cnt(snd_pcm_stream_t stream);

    static size_t list_pcm(snd_pcm_stream_t stream, char ***name_out, char ***desc_out);

    ssize_t pcm_read(size_t rcount);

    void list_record_dev();

    void record_thread();

    unsigned int sample_rate;
    unsigned int sample_bit;
    unsigned int sample_cnt;
    unsigned int buffer_time;
    unsigned int period_time;
    unsigned long buffer_frames;
    unsigned long period_frames;
    unsigned int bits_per_frame;

    char *_audio_buf;
    std::thread *_thread_handle;
    std::function<void(char *data, size_t len, void *user_para)> _data_callback;
    void *_user_parm;
    RECORD_STATE _state;
    std::string _name;
    snd_pcm_t *_pcm;
    wave_format _fmt;

    std::vector<record_dev_id> record_dev_list;
};


#endif //JARVIS_VOICERECORD_H
