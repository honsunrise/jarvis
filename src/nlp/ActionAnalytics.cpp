//
// Created by zhsyourai on 5/22/17.
//

#include <vector>
#include "ActionAnalytics.h"

Action ActionAnalytics::analytics(std::vector<CONLL> conlls) {
    Action action;
    std::string nh;
    auto root = search_semrelate(conlls, "Root");
    action.action = rootToAction(root);
    auto &&sems = search_semparent(conlls, root.id);
    auto agt = search_semrelate(sems, "Agt");
    auto pat = search_semrelate(sems, "Pat");
    if (pat) {
        auto pats = search_semparent(conlls, pat.id);
        auto nmod = search_semrelate(pats, "Nmod");
        action.target = nmod.content + pat.content;
        auto eSucc = search_semrelate(sems, "eSucc");
        auto eSuccs = search_semparent(conlls, eSucc.id);
        int i = 0;
        for (auto e : eSuccs) {
            action.params["p" + i++] = e.content;
        }

    }
    return action;
}

std::string ActionAnalytics::rootToAction(CONLL root) {
    bool open = std::find(OPEN_LIST.begin(), OPEN_LIST.end(), root.content) != OPEN_LIST.end();
    bool close = !open && std::find(CLOSE_LIST.begin(), CLOSE_LIST.end(), root.content) != CLOSE_LIST.end();
    bool setting = !open && !close && std::find(SETTING_LIST.begin(), SETTING_LIST.end(), root.content) != SETTING_LIST.end();
    if (open)
        return "OPEN";
    else if (close)
        return "CLOSE";
    else if (setting)
        return "SETTING";
    return "NONE";
}
