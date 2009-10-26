#include "reader.h"
#include <fstream>

#ifndef _OSMREADER_H
#define _OSMREADER_H
class OsmReader : public Reader
{
    std::ifstream file;
    public:
    OsmReader(const char * filename);
    int read(char * buff, int buffsize);
    bool eof() const;
};
#endif
