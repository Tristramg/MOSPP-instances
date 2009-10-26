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
#include <hash_set>
#include <deque>
#include "stdinreader.h"
#include "csvwriter.h"

using namespace std;

Node * source;
Node * prev;
std::stringstream geom;
NodeMapType nodes;
node_t ways_count;
int edge_length;
node_t ways_progress;
Edge_property ep;
double length;

ofstream temp_edges;

std::deque<node_t> way_nodes;
node_t current_way;

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
   void
start(void *, const char *el, const char **attr)
{
    if (strcmp(el, "node") == 0)
    {
        node_t id = 0;
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
            node_t node_id = atoll(value);
            way_nodes.push_back(node_id);
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

void end(void * , const char * el)
{
    if(strcmp(el, "way") == 0)
    {
        if(ep.accessible())
        {
            ep.normalize();
            deque<node_t>::const_iterator it;
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
    if (argc != 2)
    {
        cout << "Usage: " << argv[0] << " in_database.osm" << endl;
        return (EXIT_FAILURE);
    }


    //==================== STEP 1 =======================//
    cout << "Step 1: reading the xml file, extracting the Nodes list" << flush;
    temp_edges.open("temp_ways");
    XML_Parser parser = XML_ParserCreate(NULL);
    XML_SetElementHandler(parser, start, end);
    ios_base::sync_with_stdio(false);
    Reader * reader = new StdinReader();
//    OsmReader reader(argv[1]);
    while ( !reader->eof() ) 
    {
        char buf[BUFSIZ];
        //        size_t len = fread(buf, 1, sizeof (buf), fp); // read chunk of data
        int read = reader->read( buf, sizeof(buf) );
        if( !XML_Parse(parser, buf, read, reader->eof()) )
        {
            cerr << XML_ErrorString(XML_GetErrorCode(parser)) <<
                " at line " <<
                XML_GetCurrentLineNumber(parser) << endl;
        }
    }
    XML_ParserFree(parser);
    temp_edges.close();
    delete reader;
    cout << "... DONE!" << endl;
    cout << "    Nodes found: " << nodes.size() << endl;
    cout << "    Ways found: " << ways_count << endl << endl;


    //===================== STEP 2 ==========================//
    Writer * writer = new CSVWriter();
    cout << "Step 2: building edges and saving them in the file edges.csv" << endl;
    ifstream tmp;
    tmp.open("temp_ways");
    node_t id, source=0;
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
                source = id;
            }
            else
            {
                length += distance(n.lon, n.lat, pred_lon, pred_lat);
                geom << ",";
            }

            pred_lon = n.lon;
            pred_lat = n.lat;

            geom << n.lon << " " << n.lat;
            if( i>0 && n.uses > 1 && id != source)
            {
                writer->save_edge(edges_inserted, source, id, length, car_direct, car_rev, bike_direct, bike_rev, foot, geom.str());
                edges_inserted++;
                length = 0;
                geom.str("");
                source = id;
            }
        }
    }

    tmp.close();

    //==================== STEP 3 =======================//
    cout << "Step 3: storing the intersection nodes in the file nodes.csv" << endl;
    int nodes_inserted = writer->save_nodes(nodes);
    delete writer;

    cout << "DONE!" << endl << endl;

    cout << "Nodes in database: " << nodes_inserted << endl;
    cout << "Edges in database: " << edges_inserted << endl;
    cout << "Happy routing! :)" << endl << endl;

    return (EXIT_SUCCESS);
}

