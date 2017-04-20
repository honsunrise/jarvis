//
// Created by zhsyourai on 4/17/17.
//

#ifndef JARVIS_NLP_H
#define JARVIS_NLP_H


#include <cstddef>
#include <functional>

typedef struct _processor_product_info_ {
    const char *name;
    const char *version;
    const char *manufacturer_name;
    const char *home_page;
    void *custom_info;
} processor_product_info;

class NLP {
public:
    explicit NLP(std::function<void(int reason)> on_result,
                 std::function<void()> on_speech_begin,
                 std::function<void()> on_speech_end,
                 std::function<void(int reason)> on_error);

    explicit NLP(std::function<void(int reason)> on_result,
                 std::function<void(int reason)> on_error);

    virtual ~NLP();

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
    std::function<void(int reason)> _on_result;
};


#endif //JARVIS_NLP_H
