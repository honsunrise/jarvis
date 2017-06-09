//
// Created by zhsyourai on 6/9/17.
//

#include "PatternInspired.h"

PatternInspired::PatternInspired() {}

PatternInspired::~PatternInspired() {

}

void PatternInspired::start() {

}

void PatternInspired::feed(std::string source, std::string relate, std::string target) {
    if(relate == "Agt") {
        source_vertex = t;
        action.source = t_text_1;
    } else if (relate == "Pat") {
        target_vertex = t;
        action.target = t_text_1;
        action.action = VToAction(s_text_1);
    } else if (relate == "Feat") {
        if (boost::source(u, sem_graph_tree) == target_vertex)
            action.params[DESC].push_back(t_text_1);
    } else if (relate == "eSucc") {
        eSucc_source = s;
        eSucc_target = t;
    } else if (relate == "Clas") {
        if(eSucc_target == s) {
            action.params[PToKey(getVertextContext(eSucc_source).first)].push_back(t_text_1);
        } else {
            action.params[PToKey(s_text_1)].push_back(t_text_1);
        }
    }
    last_edge = u;
}

Action PatternInspired::end() {

}
