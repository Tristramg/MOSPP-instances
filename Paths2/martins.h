#ifndef MARTINS_H
#define MARTINS_H

#include "MultimodalGraph.h"


struct Path
{
    std::vector<float> cost;
    std::list<Node> nodes;
    size_t size() const
    {
        return nodes.size();
    }
};

std::vector<Path> martins(node_t start, node_t dest, MultimodalGraph & g, int start_time = 30000);
std::vector<Path> martins(node_t start, node_t dest, MultimodalGraph & g, float Edge::*, int start_time = 30000);
std::vector<Path> martins(node_t start, node_t dest, MultimodalGraph & g, float Edge::*, float Edge::*, int start_time = 30000);
std::vector<Path> martins(node_t start, node_t dest, MultimodalGraph & g, float Edge::*, float Edge::*, float Edge::*, int start_time = 30000);

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

template<size_t N>
struct Label
{
    node_t node;
    array<float, N> cost;
    node_t pred;
    size_t pred_idx;

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
        for(size_t i = 0; i < N; i++)
        {
            if(this->cost[i] > l.cost[i])
                return false;
            if(!strict && this->cost[i] < l.cost[i])
                strict = true;
        }
        return strict;
    }
};

template<size_t N>
        struct my_queue
{

    typedef multi_index_container<
            Label<N>,
            indexed_by<
            ordered_non_unique<member<Label<N>, array<float, N>, &Label<N>::cost> >,
            hashed_non_unique<member<Label<N>, node_t, &Label<N>::node > >
            >
            > Type;

    typedef typename nth_index<my_queue<N>::Type, 0>::type& by_cost;
    typedef typename nth_index<my_queue<N>::Type, 1>::type& by_nodes;
    typedef typename nth_index<my_queue<N>::Type, 0>::type::iterator cost_it;
    typedef typename nth_index<my_queue<N>::Type, 1>::type::iterator nodes_it;
};



template<size_t N>
ostream & operator<<(ostream & os, Label<N> l)
{
    os << "Label[" << l.node << "] {" << l.cost[0] << ";" << l.cost[1] << "} [" << l.pred << "]";
    return os;
}

template<size_t N>
bool is_dominated_by_any(const typename my_queue<N>::Type & Q, const Label<N> & l)
{
    typename my_queue<N>::nodes_it it, end;
    tie(it, end) = Q.get<1>().equal_range(l.node);
    for(; it != end; it++)
    {
        if( it->dominates(l) || it->cost == l.cost)
            return true;
    }
    return false;
}

template<size_t N>
bool is_dominated_by_any(const deque< Label<N> > & llist, const Label<N> & l)
{
    typedef typename deque< Label<N> >::const_iterator Iterator;
    for(Iterator it = llist.begin(); it != llist.end(); it++)
    {
        if(it->dominates(l) || it->cost == l.cost)
            return true;
    }
    return false;
}

template<size_t N>
vector<Path> martins(node_t start_node, node_t dest_node, MultimodalGraph & g, int start_time, std::vector<float Edge::*> objectives)
{
    vector< deque< Label<N> > > P(num_vertices(g.graph()));
    typename my_queue<N>::Type Q;

    Label<N> start;
    start.node = start_node;
    start.cost[0] = start_time;
    start.pred = start_node;
    Q.insert(start);
    const typename my_queue<N>::by_cost cost_q_it = Q.get<0>();
    const typename my_queue<N>::by_nodes node_q = Q.get<1>();

    while( !Q.empty() )
    {
        Label<N> l = *(cost_q_it.begin());
        Q.erase(cost_q_it.begin());
        P[l.node].push_back(l);
        graph_traits<Graph_t>::out_edge_iterator ei, end;
        tie(ei,end) = out_edges(l.node, g.graph());
        for(; ei != end; ei++)
        {
            Label<N> l2;
            l2.pred = l.node;
            l2.node = boost::target(*ei,g.graph());
            l2.pred_idx = P[l.node].size() - 1;

            l2.cost[0] = g.graph()[*ei].duration(l.cost[0]);
            for(size_t i=1; i < N; i++)
                l2.cost[i] = l.cost[i] + g.graph()[*ei].*objectives[i-1];

            if(!is_dominated_by_any(Q, l2) && !is_dominated_by_any(P[l2.node],l2) && (dest_node == invalid_node || !is_dominated_by_any(P[dest_node],l2)))
            {
                typename my_queue<N>::nodes_it it, end;
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
        typename deque< Label<N> >::const_iterator it;
        for(it = P[dest_node].begin(); it != P[dest_node].end(); it++)
        {
            Path p;
            p.cost.push_back(it->cost[0]);
            if(N >=2)
                p.cost.push_back(it->cost[1]);
            if(N >= 3)
                p.cost.push_back(it->cost[2]);
            if(N >= 4)
                p.cost.push_back(it->cost[3]);
            Label<N> last = *it;
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



#endif // MARTINS_H
