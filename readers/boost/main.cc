#include <iostream>
#include <string>
#include <limits>
#include <fstream>
#include <algorithm>
#include <tr1/functional_hash.h>
#include <ext/hash_map>
#include <boost/spirit/core.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/adjacency_list.hpp>

using namespace boost::spirit;
using namespace boost;
using namespace std;

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

typedef boost::adjacency_list<boost::listS, boost::vecS, boost::directedS, Node, Edge > Graph;
typedef boost::graph_traits<Graph>::vertex_descriptor node_t;
typedef boost::graph_traits<Graph>::edge_descriptor edge_t;

typedef __gnu_cxx::hash_map<uint64_t, node_t, tr1::hash<uint64_t> > node_map_t;

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
        default: return numeric_limits<float>::max();
    }
}

float bike_duration(int bike_property, int car_property, float length)
{
    switch(bike_property)
    {
        case 0: return numeric_limits<float>::max(); break;
        case 2: if(car_property < 5)
                    return length * 18/3.6;
                else
                    return numeric_limits<float>::max();
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
        default: return numeric_limits<float>::max();
    }
}

int main(int argc, char ** argv)
{
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

            e.elevation = max(0, g[target_n].elevation - g[source_n].elevation);
            e.bike_duration = bike_duration(bike, car, length);
            e.car_duration = car_duration(car, length);
            e.bike_comfort = bike_comfort(bike, car, length);
            add_edge(source_n, target_n, e, g);

            e.elevation = max(0, g[source_n].elevation - g[target_n].elevation);
            e.bike_duration = bike_duration(bike_reverse, car_reverse, length);
            e.bike_comfort = bike_comfort(bike_reverse, car_reverse, length);
            e.car_duration = car_duration(car_reverse, length);
            add_edge(target_n, source_n, e, g);
        }
    }
    edges.close();
    cout << "Read " << num_edges(g) << " edges" << endl;

    // Example using Dijkstra's algorithm
    // Will contain for every node it's precessor. p[u] == u if there is no path to that node
    vector<node_t> p(num_vertices(g)); 
    // Will contain the distance from the source to each node
    vector<float> d(num_vertices(g));
    dijkstra_shortest_paths(g,
            0, // source node
            // You can choose the attribute from Edge to minimize
            weight_map(get(&Edge::distance, g))
            .predecessor_map(&p[0])
            .distance_map(&d[0]));

    return(0);
}

