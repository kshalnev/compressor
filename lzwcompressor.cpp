#include "lzwcompressor.h"
#include "lzw.h"
#include "common.h"
#include "bitstream.h"
#include <cassert>

//
//
//

inline void check_true(bool expr)
{
    if (!expr) throw std::exception();
}

template <typename T, typename F>
void for_each(ISequentialReadStream& source, F f)
{
    for (T v = 0; sizeof(v) == source.Read(&v, sizeof(v));)
    {
        f(v);
    }
}

template <typename T, typename F>
void for_each(IReadStream& source, F f)
{
    check_true( source.Seek(0) );
    ISequentialReadStream& sequentialSource = source;
    for_each<T>(sequentialSource, f);
}

//
//
//

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
    for_each<unsigned char>(source, [&](unsigned char b){ compressor.Put(b); });
    compressor.End();
}

void Lzw::Decompress(ISequentialReadStream& source, ISequentialWriteStream& dest)
{
    auto l = [&](const std::vector<unsigned char>& data)
    {
        unsigned int n = static_cast<unsigned int>(data.size());
        check_true( n == dest.Write(&data[0], n) );
    };
    
    LzwDecompressor decompressor;
    decompressor.Begin(l);
    for_each<unsigned int>(source, [&](unsigned int v){ check_true( decompressor.Put(v) ); });
    decompressor.End();
}

//
//
//

void BitLzw::Compress(IReadStream& source, ISequentialWriteStream& dest)
{
    unsigned int min = 0, max = 0, count = 0;
    
    auto lscan = [&](unsigned int c)
    {
        if (0 == count) { min = max = c; }
        else if (min > c) min = c;
        else if (max < c) max = c;
        ++count;
    };
    
    LzwCompressor compressor;
    
    compressor.Begin(lscan);
    for_each<unsigned char>(source, [&](unsigned char b){ compressor.Put(b); });
    compressor.End();
    
    unsigned char len = CountBits(max - min);
    
    BitStreamWriter w(&dest);
    check_true( w.WriteBits(count) );
    check_true( w.WriteBits(min) );
    check_true( w.WriteBits(len) );
    
    auto lwrite = [&](unsigned int c)
    {
        c -= min;
        check_true( w.WriteBits(&c, len) );
    };
    
    compressor.Begin(lwrite);
    for_each<unsigned char>(source, [&](unsigned char b){ compressor.Put(b); });
    compressor.End();
    
    check_true( w.CompleteByte() );
}

void BitLzw::Decompress(ISequentialReadStream& source, ISequentialWriteStream& dest)
{
    BitStreamReader r(&source);

    unsigned char len = 0;
    unsigned int min = 0, count = 0;
    
    check_true( r.ReadBits(&count) );
    check_true( r.ReadBits(&min) );
    check_true( r.ReadBits(&len) );
    
    auto l = [&](const std::vector<unsigned char>& data)
    {
        unsigned int n = static_cast<unsigned int>(data.size());
        check_true( n == dest.Write(&data[0], n) );
    };
    
    LzwDecompressor decompressor;
    
    decompressor.Begin(l);
    
    for (unsigned int c = 0; c < count; ++c)
    {
        unsigned int code = 0;
        check_true( r.ReadBits(&code, len) );
        code += min;
        
        check_true( decompressor.Put(code) );
    }

    decompressor.End();
}
