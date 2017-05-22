//
// Created by zhsyourai on 4/17/17.
//

#ifndef JARVIS_NLP_H
#define JARVIS_NLP_H


#include <cstddef>
#include <functional>
#include <vector>
#include "../engine.h"

typedef struct _CONLL_PREDICATE_ {
    int id;
    std::string type;
    int begin;
    int end;
} CONLL_PREDICATE;

typedef struct _CONLL_ {
    int id;
    std::string content;
    std::string pos;
    std::string ne;
    int parent;
    std::string relate;
    int semparent;
    std::string semrelate;
    std::vector<CONLL_PREDICATE> arg;
} CONLL;

static CONLL NULL_CONLL = {.id = -1};

inline CONLL search_semrelate(std::vector<CONLL> conlls, std::string text) {
    for (CONLL conll : conlls) {
        if (conll.semrelate == text)
            return conll;
    }
    return NULL_CONLL;
}

inline std::vector<CONLL> search_semparent(std::vector<CONLL> conlls, int p) {
    std::vector<CONLL> ret;
    for (CONLL conll : conlls) {
        if (conll.semparent == p)
            ret.push_back(conll);
    }
    return ret;
}

class NLP {
public:
    explicit NLP(std::function<void(std::vector<CONLL> conll)> on_result,
                 std::function<void()> on_speech_begin,
                 std::function<void()> on_speech_end,
                 std::function<void(int reason)> on_error);

    explicit NLP(std::function<void(std::vector<CONLL> conll)> on_result,
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
    std::function<void(std::vector<CONLL> conll)> _on_result;
};


#endif //JARVIS_NLP_H
