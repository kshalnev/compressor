#pragma once

#include "icompressor.h"

class Lzw : public ICompressor
{
public:
    virtual void Compress(IReadStream& source, ISequentialWriteStream& dest);
    virtual void Decompress(ISequentialReadStream& source, ISequentialWriteStream& dest);
    
    void Compress(ISequentialReadStream& source, ISequentialWriteStream& dest);
};

class BitLzw : public ICompressor
{
public:
    virtual void Compress(IReadStream& source, ISequentialWriteStream& dest);
    virtual void Decompress(ISequentialReadStream& source, ISequentialWriteStream& dest);
};
