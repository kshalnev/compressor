#include "lzwcompressor.h"
#include "lzw.h"
#include <cassert>

inline void check_true(bool expr)
{
    if (!expr) throw std::exception();
}

void Lzw::Compress(IReadStream& source, ISequentialWriteStream& dest)
{
    check_true( source.Seek(0) );

    ISequentialReadStream& sequentialSource = source;
    
    Compress(sequentialSource, dest);
}

void Lzw::Compress(ISequentialReadStream& source, ISequentialWriteStream& dest)
{
    auto l = [&](unsigned int c)
    {
        check_true( dest.Write(&c, sizeof(c)) == sizeof(c) );
    };
    
    LzwCompressor compressor;
    
    compressor.Begin(l);
    
    for (unsigned char b = 0; sizeof(b) == source.Read(&b, sizeof(b));)
    {
        compressor.Put(b);
    }
    
    compressor.End();
}

void Lzw::Decompress(ISequentialReadStream& source, ISequentialWriteStream& dest)
{
    LzwDecompressor decompressor;
    
    auto l = [&](const std::vector<unsigned char>& data)
    {
        unsigned int n = static_cast<unsigned int>(data.size());
        check_true( n == dest.Write(&data[0], n) );
    };
    
    decompressor.Begin(l);
    
    for (unsigned int c = 0; sizeof(c) == source.Read(&c, sizeof(c));)
    {
        check_true( decompressor.Put(c) );
    }
    
    decompressor.End();
}
