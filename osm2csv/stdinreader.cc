#include "stdinreader.h"
#include <iostream>

using namespace std;

int StdinReader::read(char * buff, int buffsize)
{
    cin.read(buff, buffsize);
    return cin.gcount();
}

bool StdinReader::eof() const
{
    return cin.eof();
}
