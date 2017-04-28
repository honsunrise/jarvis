//
// Created by zhsyourai on 4/12/17.
//

#ifndef ZRPC_SPEECHRECOGNIZER_H
#define ZRPC_SPEECHRECOGNIZER_H

#include <functional>
#include "../engine.h"

class SpeechRecognizer {
public:
    explicit SpeechRecognizer(std::function<void(char *result, bool is_last)> on_result,
                              std::function<void()> on_speech_begin,
                              std::function<void()> on_speech_end,
                              std::function<void(int reason)> on_error);

    explicit SpeechRecognizer(std::function<void(char *result, bool is_last)> on_result,
                              std::function<void(int reason)> on_error);

    virtual ~SpeechRecognizer();

    inline processor_product_info get_product_info();

    virtual int initialize() = 0;

    virtual int uninitialize() = 0;

    virtual int start() = 0;

    virtual int listen(char *data, size_t len) = 0;

    virtual int end() = 0;

protected:
    std::function<void(char *result, bool is_last)> _on_result;
    std::function<void()> _on_begin;
    std::function<void()> _on_end;
    std::function<void(int reason)> _on_error;
private:
    processor_product_info _product_info;
};

#endif //ZRPC_SPEECHRECOGNIZER_H
