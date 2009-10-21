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
#include <bzlib.h>

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
int edges_inserted;
ofstream edges_file;

__gnu_cxx::hash_set<uint64_t, std::tr1::hash<uint64_t> > ways_black_list;
std::deque<uint64_t> way_nodes;
uint64_t current_way;
bool keep;

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
        keep = false;
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

    else if(!keep && strcmp(el, "tag") == 0)
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
                keep = ep.update(key, value);
            }
        }
    }

}

    void
start2(void *, const char *el, const char **attr)
{
    if(strcmp(el, "way") == 0)
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
            if(ways_black_list.find(atoll(value)) == ways_black_list.end())
            {
                keep = true;
            }
            else
                keep = false;

        }
    }

    if (keep && strcmp(el, "nd") == 0)
    {
        const char* name = *attr++;
        const char* value = *attr++;
        if (strcmp(name, "ref") == 0)
        {
            uint64_t id = atoll(value);
            Node * n = &(nodes[id]);

            if(edge_length == 0)
            {
                source = n;
            }
            else
            {
                geom << ", ";
                length += distance(prev->lon, prev->lat, n->lon, n->lat);
            }

            geom << n->lon << " " << n->lat;
            prev = n;
            edge_length++;

            if(n->uses > 1 && edge_length > 1 )
            {
                // It's a duplicate node, or a loop way
                if(source != prev)
                    edges.push_back(Edge(source, prev, geom.str(), length));

                source = n;
                length = 0;

                geom.str("");
                geom << n->lon << " " << n->lat;
                edge_length = 1;
            }
        }
    }

    else if(keep && strcmp(el, "tag") == 0)
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
        if(keep)
        {
            deque<uint64_t>::const_iterator it;
            for(it = way_nodes.begin(); it < way_nodes.end(); it++)
            {
                nodes[*it].uses++;
            }
        }
        else
        {
            ways_black_list.insert(current_way);
        }
      } 

}

    void
