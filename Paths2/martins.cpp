#include "martins.h"

#include <iostream>
#include <string>
#include <fstream>
#include <deque>
#include <boost/spirit/include/classic_core.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include "boost/multi_index/hashed_index.hpp"
#include <boost/multi_index/member.hpp>
#include <boost/array.hpp>

using namespace boost::multi_index;
using namespace boost::spirit::classic;
using namespace boost;
using namespace std;


struct Label
{
    node_t node;
    vector<float> cost;
    node_t pred;
    size_t pred_idx;

    Label(size_t nb_objectives) : node(0), cost(nb_objectives, 0), pred(0), pred_idx(0)
    {
    }

    bool operator==(const Label & l)
    {
        return pred == l.pred && cost == l.cost;
    }

    bool operator<(const Label & l)
    {
        return cost < l.cost;
    }

    bool dominates(const Label & l) const
    {
        bool strict = false;
        for(size_t i = 0; i < cost.size(); i++)
        {
            if(this->cost[i] > l.cost[i])
                return false;
            if(!strict && this->cost[i] < l.cost[i])
                strict = true;
        }
        return strict;
    }
};


typedef multi_index_container<
        Label,
        indexed_by<
        ordered_non_unique<member<Label, vector<float>, &Label::cost> >,
        hashed_non_unique<member<Label, node_t, &Label::node > >
        >
        > my_queue;


typedef my_queue::nth_index<0>::type& queue_by_cost;
typedef my_queue::nth_index<1>::type& queue_by_node;

ostream & operator<<(ostream & os, Label l)
{
    os << "Label[" << l.node << "] {" << l.cost[0] << ";" << l.cost[1] << "} [" << l.pred << "]";
    return os;
}

bool is_dominated_by_any(const my_queue & Q, const Label & l)
{
    my_queue::nth_index<1>::type::iterator it, end;
    tie(it, end) = Q.get<1>().equal_range(l.node);
    for(; it != end; it++)
    {
        if( it->dominates(l) || it->cost == l.cost)
            return true;
    }
    return false;
}

bool is_dominated_by_any(const deque<Label> & llist, const Label & l)
{
    for(deque<Label>::const_iterator it = llist.begin(); it != llist.end(); it++)
    {
        if(it->dominates(l) || it->cost == l.cost)
            return true;
    }
    return false;
}


vector<Path> martins(node_t start_node, node_t dest_node, MultimodalGraph & g, int start_time, int nb_objectives)
{
    vector<deque<Label> > P(num_vertices(g.graph()));
    my_queue Q;

    Label start(nb_objectives);
    start.node = start_node;
    start.cost[0] = start_time;
    start.pred = start_node;
    Q.insert(start);
    const queue_by_cost cost_q_it = Q.get<0>();
    const queue_by_node node_q = Q.get<1>();

    while( !Q.empty() )
    {
        Label l = *(cost_q_it.begin());
        Q.erase(cost_q_it.begin());
        P[l.node].push_back(l);
        graph_traits<Graph_t>::out_edge_iterator ei, end;
        tie(ei,end) = out_edges(l.node, g.graph());
        for(; ei != end; ei++)
        {
            Label l2(nb_objectives);
            l2.pred = l.node;
            l2.node = boost::target(*ei,g.graph());
            l2.pred_idx = P[l.node].size() - 1;

            l2.cost[0] = g.graph()[*ei].duration(l.cost[0]);

            if(nb_objectives >= 2)
                l2.cost[1] = l.cost[1] + g.graph()[*ei].nb_changes;

            if(nb_objectives >= 3)
                l2.cost[2] = l.cost[2] + g.graph()[*ei].elevation;
            if(nb_objectives >= 4)
                l2.cost[3] += l.cost[3] + g.graph()[*ei].cost;



            if(!is_dominated_by_any(Q, l2) && !is_dominated_by_any(P[l2.node],l2) && (dest_node == invalid_node || !is_dominated_by_any(P[dest_node],l2)))
            {
                my_queue::nth_index<1>::type::iterator it, end;
                tie(it, end) = node_q.equal_range(l2.node);
                while(it != end)
                {
                    if(l2.dominates(*it) )
                        it = Q.get<1>().erase(it);
                    else
                    {
                        it++;
                    }
                }
                Q.insert(l2);
            }
        }
    }

    vector<Path> ret;
    if(dest_node != invalid_node)
    {
        deque<Label>::const_iterator it;
        for(it = P[dest_node].begin(); it != P[dest_node].end(); it++)
        {
            Path p;
            p.cost.push_back(it->cost[0]);
            if(nb_objectives >=2)
                p.cost.push_back(it->cost[1]);
            if(nb_objectives >= 3)
                p.cost.push_back(it->cost[2]);
            if(nb_objectives >= 4)
                p.cost.push_back(it->cost[3]);
            Label last = *it;
            p.nodes.push_front(g.graph()[last.node]);
            while(last.node != start.node)
            {
                last = P[last.pred][last.pred_idx];
                p.nodes.push_front(g.graph()[last.node]);
            }
            ret.push_back(p);
        }
    }
    return ret;
}
