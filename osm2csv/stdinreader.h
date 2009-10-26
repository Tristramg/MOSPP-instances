#include "reader.h"

#ifndef _STDINREADER_H
#define _STDINREADER_H
class StdinReader : public Reader
{
    public:
    int read(char * buff, int buffsize);
    bool eof() const;
};
#endif
