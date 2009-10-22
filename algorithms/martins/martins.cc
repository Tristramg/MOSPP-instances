#include <iostream>
#include <string>
#include <limits>
#include <fstream>
#include <tr1/functional_hash.h>
#include <ext/hash_map>
#include <boost/spirit/core.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/array.hpp>
#include <deque>
#include "boost/multi_index/hashed_index.hpp" 

using namespace boost::multi_index;
using namespace boost::spirit;
using namespace boost;
using namespace std;

// Part 1: defining the graph structure
struct Node
{
    uint64_t id;
    int elevation;
    float lon;
    float lat;
};

struct Edge
{
    float distance;
    float bike_duration;
    float car_duration;
    float foot_duration;
    float elevation;
    float bike_comfort;
};

struct not_accessible{};

typedef boost::adjacency_list<boost::listS, boost::vecS, boost::directedS, Node, Edge > Graph;
typedef boost::graph_traits<Graph>::vertex_descriptor node_t;
typedef boost::graph_traits<Graph>::edge_descriptor edge_t;

typedef __gnu_cxx::hash_map<uint64_t, node_t, tr1::hash<uint64_t> > node_map_t;


//Part 2: defining helping functions to calculate costs
float car_duration(int property, float length)
{
    switch(property)
    {
        case 1: return 20 * length / 3.6; break;
        case 2: return 30 * length / 3.6; break;
        case 3: return 50 * length / 3.6; break;
        case 4: return 90 * length / 3.6; break;
        case 5: return 100 * length / 3.6; break;
        case 6: return 120 * length / 3.6; break;
        default: throw not_accessible();
    }
}

float bike_duration(int bike_property, int car_property, float length)
{
    switch(bike_property)
    {
        case 0:  throw not_accessible(); break;
        case 2: if(car_property < 5)
                    return length * 18/3.6;
                else
                    throw not_accessible();
                break;
        default: return length * 18/3.6;
    }
}

float bike_comfort(int bike_property, int car_property, float length)
{
    switch(bike_property)
    {
        case 1:
        case 3: return length; break;
        case 2: return car_property * length; break;
        case 4: return length * 1.5; break;
        case 5: return 0.5 * length; break;
        default: throw not_accessible();
    }
}

// Part 3: defining datastructures for Martins' algorithm
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
            if(this->cost[i] < l.cost[i])
                strict = true;
        }
        return strict;
    }
};

typedef LabelN<3> Label;

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

