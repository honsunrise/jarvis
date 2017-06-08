//
// Created by zhsyourai on 5/22/17.
//

#include <vector>
#include <iostream>
#include "ActionAnalytics.h"

class Visitor : public boost::default_dfs_visitor {
public:
    Visitor(ActionAnalytics *analytics, Action &action) : analytics(analytics), action(action) {}

    template<class Vertex, class Graph>
    void discover_vertex(Vertex v, const Graph &g) {
        analytics->discover_vertex(v, action);
        return;
    }

    template<class Edge, class Graph>
    void examine_edge(Edge u, const Graph &g) {
        analytics->examine_edge(u, action);
        return;
    }

    template<class Edge, class Graph>
    void back_edge(Edge u, const Graph& g) {
        analytics->back_edge(u, action);
    }

private:
    ActionAnalytics *analytics;
    Action &action;
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
    Visitor vis(this, action);
    std::vector<boost::default_color_type> color_map_r(boost::num_vertices(sem_graph_tree));
    auto color_map = boost::make_iterator_property_map(color_map_r.begin(), t_vertex_index);
    boost::depth_first_search(sem_graph_tree, vis, color_map, tree_root);
    return action;
}

void ActionAnalytics::buildGraph(std::vector<CONLL> conlls) {
    sem_graph.clear();
    for (auto conll : conlls) {
        vertex_t vertex = boost::add_vertex(sem_graph);
        g_vertex_context[vertex] = conll.content + "|" + conll.pos;
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
        t_vertex_context[vertex] = conll.content + "|" + conll.pos;
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

std::string ActionAnalytics::VToAction(std::string text) {
    bool open = std::find(OPEN_LIST.begin(), OPEN_LIST.end(), text) != OPEN_LIST.end();
    bool close = !open && std::find(CLOSE_LIST.begin(), CLOSE_LIST.end(), text) != CLOSE_LIST.end();
    bool setting = !open && !close && std::find(SETTING_LIST.begin(), SETTING_LIST.end(), text) != SETTING_LIST.end();
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

void ActionAnalytics::discover_vertex(vertex_t v, Action &action) {
}

std::pair<std::string, std::string> ActionAnalytics::getVertextContext(vertex_t v) {
    std::pair<std::string, std::string> pair;
    std::string text = t_vertex_context[v];
    pair.first = text.substr(0, text.find('|'));
    pair.second = text.substr(text.find('|') + 1);
    return pair;
};

void ActionAnalytics::back_edge(ActionAnalytics::edge_t u, Action &action) {

}

void ActionAnalytics::examine_edge(edge_t u, Action &action) {
    static edge_t last_edge;
    vertex_t s = boost::source(u, sem_graph_tree);
    vertex_t t = boost::target(u, sem_graph_tree);
    std::string u_text = t_edge_relate[u];
    std::string s_text = t_vertex_context[s];
    std::string s_text_1 = getVertextContext(s).first;
    std::string s_text_2 = getVertextContext(s).second;
    std::string t_text = t_vertex_context[t];
    std::string t_text_1 = getVertextContext(t).first;
    std::string t_text_2 = getVertextContext(t).second;

    if (t_edge_relate(last_edge) == "eSucc") {
        if(u_text == "Agt") {
            parse_map[s].text = s_text_1;
            parse_map[s].pos = A;
            parse_map[t].text = t_text_1;
            parse_map[t].pos = S;
        } else if (u_text == "Pat") {
            parse_map[s].text = s_text_1;
            parse_map[s].pos = A;
            parse_map[t].text = t_text_1;
            parse_map[t].pos = T;
        } else if (u_text == "Feat") {
            parse_map[s].text = t_text_1 + s_text_1;
        } else if (u_text == "Clas") {
        }
    }

    last_edge = u;
    BOOST_LOG_TRIVIAL(info) << s_text << " --- " << u_text << " ---> " << t_text;
}
