//
// Created by zhsyourai on 4/28/17.
//

#ifndef JARVIS_CHAT_H
#define JARVIS_CHAT_H

#include <functional>
#include "../engine.h"

class chat {
public:
    explicit chat(std::function<void(std::string info)> on_result,
    std::function<void()> on_speech_begin,
            std::function<void()> on_speech_end,
    std::function<void(int reason)> on_error);

    explicit chat(std::function<void(std::string info)> on_result,
    std::function<void(int reason)> on_error);

    virtual ~chat();

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
    std::function<void(std::string info)> _on_result;
};


#endif //JARVIS_CHAT_H
