//
// Created by zhsyourai on 5/22/17.
//

#ifndef JARVIS_ACTIONANALYTICS_H
#define JARVIS_ACTIONANALYTICS_H

#include "NLP.h"
#include <algorithm>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <map>


struct Action {
    std::string action;
    std::string target;
    std::map<std::string, std::vector<std::string>> params;
};

class ActionAnalytics {
    typedef boost::property<boost::edge_name_t, std::string> relate;
    typedef boost::property<boost::vertex_name_t, std::string, boost::property<boost::vertex_index2_t, int>> context;
    typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS, context, relate> Graph;
    typedef boost::graph_traits<Graph>::vertex_descriptor Vertex;
    typedef boost::graph_traits<Graph>::edge_descriptor Edge;
    typedef boost::graph_traits<Graph>::vertex_iterator VertexIterator;
public:
    Action analytics(std::vector<CONLL> conlls);

private:
    typename boost::property_map<Graph, boost::vertex_name_t>::type vertex_context =
            boost::get(boost::vertex_name, sem_graph);

    typename boost::property_map<Graph, boost::vertex_index2_t>::type vertex_index2 =
            boost::get(boost::vertex_index2, sem_graph);

    typename boost::property_map<Graph, boost::edge_name_t>::type edge_relate =
            boost::get(boost::edge_name, sem_graph);

    std::string rootToAction(CONLL root);

    void buildGraph(std::vector<CONLL> conlls);

    std::vector<std::string> OPEN_LIST{
            "打开"
    };
    std::vector<std::string> CLOSE_LIST{
            "关闭", "关上", "关"
    };
    std::vector<std::string> SETTING_LIST{
            "设置", "设"
    };

    Graph sem_graph;
};


#endif //JARVIS_ACTIONANALYTICS_H
