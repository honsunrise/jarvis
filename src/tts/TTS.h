//
// Created by zhsyourai on 4/27/17.
//

#ifndef JARVIS_TTS_H
#define JARVIS_TTS_H

#include <functional>

typedef struct _processor_product_info_ {
    const char *name;
    const char *version;
    const char *manufacturer_name;
    const char *home_page;
    void *custom_info;
} processor_product_info;

class TTS {
public:
    explicit TTS(std::function<void(const char *, unsigned int)> on_result,
                 std::function<void()> on_speech_begin,
                 std::function<void()> on_speech_end,
                 std::function<void(int reason)> on_error);

    explicit TTS(std::function<void(const char *, unsigned int)> on_result,
                 std::function<void(int reason)> on_error);

    virtual ~TTS();

    inline processor_product_info get_product_info();

    virtual int initialize() = 0;

    virtual int uninitialize() = 0;

    virtual int start() = 0;

    virtual int process(std::string text) = 0;

    virtual int end() = 0;

protected:
    std::function<void()> _on_begin;
    std::function<void()> _on_end;
    std::function<void(int reason)> _on_error;
    std::function<void(const char *, unsigned int)> _on_result;
};


#endif //JARVIS_TTS_H
