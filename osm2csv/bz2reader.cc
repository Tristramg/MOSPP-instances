#include "bz2reader.h"
#include <iostream>
#include <cerrno>
#include <string.h>
#include <stdlib.h>


using namespace std;
BZReader::BZReader(const char * filename)
{
    FILE* fp = fopen64(filename, "rb");
    if(!fp)
    {
        std::cout << std::endl;
        std::cerr << "Error opening file " << filename << " errorno " << errno << " " << strerror(errno) << std::endl;
        exit(1);
    }

    b = BZ2_bzReadOpen ( &bzerror, fp, 0, 0, NULL, 0 );
    if ( bzerror != BZ_OK )
    {
        std::cerr << "Error opening file " << filename << " as bzip2 file, errno " << bzerror << " " << 
            BZ2_bzerror(b, &bzerror) << std::endl;
        BZ2_bzReadClose ( &bzerror, b );
    }
}

int BZReader::read(char * buff, int buffsize)
{
    return BZ2_bzRead ( &bzerror, b, buff, buffsize );
}

bool BZReader::eof() const
{
    return bzerror == BZ_STREAM_END;
}