end2(void *, const char *el)
{
    if(keep && strcmp(el, "way") == 0)
    {
        int advance = (ways_progress++ * 50 / (ways_count));
        cout << "\r[" << setfill('=') << setw(advance) << ">" <<setfill(' ') << setw(50-advance) << "] " << flush;
        if(edge_length >= 2)       
        {
            edges.push_back(Edge(source, prev, geom.str(), length));
        }
 
        if(ep.accessible())
        {
            ep.normalize();
            list<Edge>::iterator it;
            for(it = edges.begin(); it != edges.end(); it++)
            {
                (*it).source->inserted = true;
                (*it).target->inserted = true;
                edges_file << edges_inserted << "," <<  // id
                    (*it).source->id << "," << // source
                    (*it).target->id << "," << // target
                    (*it).length << "," << // length
                    ep.car_direct << "," <<
                    ep.car_reverse << "," <<
                    ep.bike_direct << "," <<
                    ep.bike_reverse << "," <<
                    ep.foot << "," <<
                    "LINESTRING(\"" << (*it).geom << "\")" << endl;
                edges_inserted++;
            }
        }
        edges.clear();
        length = 0;
        geom.str("");
        edge_length = 0;
        ep.reset();

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

BZFILE* b;
int     nBuf;
int     bzerror;


    //==================== STEP 1 =======================//
    cout << "Step 1: reading the xml file, extracting the Nodes list" << flush;

    FILE* fp = fopen64(argv[1], "rb");
    if(!fp)
    {
        std::cout << std::endl;
        std::cerr << "Error opening file " << argv[1] << " errorno " << errno << " " << strerror(errno) << std::endl;
        exit(1);
    }

    b = BZ2_bzReadOpen ( &bzerror, fp, 0, 0, NULL, 0 );
    if ( bzerror != BZ_OK )
    {
        std::cerr << "Error opening file " << argv[1] << " as bzip2 file, errno " << bzerror << " " << 
            BZ2_bzerror(b, &bzerror) << std::endl;
        BZ2_bzReadClose ( &bzerror, b );
        exit(1);
    }


    XML_Parser parser = XML_ParserCreate(NULL);
    XML_SetElementHandler(parser, start, end);

    while ( bzerror == BZ_OK ) 
    {
        char buf[BUFSIZ];
        //        size_t len = fread(buf, 1, sizeof (buf), fp); // read chunk of data
        nBuf = BZ2_bzRead ( &bzerror, b, buf, sizeof(buf) );
        if ( bzerror == BZ_OK ) {

            if (!XML_Parse(parser, buf, (int) nBuf, bzerror == BZ_STREAM_END))
            {
                // a parse error occured:
                cerr << XML_ErrorString(XML_GetErrorCode(parser)) <<
                    " at line " <<
                    XML_GetCurrentLineNumber(parser) << endl;
                cout << buf << std::endl;
                fclose(fp);
            }
        }
    }
    if ( bzerror != BZ_STREAM_END )
    {
        perror("foo");
        std::cerr << std::endl << "An error occured while parsing " << bzerror << " " <<  BZ2_bzerror(b, &bzerror) << " " << errno << " " << strerror(errno)   << std::endl;
        BZ2_bzReadClose ( &bzerror, b );
        exit(1);
    }

    cout << "... DONE!" << endl;
    cout << "    Nodes found: " << nodes.size() << endl;
    cout << "    Ways found: " << ways_count << endl << endl;


    //===================== STEP 2 ==========================//
    cout << "Step 2: building edges and saving them in the file edges.csv" << endl;
    keep = false;
    XML_Parser parser2 = XML_ParserCreate(NULL);
    XML_SetElementHandler(parser2, start2, end2);

    edges_file.open ("edges.csv");
    // By default outstream only give 4 digits after the dot (~10m precision)
    edges_file << setprecision(9);
    edges_file << "\"edge_id\",\"source\",\"target\",\"length\",\"car\",\"car reverse\",\"bike\",\"bike reverse\",\"foot\",\"WKT\"" << endl;

    fp = fopen64(argv[1], "rb");
    if(!fp)
    {
        std::cout << std::endl;
        std::cerr << "Error opening file " << argv[1] << " errorno " << errno << " " << strerror(errno) << std::endl;
        exit(1);
    }

    b = BZ2_bzReadOpen ( &bzerror, fp, 0, 0, NULL, 0 );
    if ( bzerror != BZ_OK )
    {
        BZ2_bzReadClose ( &bzerror, b );
        std::cerr << "Error opening file " << argv[1] << " as bzip2 file, errno " << bzerror << std::endl;
        exit(1);
    }

    while ( bzerror == BZ_OK ) 
    {
        char buf[BUFSIZ];
        //        size_t len = fread(buf, 1, sizeof (buf), fp); // read chunk of data
        nBuf = BZ2_bzRead ( &bzerror, b, buf, sizeof(buf) );
        if ( bzerror == BZ_OK ) {

            if (!XML_Parse(parser2, buf, (int) nBuf, bzerror == BZ_STREAM_END))
            {
                // a parse error occured:
                cerr << XML_ErrorString(XML_GetErrorCode(parser)) <<
                    " at line " <<
                    XML_GetCurrentLineNumber(parser) << endl;
                cout << buf << std::endl;
                fclose(fp);
            }
        }
    }
    if ( bzerror != BZ_STREAM_END )
    {
        perror("foo");
        std::cerr << std::endl << "An error occured while parsing " << bzerror << " " <<  BZ2_bzerror(b, &bzerror) << " " << errno << " " << strerror(errno)   << std::endl;
        BZ2_bzReadClose ( &bzerror, b );
        exit(1);
    } else {
        BZ2_bzReadClose ( &bzerror, b );
    }


    cout << "DONE!" << endl << endl;

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

        if( (*i).second.inserted )
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

