#include "csvwriter.h"

using namespace std;

CSVWriter::CSVWriter()
{
    edges_file.open("edges.csv");
    edges_file << setprecision(9);
    edges_file << "\"edge_id\",\"source\",\"target\",\"length\",\"car\",\"car reverse\",\"bike\",\"bike reverse\",\"foot\",\"WKT\"" << endl;
}

int CSVWriter::save_nodes(const NodeMapType & nodes)
{
    ofstream nodes_file;
    nodes_file.open ("nodes.csv");
    // By default outstream only give 4 digits after the dot (~10m precision)
    nodes_file << setprecision(9);
    nodes_file << "\"node_id\",\"longitude\",\"latitude\",\"altitude\"" << endl;
    int nodes_inserted = 0;

    for(NodeMapType::const_iterator i = nodes.begin(); i != nodes.end(); i++)
    {
        if( (*i).second.uses > 1 )
        {
            nodes_file << (*i).first << "," <<
                (*i).second.lon << "," << 
                (*i).second.lat << "," << endl;
            nodes_inserted++;
        }
    }
    nodes_file.close();
    return nodes_inserted;
}

void CSVWriter::save_edge(int edge_id,
            node_t source, node_t target, float length,
            char car_direct, char car_rev,
            char bike_direct, char bike_rev,
            char foot,
            std::string geom)
{
    edges_file << edge_id << "," << source << "," << target << ","
                    << length << ","
                    << car_direct << "," << car_rev << "," 
                    << bike_direct << "," << bike_rev << ","
                    << foot << ","
                    << "LINESTRING(\"" << geom << "\")" << endl;
}


CSVWriter::~CSVWriter()
{
    edges_file.close();
}
