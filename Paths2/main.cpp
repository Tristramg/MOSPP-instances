#include "mainwindow.h"
#include "martins.h"
#include "MultimodalGraph.h"

#include "boost/date_time/posix_time/posix_time_types.hpp"
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include "cmath"
using namespace std;
using namespace boost::posix_time;

int main(int argc, char *argv[])
{
    if(argc == 1)
    {
        QApplication app(argc, argv);

        MainWindow ta;
        ta.setWindowTitle("QMapControl Demo");
        ta.show();
        return app.exec();
    }
    else
    {
        MultimodalGraph g;
        std::string path = "/home/tristram/MOSPP-instances/instances/San Francisco";
        std::pair<int, int> a, b, c;
        a = g.load("foot", path + "/nodes.csv", path+"/edges.csv", Foot);
        cout << "Loaded " << a.first << " nodes, and " << a.second << " edges" << endl;
        b = g.load("bart", path + "/stops.txt", path+"/stop_times.txt", PublicTransport);
        cout << "Loaded " << b.first << " nodes, and " << b.second << " edges" << endl;
        c = g.load("muni", path + "/stops_muni.txt", path+"/stop_times_muni.txt", PublicTransport);
        cout << "Loaded " << c.first << " nodes, and " << c.second << " edges" << endl;

        Edge interconnexion;
        interconnexion.distance = 0;
        interconnexion.duration = Duration(30);
        interconnexion.elevation = 0;
        interconnexion.nb_changes = 1;
        cout << "Nb of interconnecting edges: " << g.connect_closest("bart", "foot", interconnexion) << endl;
        cout << "Nb of interconnecting edges: " << g.connect_closest("muni", "foot", interconnexion) << endl;

        srand ( time(NULL) );
        for(int i=0; i<10; i++)
        {
            ptime stime(microsec_clock::local_time());
            node_t start = rand() % a.first;
            node_t dest = rand() % b.first;
            size_t n=(martins(start, dest, g)).size();
            ptime etime(microsec_clock::local_time());

            cout << "Duration: " << (etime - stime).total_milliseconds() << "ms; distance: " << distance(g[start].lon, g[start].lat, g[dest].lon, g[dest].lat) << "; Nb solutions: " << n << endl;
        }
    }

}
