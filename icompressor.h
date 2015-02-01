#pragma once

#include "istream.h"

class ICompressor
{
public:
    virtual void Compress(IReadStream& source, ISequentialWriteStream& dest) = 0;
    virtual void Decompress(IReadStream& source, ISequentialWriteStream& dest) = 0;
    
    virtual ~ICompressor() {}
};
