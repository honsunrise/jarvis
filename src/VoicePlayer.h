//
// Created by zhsyourai on 4/27/17.
//

#ifndef JARVIS_VOICEPLAYER_H
#define JARVIS_VOICEPLAYER_H

#include "Voice.h"

namespace Voice {
    class VoicePlayer {
        enum {
            PLAYER_ERR_BASE = 0,
            PLAYER_ERR_GENERAL,
            PLAYER_ERR_MEMFAIL,
            PLAYER_ERR_INVAL,
            PLAYER_ERR_NOT_READY
        };

        typedef enum {
            PLAYER_STATE_CREATED,    /* Init		*/
            PLAYER_STATE_CLOSING,    /* Closing */
            PLAYER_STATE_READY,        /* Opened	*/
            PLAYER_STATE_STOPPING,    /* During Stop	*/
            PLAYER_STATE_PLAYERING,    /* Started	*/
        } PLAYER_STATE;
    public:
        VoicePlayer();

        int open(const voice_dev &dev, wave_format fmt);

        int close();

        int start();

        int stop();

        void play(const char *data, unsigned int len);

    private:
        int setup();

        PLAYER_STATE _state;
        snd_pcm_t *_handle;
        wave_format _fmt;
    };
}

#endif //JARVIS_VOICEPLAYER_H
