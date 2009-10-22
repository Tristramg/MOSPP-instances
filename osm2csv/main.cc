/*
    This file is part of Mumoro.

    Mumoro is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Mumoro is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Mumoro.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "main.h"
#include "cmath"
#include <cerrno>
#include <hash_set>
#include <deque>

using namespace std;

Node * source;
Node * prev;
std::stringstream geom;
NodeMapType nodes;
uint64_t ways_count;
int edge_length;
uint64_t ways_progress;
Edge_property ep;
std::list<Edge> edges;
double length;
ofstream edges_file;

ofstream temp_edges;

std::deque<uint64_t> way_nodes;
uint64_t current_way;

double rad(double deg)
{
    return deg * M_PIl / 180;
}

double distance(double lon1, double lat1, double lon2, double lat2)
{
    const double r = 6371000;

    return acos( sin( rad(lat1) ) * sin( rad(lat2) ) +
            cos( rad(lat1) ) * cos( rad(lat2) ) * cos( rad(lon2-lon1 ) )
            ) * r;
}

Edge::Edge(Node * s, Node * t, const std::string & g, double l) :
    source(s), target(t), geom(g), length(l)
{
}
    void
start(void *, const char *el, const char **attr)
{
    if (strcmp(el, "node") == 0)
    {
        uint64_t id = 0;
        double lat = 0, lon = 0;
        while (*attr != NULL)
        {
            const char* name = *attr++;
            const char* value = *attr++;

            if (strcmp(name, "id") == 0)
            {
                id = atoll(value);
            }
            else if (strcmp(name, "lat") == 0)
            {
                lat = atof(value);
            }
            else if (strcmp(name, "lon") == 0)
            {
                lon = atof(value);
            }
        }
        nodes[id] = Node(lon, lat, id);
    }

    else if (strcmp(el, "nd") == 0)
    {
        const char* name = *attr++;
        const char* value = *attr++;
        if (strcmp(name, "ref") == 0)
        {
            uint64_t node_id = atoll(value);
            way_nodes.push_back(node_id);
            //            nodes[node_id].uses++;
        }
    }

    else if(strcmp(el, "way") == 0)
    {
        way_nodes.clear();
        ep.reset();
        const char* name = *attr++;
        const char* value = *attr++;
        if( !strcmp(name, "id") == 0 )
        {
            cout << "fuck" << std::endl;
        }
        else
        {
            current_way = atoll(value);
        }
        ways_count++;
    }

    else if(strcmp(el, "tag") == 0)
    {
        string key;
        while (*attr != NULL)
        {
            const char* name = *attr++;
            const char* value = *attr++;

            if ( strcmp(name, "k") == 0 )
                key = value;
            else if ( strcmp(name, "v") == 0 )
            {
                ep.update(key, value);
            }
        }
    }

}

    void
end(void * , const char * el)
{
    if(strcmp(el, "way") == 0)
    {
        if(ep.accessible())
        {
            ep.normalize();
            deque<uint64_t>::const_iterator it;
            temp_edges << ep.foot << " "
                << ep.car_direct << " " << ep.car_reverse << " "
                << ep.bike_direct << " " << ep.bike_reverse << " "
                << way_nodes.size();
            for(it = way_nodes.begin(); it < way_nodes.end(); it++)
            {
                nodes[*it].uses++;
                temp_edges << " " << *it;
            }
            temp_edges << endl;

            nodes[way_nodes.front()].uses++;
            nodes[way_nodes.back()].uses++;
        }
    } 
}

    int
main(int argc, char** argv)
{
    temp_edges.open("temp_ways");
    if (argc != 2)
    {
        cout << "Usage: " << argv[0] << " in_database.osm" << endl;
        return (EXIT_FAILURE);
    }


    //==================== STEP 1 =======================//
    cout << "Step 1: reading the xml file, extracting the Nodes list" << flush;

    FILE* fp = fopen64(argv[1], "rb");
    if(!fp)
    {
        std::cout << std::endl;
        std::cerr << "Error opening file " << argv[1] << " errorno " << errno << " " << strerror(errno) << std::endl;
        exit(1);
    }
    XML_Parser parser = XML_ParserCreate(NULL);
    XML_SetElementHandler(parser, start, end);
    int done;
    do // loop over whole file content
    {
        char buf[BUFSIZ];
        size_t len = fread(buf, 1, sizeof (buf), fp); // read chunk of data
        done = len < sizeof (buf); // end of file reached if buffer not completely filled
        if (!XML_Parse(parser, buf, (int) len, done))
        {
            // a parse error occured:
            cerr << XML_ErrorString(XML_GetErrorCode(parser)) <<
                " at line " <<
                XML_GetCurrentLineNumber(parser) << endl;
            fclose(fp);
            done = 1;
        }
    }
    while (!done);
    cout << "... DONE!" << endl;
    cout << "    Nodes found: " << nodes.size() << endl;
    cout << "    Ways found: " << ways_count << endl << endl;


    //===================== STEP 2 ==========================//
    cout << "Step 2: building edges and saving them in the file edges.csv" << endl;

    temp_edges.close();
    edges_file.open("edges.csv");
    ifstream tmp;
    tmp.open("temp_ways");
    uint64_t id;
    stringstream geom;
    float length = 0, pred_lon = 0, pred_lat = 0;
    char car_direct, car_rev, foot, bike_direct, bike_rev;
    int nb;
    int edges_inserted = 0;
    Node n;
    string line;

    while(getline(tmp, line))
    {
        stringstream way(line);
        way >> foot >> car_direct >> car_rev >> bike_direct >> bike_rev >> nb;
        for(int i=0; i<nb; i++)
        {
            way >> id;
            n = nodes[id];

            if(i == 0)
            {
                edges_file << edges_inserted << "," << id << ",";
            }
            else
            {
                length += distance(n.lon, n.lat, pred_lon, pred_lat);
                geom << ",";
            }

            pred_lon = n.lon;
            pred_lat = n.lat;

            geom << n.lon << " " << n.lat;
            if( i>0 && n.uses > 1)
            {
                edges_file << n.id << ","
                    << length << ","
                    << car_direct << "," << car_rev << "," 
                    << bike_direct << "," << bike_rev << ","
                    << foot << ","
                    << "LINESTRING(\"" << geom.str() << "\")" << endl;
                edges_inserted++;
                length = 0;
                geom.str("");
                edges_file << edges_inserted << "," << id << ",";
                geom << n.lon << " " << n.lat;
            }
        }
    }

    tmp.close();
    edges_file.close();

    //==================== STEP 3 =======================//
    cout << "Step 3: storing the intersection nodes in the file nodes.csv" << endl;
    uint64_t count = 0, step = nodes.size() / 50, next_step = 0;

    ofstream nodes_file;
    nodes_file.open ("nodes.csv");
    // By default outstream only give 4 digits after the dot (~10m precision)
    nodes_file << setprecision(9);
    nodes_file << "\"node_id\",\"longitude\",\"latitude\",\"altitude\"" << endl;
    int nodes_inserted = 0;

    for(NodeMapType::const_iterator i = nodes.begin(); i != nodes.end(); i++)
    {
        count++;
        if(count >= next_step)
        {
            int advance = (count * 50 / (nodes.size()));
            cout << "\r[" << setfill('=') << setw(advance) << ">" <<setfill(' ') << setw(50-advance) << "] " << flush;
            next_step += step;
        }

        if( (*i).second.uses > 1 )
        {
            nodes_file << (*i).first << "," <<
                (*i).second.lon << "," << 
                (*i).second.lat << "," << endl;
            nodes_inserted++;
        }
    }
    nodes_file.close();
    cout << "DONE!" << endl << endl;

    cout << "Nodes in database: " << nodes_inserted << endl;
    cout << "Edges in database: " << edges_inserted << endl;
    cout << "Happy routing! :)" << endl << endl;

    return (EXIT_SUCCESS);
}

