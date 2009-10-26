#include "reader.h"
#include <bzlib.h>

#ifndef _BZ2READER_H
#define _BZ2READER_H
class BZReader :  public Reader
{
    BZFILE* b;
    int bzerror;
    public:
    BZReader(const char * filename);
    int read(char * buff, int buffsize);
    bool eof() const;
};
#endif
