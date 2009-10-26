#include "writer.h"
#include <iostream>

#ifndef _CSVWRITER_H
#define _CSVWRITER_H

class CSVWriter : public Writer
{
    std::ofstream edges_file;
    public:
    CSVWriter();

    int save_nodes(const NodeMapType & nodes);

    void save_edge(int edge_id,
            node_t source, node_t target, float length,
            char car, char car_d,
            char bike, char bike_d,
            char foot,
            std::string geom);

    ~CSVWriter();
};

#endif
