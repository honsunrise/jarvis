//
// Created by zhsyourai on 4/28/17.
//

#include "chat.h"

chat::chat(std::function<void(std::string info)> on_result, std::function<void()> on_speech_begin,
           std::function<void()> on_speech_end, std::function<void(int reason)> on_error)
        : _on_result(on_result), _on_begin(on_speech_begin),
          _on_end(on_speech_end), _on_error(on_error) {

}

chat::chat(std::function<void(std::string info)> on_result, std::function<void(int reason)> on_error)
        : _on_result(on_result), _on_begin(nullptr),
          _on_end(nullptr), _on_error(on_error) {

}

chat::~chat() {

}

processor_product_info chat::get_product_info() {
    return processor_product_info();
}
