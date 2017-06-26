//
// Created by zhsyourai on 6/26/17.
//

#ifndef JARVIS_TEXTCLASSIFY_H
#define JARVIS_TEXTCLASSIFY_H

#include <engine.h>
class TextClassify {
 public:
  explicit TextClassify(std::function<void(bool is)> on_result,
                        std::function<void()> on_speech_begin,
                        std::function<void()> on_speech_end,
                        std::function<void(int reason)> on_error);

  explicit TextClassify(std::function<void(bool is)> on_result,
                        std::function<void(int reason)> on_error);

  virtual ~TextClassify();

  inline processor_product_info get_product_info();

  virtual int initialize() = 0;

  virtual int uninitialize() = 0;

  virtual int start() = 0;

  virtual int process(std::string data) = 0;

  virtual int end() = 0;

 protected:
  std::function<void()> _on_begin;
  std::function<void()> _on_end;
  std::function<void(int reason)> _on_error;
  std::function<void(bool is)> _on_result;
};

#endif //JARVIS_TEXTCLASSIFY_H
