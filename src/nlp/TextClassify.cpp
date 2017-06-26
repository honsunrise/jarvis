//
// Created by zhsyourai on 6/26/17.
//

#include "TextClassify.h"

TextClassify::TextClassify(std::function<void(bool is)> on_result, std::function<void()> on_speech_begin,
                           std::function<void()> on_speech_end, std::function<void(int reason)> on_error)
    : _on_result(on_result), _on_begin(on_speech_begin),
      _on_end(on_speech_end), _on_error(on_error) {

}

TextClassify::TextClassify(std::function<void(bool is)> on_result, std::function<void(int reason)> on_error)
    : _on_result(on_result), _on_begin(nullptr),
      _on_end(nullptr), _on_error(on_error) {

}

TextClassify::~TextClassify() {

}

processor_product_info TextClassify::get_product_info() {
  return processor_product_info();
}
