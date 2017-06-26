//
// Created by zhsyourai on 6/26/17.
//

#ifndef JARVIS_SELFTC_H
#define JARVIS_SELFTC_H

#include <boost/asio/io_service.hpp>
#include "../TextClassify.h"

class SelfTC : public TextClassify {
 public:
  SelfTC(const std::function<void(bool)> &on_result,
         const std::function<void()> &on_speech_begin,
         const std::function<void()> &on_speech_end,
         const std::function<void(int)> &on_error);

  SelfTC(const std::function<void(bool)> &on_result, const std::function<void(int)> &on_error);

  int initialize() override;

  int uninitialize() override;

  int start() override;

  int process(std::string data) override;

  int end() override;
 private:

  boost::asio::io_service *io_service;
};

#endif //JARVIS_SELFTC_H
