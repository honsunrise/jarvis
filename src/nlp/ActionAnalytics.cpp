//
// Created by zhsyourai on 5/22/17.
//

#include <vector>
#include <iostream>
#include "ActionAnalytics.h"
template<class Graph>
class Visitor: public boost::default_dfs_visitor
{
public:
    typedef typename
    boost::graph_traits<Graph>::vertex_descriptor Vertex;

    void discover_vertex(Vertex v, const Graph& g)
    {std::cout << v << " "; return;}

    void start_vertex(Vertex v, const Graph& g)
    {std::cout << v << " "; return;}
};


Action ActionAnalytics::analytics(std::vector<CONLL> conlls) {
    Action action;
    buildGraph(conlls);
    buildTree(conlls);
    Vertex root = findTreeRoot();
    Visitor<Graph> vis;
    typedef std::vector<boost::default_color_type> color_map_t;
    color_map_t color_map;
    boost::depth_first_visit(sem_tree, root, boost::visitor(vis), boost::vertex_color_map(color_map));
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
        Vertex vertex = boost::add_vertex(sem_graph);
        vertex_context[vertex] = conll.content;
        vertex_index2[vertex] = conll.id;
    }
    for (auto conll : conlls) {
        int search_done = 0;
        Vertex u1;
        VertexIterator vertexIt, vertexEnd;
        boost::tie(vertexIt, vertexEnd) = boost::vertices(sem_graph);
        for (; vertexIt != vertexEnd; ++vertexIt) {
            if (vertex_index2[*vertexIt] == conll.id) {
                u1 = *vertexIt;
                search_done++;
                break;
            }
        }
        for (auto sem : conll.sem) {
            Vertex u2;
            boost::tie(vertexIt, vertexEnd) = boost::vertices(sem_graph);
            for (; vertexIt != vertexEnd; ++vertexIt) {
                if (vertex_index2[*vertexIt] == sem.parent) {
                    u2 = *vertexIt;
                    search_done++;
                    break;
                }
            }
            if(search_done == 2) {
                Edge edge = boost::add_edge(u2, u1, sem_graph).first;
                edge_relate[edge] = sem.relate;
            }
        }
    }
}

void ActionAnalytics::buildTree(std::vector<CONLL> conlls) {
    sem_tree.clear();
    for (auto conll : conlls) {
        Vertex vertex = boost::add_vertex(sem_tree);
        vertex_context[vertex] = conll.content;
        vertex_index2[vertex] = conll.id;
    }
    for (auto conll : conlls) {
        int search_done = 0;
        Vertex u1;
        VertexIterator vertexIt, vertexEnd;
        boost::tie(vertexIt, vertexEnd) = boost::vertices(sem_tree);
        for (; vertexIt != vertexEnd; ++vertexIt) {
            if (vertex_index2[*vertexIt] == conll.id) {
                u1 = *vertexIt;
                search_done++;
                break;
            }
        }
        Vertex u2;
        boost::tie(vertexIt, vertexEnd) = boost::vertices(sem_tree);
        for (; vertexIt != vertexEnd; ++vertexIt) {
            if (vertex_index2[*vertexIt] == conll.semparent) {
                u2 = *vertexIt;
                search_done++;
                break;
            }
        }
        if(search_done == 2) {
            Edge edge = boost::add_edge(u2, u1, sem_tree).first;
            edge_relate[edge] = conll.semrelate;
        }
    }
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

ActionAnalytics::Vertex ActionAnalytics::findTreeRoot() {
    VertexIterator vertexIt, vertexEnd;
    boost::tie(vertexIt, vertexEnd) = boost::vertices(sem_tree);
    for (; vertexIt != vertexEnd; ++vertexIt) {
        if (boost::in_degree(*vertexIt, sem_tree) == 0){
            return *vertexIt;
        }
    }
    return Vertex();
}
