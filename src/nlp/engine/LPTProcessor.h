//
// Created by zhsyourai on 4/17/17.
//

#ifndef JARVIS_LPTPROCESSOR_H
#define JARVIS_LPTPROCESSOR_H

#include <utils/AsyncHttpClient.h>
#include "../NLP.h"

/*
   {
    "id": 1,
    "cont": "打开",
    "pos": "v",
    "ne": "O",
    "parent": -1,
    "relate": "HED",
    "arg": [
     {
      "id": 0,
      "type": "A0",
      "beg": 0,
      "end": 0
     },
     {
      "id": 1,
      "type": "A1",
      "beg": 2,
      "end": 2
     }
    ]
   }
*/

class LPTProcessor : public NLP {
public:
    LPTProcessor(const std::function<void(std::vector<CONLL> conll)> &on_result, const std::function<void()> &on_speech_begin,
                 const std::function<void()> &on_speech_end, const std::function<void(int)> &on_error);

    LPTProcessor(const std::function<void(std::vector<CONLL> conll)> &on_result, const std::function<void(int)> &on_error);

    int initialize() override;

    int uninitialize() override;

    int start() override;

    int process(std::string data) override;

    int end() override;

private:
    std::string api_key;
    boost::asio::io_service *io_service;
};


#endif //JARVIS_LPTPROCESSOR_H
