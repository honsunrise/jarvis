//
// Created by zhsyourai on 4/28/17.
//

#ifndef JARVIS_TULINGOTT_H
#define JARVIS_TULINGOTT_H

#include <boost/asio/io_service.hpp>
#include "../chat.h"

class TulingOTT: public chat {
public:
    TulingOTT(const std::function<void(std::string)> &on_result, const std::function<void()> &on_speech_begin,
              const std::function<void()> &on_speech_end, const std::function<void(int)> &on_error);

    TulingOTT(const std::function<void(std::string)> &on_result, const std::function<void(int)> &on_error);

    int initialize() override;

    int uninitialize() override;

    int start() override;

    int process(std::string data) override;

    int end() override;

private:
    std::string api_key;
    boost::asio::io_service *io_service;
};


#endif //JARVIS_TULINGOTT_H
