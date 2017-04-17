//
// Created by zhsyourai on 4/12/17.
//

#ifndef ZRPC_SPEECHRECOGNIZER_H
#define ZRPC_SPEECHRECOGNIZER_H

#include <functional>

typedef struct _recognizer_product_info_ {
    const char *name;
    const char *version;
    const char *manufacturer_name;
    const char *home_page;
    void *custom_info;
} recognizer_product_info;

class SpeechRecognizer {
public:
    explicit SpeechRecognizer(std::function<void(const char *result, char is_last)> on_result,
                              std::function<void()> on_speech_begin,
                              std::function<void()> on_speech_end,
                              std::function<void(int reason)> on_error);

    explicit SpeechRecognizer(std::function<void(const char *result, char is_last)> on_result,
                              std::function<void(int reason)> on_error);

    virtual ~SpeechRecognizer();

    inline recognizer_product_info get_product_info();

    virtual int initialize() = 0;

    virtual int uninitialize() = 0;

    virtual int start() = 0;

    virtual int listen(char *data, size_t len) = 0;

    virtual int end() = 0;

protected:
    std::function<void(const char *result, char is_last)> _on_result;
    std::function<void()> _on_begin;
    std::function<void()> _on_end;
    std::function<void(int reason)> _on_error;
private:
    recognizer_product_info _product_info;
};

#endif //ZRPC_SPEECHRECOGNIZER_H
