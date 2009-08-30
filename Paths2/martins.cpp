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


template<size_t nb_objectives> struct LabelN
{
    static const size_t obj = nb_objectives;
    node_t node;
    array<float, nb_objectives> cost;
    node_t pred;
    size_t pred_idx;


    bool operator==(const LabelN & l)
    {
        return pred == l.pred && cost == l.cost;
    }

    bool operator<(const LabelN & l)
    {
        return cost < l.cost;
    }

    bool dominates(const LabelN & l) const
    {
        bool strict = false;
        for(size_t i = 0; i < nb_objectives; i++)
        {
            if(this->cost[i] > l.cost[i])
                return false;
            if(!strict && this->cost[i] < l.cost[i])
                strict = true;
        }
        return strict;
    }
};

typedef LabelN<2> Label;
typedef multi_index_container<
        Label,
        indexed_by<
        ordered_non_unique<member<Label, array<float, Label::obj>, &Label::cost> >,
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
        if( it->dominates(l) )
            return true;
    }
    return false;
}

bool is_dominated_by_any(const deque<Label> & llist, const Label & l)
{
    for(deque<Label>::const_iterator it = llist.begin(); it != llist.end(); it++)
    {
        if(it->dominates(l))
            return true;
    }
    return false;
}


vector<Path> martins(node_t start_node, node_t dest_node, MultimodalGraph & g, int start_time)
{
    vector<deque<Label> > P(num_vertices(g.graph()));
    my_queue Q;

    Label start;
    start.node = start_node;
    for(size_t i=0; i < start.cost.size(); i++)
        start.cost[i] = 0;
    start.cost[0] = start_time;
    start.pred = start_node;
    Q.insert(start);
    const queue_by_cost cost_q_it = Q.get<0>();
    const queue_by_node node_q = Q.get<1>();
    bool finished = false;

    while( !Q.empty() && !finished)
    {
        Label l = *(cost_q_it.begin());
        Q.erase(cost_q_it.begin());
        P[l.node].push_back(l);
        graph_traits<Graph_t>::out_edge_iterator ei, end;
        tie(ei,end) = out_edges(l.node, g.graph());
        for(; ei != end; ei++)
        {
            Label l2;
            l2.node = boost::target(*ei,g.graph());
            l2.cost = l.cost;
            l2.pred = l.node;
            l2.pred_idx = P[l.node].size() - 1;
            l2.cost[0] = g.graph()[*ei].duration(l.cost[0]);
            l2.cost[1] = l.cost[1] + g.graph()[*ei].nb_changes;

            bool to_insert = true;
            if(!is_dominated_by_any(Q, l2) && !is_dominated_by_any(P[l2.node],l2) && !is_dominated_by_any(P[dest_node],l2))
            {
                my_queue::nth_index<1>::type::iterator it, end;
                tie(it, end) = node_q.equal_range(l2.node);
                while(it != end)
                {
                    if(l2.dominates(*it) )
                        it = Q.get<1>().erase(it);
                    else
                    {
                        if(l2.cost == it->cost)
                            to_insert = false;
                        it++;
                    }
                }
                if(to_insert)
                    Q.insert(l2);
            }
        }
    }

    vector<Path> ret;
    deque<Label>::const_iterator it;
    for(it = P[dest_node].begin(); it != P[dest_node].end(); it++)
    {
        Path p;
        p.cost.push_back(it->cost[0]);
        p.cost.push_back(it->cost[1]);
        //  p.cost.push_back(it->cost[2]);
        Label last = *it;
        p.nodes.push_front(g.graph()[last.node]);
        while(last.node != start.node)
        {
            last = P[last.pred][last.pred_idx];
            p.nodes.push_front(g.graph()[last.node]);
        }
        ret.push_back(p);
    }
    return ret;
}
