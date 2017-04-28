//
// Created by zhsyourai on 4/13/17.
//

#include "SpeechRecognizer.h"

processor_product_info SpeechRecognizer::get_product_info() {
    return _product_info;
}

SpeechRecognizer::~SpeechRecognizer() {
}

SpeechRecognizer::SpeechRecognizer(std::function<void(char *result, bool is_last)> on_result,
                                   std::function<void()> on_speech_begin,
                                   std::function<void()> on_speech_end,
                                   std::function<void(int reason)> on_error)
        : _on_result(on_result), _on_begin(on_speech_begin),
          _on_end(on_speech_end), _on_error(on_error) {
}

SpeechRecognizer::SpeechRecognizer(std::function<void(char *result, bool is_last)> on_result,
                                   std::function<void(int reason)> on_error)
        : _on_result(on_result), _on_error(on_error), _on_begin(nullptr),
          _on_end(nullptr) {
}