int main(int argc, char ** argv)
{
    //Step 1: parsing the data
    Graph g;
    node_map_t node_map;

    ifstream nodes("nodes.csv");
    string s;
    getline(nodes, s); // We skip the header
    uint64_t id;
    int elevation;
    float lon, lat;
    rule<> nodes_rules = int_p[assign_a(id)] >> ',' >>
        real_p[assign_a(lon)] >> ',' >>
        real_p[assign_a(lat)] >> ',' >>
        int_p[assign_a(elevation)];
    while( getline(nodes, s) )
    {
        Node n;
        if(!parse(s.c_str(), nodes_rules, space_p).full)
            cerr << " Parsing error. Line was: " << s << endl;
        else
        {
            n.id = id;
            n.lon = lon;
            n.lat = lat;
            n.elevation = elevation;
            if(elevation < 1)
                cout << "Damn ! " << elevation << endl;
            node_map[id] = add_vertex(n, g);
        }
    }
    nodes.close();
    cout << "Read " << num_vertices(g) << " nodes" << endl;

    ifstream edges("edges.csv");
    getline(edges, s); // We skip the header
    uint64_t source, target;
    float length;
    int car, car_reverse, bike, bike_reverse, foot;
    rule<> edges_rules = int_p[assign_a(id)] >> ',' >>
        int_p[assign_a(source)] >> ',' >>
        int_p[assign_a(target)] >> ',' >>
        real_p[assign_a(length)] >> ',' >>
        int_p[assign_a(car)] >> ',' >>
        int_p[assign_a(car_reverse)] >> ',' >>
        int_p[assign_a(bike)] >> ',' >>
        int_p[assign_a(bike_reverse)] >> ',' >>
        int_p[assign_a(foot)];

    while( getline(edges,s) )
    {
        if(!parse(s.c_str(), edges_rules, space_p).hit)
            cerr << " Parsing error. Line was: " << s << endl;
        else
        {
            Edge e;
            node_t source_n = node_map[source];
            node_t target_n = node_map[target];
            e.distance = length;
            e.foot_duration = length * 4/3.6;
            try
            {
                e.elevation = max(0, g[target_n].elevation - g[source_n].elevation);
                e.bike_duration = bike_duration(bike, car, length);
         //       e.car_duration = car_duration(car, length);
                e.bike_comfort = bike_comfort(bike, car, length);
                add_edge(source_n, target_n, e, g);
            }
            catch(not_accessible e)
            {}

            try
            {
                e.elevation = max(0, g[source_n].elevation - g[target_n].elevation);
                e.bike_duration = bike_duration(bike_reverse, car_reverse, length);
                e.bike_comfort = bike_comfort(bike_reverse, car_reverse, length);
           //     e.car_duration = car_duration(car_reverse, length);
                add_edge(target_n, source_n, e, g);
            }
            catch(not_accessible e)
            {}
        }
    }
    edges.close();
    cout << "Read " << num_edges(g) << " edges" << endl;

    // Step 2: running the algorithm
    vector<deque<Label> > P(num_vertices(g));
    my_queue Q;

    Label start;
    start.node = 0;
    for(size_t i=0; i < start.cost.size(); i++)
        start.cost[i] = 0;
    start.pred = 0;
    Q.insert(start);
    const queue_by_cost cost_q_it = Q.get<0>();
    const queue_by_node node_q = Q.get<1>();
    bool finished = false;

    while( !Q.empty() && !finished)
    {
        Label l = *(cost_q_it.begin());
        Q.erase(cost_q_it.begin());
        P[l.node].push_back(l);
        graph_traits<Graph>::out_edge_iterator ei, end;
        tie(ei,end) = out_edges(l.node, g);
        for(; ei != end; ei++)
        {
            Label l2;
            l2.node = boost::target(*ei,g);
            l2.cost = l.cost;
            l2.pred = l.node;
            l2.pred_idx = P[l.node].size();
            l2.cost[0] = l.cost[0] + g[*ei].distance;
            l2.cost[1] = l.cost[1] + g[*ei].elevation;
            l2.cost[2] = l.cost[2] + g[*ei].bike_comfort;
            if(!is_dominated_by_any(Q, l2) && !is_dominated_by_any(P[l2.node],l2) && !is_dominated_by_any(P[524],l2))
            {
                bool insert = true;
                my_queue::nth_index<1>::type::iterator it, end;
                tie(it, end) = node_q.equal_range(l2.node);
                while(it != end)
                {
                    if(l2.dominates(*it) )
                        it = Q.get<1>().erase(it);
                    else
                    {
                        it++;
                        if(l2.cost == it->cost)
                            insert = false;
                    }
                }
                if(insert)
                {
                    Q.insert(l2);
//                    cout << "   Inserted " << l2 << endl;
                }
            }
        }
    }

    //Step 3: displaying how many labels we have on each node
    for (size_t i = 0; i < P.size(); i+=P.size()/10)
    {
        cout << "node: " << i << " " << g[i].id << endl;
        for(deque<Label>::iterator it = P[i].begin(); it != P[i].end(); it++)
            cout << "  - [" << it->cost[0] << ", " << it->cost[1] << ", " << it->cost[2]
                << "]" << endl;

        //        cout << i << "[" << P[i].size() << "] ";
    }

    return(0);
}

