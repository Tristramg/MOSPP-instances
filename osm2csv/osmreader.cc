#include "osmreader.h"
#include <iostream>

using namespace std;

OsmReader::OsmReader(const char * filename)
{
    file.open(filename, ifstream::in);
    if( !file.good() )
    {
        std::cerr << "Unable to open file " << file << std::endl;
        throw file_problem();
    }
}

int OsmReader::read(char * buff, int buffsize)
{
    file.read(buff, buffsize);
    return file.gcount();
}

bool OsmReader::eof() const
{
    return file.eof();
}
