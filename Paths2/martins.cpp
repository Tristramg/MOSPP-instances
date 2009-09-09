#include "martins.h"

vector<Path> martins(node_t start_node, node_t dest_node, MultimodalGraph & g, int start_time)
{
    vector<float Edge::*> objectives;
    return martins<1>(start_node, dest_node, g, start_time, objectives);
}

vector<Path> martins(node_t start_node, node_t dest_node, MultimodalGraph & g, float Edge::*obj2, int start_time)
{
    vector<float Edge::*> objectives;
    objectives.push_back(obj2);
    return martins<2>(start_node, dest_node, g, start_time, objectives);
}

vector<Path> martins(node_t start_node, node_t dest_node, MultimodalGraph & g, float Edge::*obj2, float Edge::*obj3, int start_time)
{
    vector<float Edge::*> objectives;
    objectives.push_back(obj2);
    objectives.push_back(obj3);
    return martins<3>(start_node, dest_node, g, start_time, objectives);
}


vector<Path> martins(node_t start_node, node_t dest_node, MultimodalGraph & g, float Edge::*obj2, float Edge::*obj3, float Edge::*obj4, int start_time)
{
    vector<float Edge::*> objectives;
    objectives.push_back(obj2);
    objectives.push_back(obj3);
    objectives.push_back(obj4);
    return martins<4>(start_node, dest_node, g, start_time, objectives);
}
