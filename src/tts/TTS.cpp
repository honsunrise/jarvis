//
// Created by zhsyourai on 4/27/17.
//

#include "TTS.h"

TTS::TTS(std::function<void(const char *, unsigned int)> on_result, std::function<void()> on_speech_begin,
         std::function<void()> on_speech_end, std::function<void(int reason)> on_error)
        : _on_result(on_result), _on_begin(on_speech_begin),
          _on_end(on_speech_end), _on_error(on_error) {

}

TTS::TTS(std::function<void(const char *, unsigned int)> on_result, std::function<void(int reason)> on_error)
        : _on_result(on_result), _on_begin(nullptr),
          _on_end(nullptr), _on_error(on_error) {

}

TTS::~TTS() {

}

processor_product_info TTS::get_product_info() {
    return processor_product_info();
}