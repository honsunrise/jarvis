//
// Created by zhsyourai on 5/22/17.
//

#ifndef JARVIS_ACTIONANALYTICS_H
#define JARVIS_ACTIONANALYTICS_H

#include <map>
#include "NLP.h"

struct Action {
    std::string action;
    std::string target;
    std::map<std::string, std::string> params;
};

class ActionAnalytics {
public:
    Action analytics(std::vector<CONLL> conlls);

private:
    std::string rootToAction(CONLL root);

    std::vector<std::string> OPEN_LIST{
            "打开"
    };
    std::vector<std::string> CLOSE_LIST{
            "关闭", "关上", "关"
    };
    std::vector<std::string> SETTING_LIST{
            "设置", "设"
    };
};


#endif //JARVIS_ACTIONANALYTICS_H
