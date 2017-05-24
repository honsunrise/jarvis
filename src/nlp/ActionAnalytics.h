//
// Created by zhsyourai on 5/22/17.
//

#ifndef JARVIS_ACTIONANALYTICS_H
#define JARVIS_ACTIONANALYTICS_H

#include "NLP.h"
#include <algorithm>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_utility.hpp>
#include <boost/graph/graph_as_tree.hpp>
#include <boost/graph/visitors.hpp>


struct Action {
    std::string action;
    std::string target;
    std::map<std::string, std::vector<std::string>> params;
};

class ActionAnalytics {
public:
    typedef boost::property<boost::edge_name_t, std::string> relate_t;
    typedef boost::property<boost::vertex_name_t, std::string, boost::property<boost::vertex_index2_t, int>> context_t;
    typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS, context_t, relate_t> graph_t;
    typedef boost::graph_traits<graph_t>::vertex_descriptor vertex_t;
    typedef boost::graph_traits<graph_t>::edge_descriptor edge_t;
    typedef boost::graph_traits<graph_t>::vertex_iterator vertex_iterator_t;
    typedef boost::property_map<graph_t, boost::vertex_index_t>::type vertex_index_t;
    typedef boost::property_map<graph_t, boost::vertex_name_t>::type vertex_name_t;
    typedef boost::property_map<graph_t, boost::vertex_index2_t>::type vertex_index2_t;
    typedef boost::property_map<graph_t, boost::edge_name_t>::type edge_name_t;
    typedef boost::iterator_property_map<std::vector<boost::default_color_type >::iterator, vertex_index_t> color_map_t;
    typedef boost::iterator_property_map<std::vector<vertex_t >::iterator, vertex_index_t> parent_map_t;
    typedef boost::graph_as_tree<graph_t, parent_map_t> tree_t;

    ActionAnalytics();

    Action analytics(std::vector<CONLL> conlls);

private:
    vertex_index_t g_vertex_index;
    vertex_index_t t_vertex_index;

    vertex_name_t g_vertex_context;
    vertex_name_t t_vertex_context;

    vertex_index2_t g_vertex_id;
    vertex_index2_t t_vertex_id;

    edge_name_t g_edge_relate;
    edge_name_t t_edge_relate;

    std::string rootToAction(CONLL root);

    void buildGraph(std::vector<CONLL> conlls);

    void buildTree(std::vector<CONLL> conlls);

    vertex_t findTreeRoot();

    std::vector<std::string> OPEN_LIST{
            "打开"
    };
    std::vector<std::string> CLOSE_LIST{
            "关闭", "关上", "关"
    };
    std::vector<std::string> SETTING_LIST{
            "设置", "设"
    };

    graph_t sem_graph;
    graph_t sem_graph_tree;
    vertex_t tree_root;
};


#endif //JARVIS_ACTIONANALYTICS_H
