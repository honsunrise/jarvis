//
// Created by zhsyourai on 5/22/17.
//

#include <vector>
#include "ActionAnalytics.h"

Action ActionAnalytics::analytics(std::vector<CONLL> conlls) {
    Action action;
    std::string nh;
    auto root = search_semrelate(conlls, "Root");
    auto sems = search_semparent(conlls, root.id);
    auto agt = search_semrelate(sems, "Agt");
    auto pat = search_semrelate(sems, "Pat");
    BOOST_LOG_TRIVIAL(info) << agt.content;
    BOOST_LOG_TRIVIAL(info) << pat.content;
    return action;
}
