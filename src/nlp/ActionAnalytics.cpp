//
// Created by zhsyourai on 5/22/17.
//

#include <vector>
#include <iostream>
#include "ActionAnalytics.h"

class Visitor : public boost::default_dfs_visitor {
public:
    Visitor(ActionAnalytics *analytics) : analytics(analytics) {}

    template<class Vertex, class Graph>
    void discover_vertex(Vertex v, const Graph &g) {
        std::cout << v << " ";
        return;
    }

    template<class Vertex, class Graph>
    void start_vertex(Vertex v, const Graph &g) {
        std::cout << v << " ";
        return;
    }

    Action get_action() {
        return Action();
    }

private:
    ActionAnalytics *analytics;
};

ActionAnalytics::ActionAnalytics() {
    g_vertex_context = boost::get(boost::vertex_name, sem_graph);
    g_vertex_id = boost::get(boost::vertex_index2, sem_graph);
    g_edge_relate = boost::get(boost::edge_name, sem_graph);
    g_vertex_index = boost::get(boost::vertex_index, sem_graph);

    t_vertex_context = boost::get(boost::vertex_name, sem_graph_tree);
    t_vertex_id = boost::get(boost::vertex_index2, sem_graph_tree);
    t_edge_relate = boost::get(boost::edge_name, sem_graph_tree);
    t_vertex_index = boost::get(boost::vertex_index, sem_graph_tree);
}

Action ActionAnalytics::analytics(std::vector<CONLL> conlls) {
    Action action;
    buildGraph(conlls);
    buildTree(conlls);
    vertex_t root = findTreeRoot();
    Visitor vis(this);
    std::vector<boost::default_color_type> color_map_r(boost::num_vertices(sem_graph_tree));
    auto color_map = boost::make_iterator_property_map(color_map_r.begin(), t_vertex_index);
    boost::depth_first_search(sem_graph_tree, vis, color_map, root);
    action = vis.get_action();
//    std::string nh;
//    auto root = search_semrelate(conlls, "Root");
//    action.action = rootToAction(root);
//    auto &&sems = search_semparent(conlls, root.id);
//    auto agt = search_semrelate(sems, "Agt");
//    auto pat = search_semrelate(sems, "Pat");
//    if (pat) {
//        auto pats = search_semparent(conlls, pat.id);
//        auto nmod = search_semrelate(pats, "Nmod");
//        auto desc = search_semrelate(pats, "Feat");
//        action.target = nmod.content + pat.content;
//        auto eSucc = search_semrelate(sems, "eSucc");
//        if(eSucc) {
//            auto eSuccs = search_semparent(conlls, eSucc.id);
//            for (auto e : eSuccs) {
//                auto es = search_semparent(conlls, e.id);
//                std::for_each(es.begin(), es.end(), [&](CONLL &a){
//                    action.params[e.content].push_back(a.content);
//                });
//            }
//        }
//
//    }
    return action;
}

void ActionAnalytics::buildGraph(std::vector<CONLL> conlls) {
    sem_graph.clear();
    for (auto conll : conlls) {
        vertex_t vertex = boost::add_vertex(sem_graph);
        g_vertex_context[vertex] = conll.content;
        g_vertex_id[vertex] = conll.id;
    }
    for (auto conll : conlls) {
        int search_done = 0;
        vertex_t u1;
        vertex_iterator_t vertexIt, vertexEnd;
        boost::tie(vertexIt, vertexEnd) = boost::vertices(sem_graph);
        for (; vertexIt != vertexEnd; ++vertexIt) {
            if (g_vertex_id[*vertexIt] == conll.id) {
                u1 = *vertexIt;
                search_done++;
                break;
            }
        }
        for (auto sem : conll.sem) {
            vertex_t u2;
            boost::tie(vertexIt, vertexEnd) = boost::vertices(sem_graph);
            for (; vertexIt != vertexEnd; ++vertexIt) {
                if (g_vertex_id[*vertexIt] == sem.parent) {
                    u2 = *vertexIt;
                    search_done++;
                    break;
                }
            }
            if (search_done == 2) {
                edge_t edge = boost::add_edge(u2, u1, sem_graph).first;
                g_edge_relate[edge] = sem.relate;
            }
        }
    }
}

void ActionAnalytics::buildTree(std::vector<CONLL> conlls) {
    sem_graph_tree.clear();
    for (auto conll : conlls) {
        vertex_t vertex = boost::add_vertex(sem_graph_tree);
        t_vertex_context[vertex] = conll.content;
        t_vertex_id[vertex] = conll.id;
    }
    for (auto conll : conlls) {
        int search_done = 0;
        vertex_t u1;
        vertex_iterator_t vertexIt, vertexEnd;
        boost::tie(vertexIt, vertexEnd) = boost::vertices(sem_graph_tree);
        for (; vertexIt != vertexEnd; ++vertexIt) {
            if (t_vertex_id[*vertexIt] == conll.id) {
                u1 = *vertexIt;
                search_done++;
                break;
            }
        }
        vertex_t u2;
        boost::tie(vertexIt, vertexEnd) = boost::vertices(sem_graph_tree);
        for (; vertexIt != vertexEnd; ++vertexIt) {
            if (t_vertex_id[*vertexIt] == conll.semparent) {
                u2 = *vertexIt;
                search_done++;
                break;
            }
        }
        if (search_done == 2) {
            edge_t edge = boost::add_edge(u2, u1, sem_graph_tree).first;
            t_edge_relate[edge] = conll.semrelate;
        }
    }
    tree_root = findTreeRoot();
}

std::string ActionAnalytics::rootToAction(CONLL root) {
    bool open = std::find(OPEN_LIST.begin(), OPEN_LIST.end(), root.content) != OPEN_LIST.end();
    bool close = !open && std::find(CLOSE_LIST.begin(), CLOSE_LIST.end(), root.content) != CLOSE_LIST.end();
    bool setting =
            !open && !close && std::find(SETTING_LIST.begin(), SETTING_LIST.end(), root.content) != SETTING_LIST.end();
    if (open)
        return "OPEN";
    else if (close)
        return "CLOSE";
    else if (setting)
        return "SETTING";
    return "NONE";
}

ActionAnalytics::vertex_t ActionAnalytics::findTreeRoot() {
    vertex_iterator_t vertexIt, vertexEnd;
    boost::tie(vertexIt, vertexEnd) = boost::vertices(sem_graph_tree);
    for (; vertexIt != vertexEnd; ++vertexIt) {
        if (boost::in_degree(*vertexIt, sem_graph_tree) == 0) {
            return *vertexIt;
        }
    }
    return vertex_t();
}
