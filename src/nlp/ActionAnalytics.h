//
// Created by zhsyourai on 5/22/17.
//

#ifndef JARVIS_ACTIONANALYTICS_H
#define JARVIS_ACTIONANALYTICS_H

#include "NLP.h"

struct Action {
    std::string action;
    std::vector<std::string> params;
};

class ActionAnalytics {
public:
    Action analytics(std::vector<CONLL> conlls);
};


#endif //JARVIS_ACTIONANALYTICS_H
