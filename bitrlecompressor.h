#pragma once

#include "icompressor.h"

class BitRle : public ICompressor
{
public:
    virtual void Compress(IReadStream& source, ISequentialWriteStream& dest);
    virtual void Decompress(ISequentialReadStream& source, ISequentialWriteStream& dest);
};
