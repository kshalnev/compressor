#include "bitrlecompressor.h"
#include "bitrle.h"
#include "bitstream.h"
#include "streamimpl.h"
#include <cassert>

inline void check_true(bool expr)
{
    if (!expr) throw std::exception();
}

static void CompressBitRleTable(BitStreamWriter& w, const BitRleTable& table)
{
    const unsigned char minValue = table.GetMinValue();
    const unsigned char maxValue = table.GetMaxValue();
    const unsigned char minRepeats = table.GetMinRepeats();
    const unsigned char maxRepeats = table.GetMaxRepeats();
    
    check_true( w.WriteBits(minValue) );
    check_true( w.WriteBits(maxValue) );
    check_true( w.WriteBits(minRepeats) );
    check_true( w.WriteBits(maxRepeats) );
}

static BitRleTable DecompressBitRleTable(BitStreamReader& r)
{
    unsigned char minValue = 0;
    unsigned char maxValue = 0;
    unsigned char minRepeats = 0;
    unsigned char maxRepeats = 0;
    
    check_true( r.ReadBits(&minValue) );
    check_true( r.ReadBits(&maxValue) );
    check_true( r.ReadBits(&minRepeats) );
    check_true( r.ReadBits(&maxRepeats) );
    
    check_true( minValue <= maxValue );
    check_true( minRepeats <= maxRepeats );
    
    return BitRleTable(minValue, maxValue, minRepeats, maxRepeats);
}

void BitRle::Compress(IReadStream& source, ISequentialWriteStream& dest)
{
    BitRleTable table;
    unsigned int cntBits = 0;
    
    check_true( source.Seek(0) );
    
    {
        BitRleScanner scanner;
        scanner.BeginScan();
        for (unsigned char b = 0; sizeof(b) == source.Read(&b, sizeof(b));)
        {
            scanner.Scan(b);
        }
        scanner.EndScan(table, cntBits);
    }
    
    check_true( source.Seek(0) );
    
    BitStreamWriter w(&dest);
    
    CompressBitRleTable(w, table);
    check_true( w.WriteBits(cntBits) );
    
    auto sink = [&](const CodeLength& value, const CodeLength& repeats)
    {
        check_true( w.WriteBits(&value.code, value.length) );
        check_true( w.WriteBits(&repeats.code, repeats.length) );
    };
    
    BitRleCompressor compressor(table);
    compressor.BeginCompress(sink);
    for (unsigned char b = 0; sizeof(b) == source.Read(&b, sizeof(b));)
    {
        compressor.Compress(b);
    }
    compressor.EndCompress();
    
    check_true( w.CompleteByte() );
}

void BitRle::Decompress(ISequentialReadStream& source, ISequentialWriteStream& dest)
{    
    BitStreamReader r(&source);
    
    const BitRleTable& table = DecompressBitRleTable(r);
    
    check_true( table.GetValueLength() <= BitsPerByte );
    check_true( table.GetRepeatsLength() <= BitsPerByte );
    
    unsigned int cntBits = 0;
    check_true( r.ReadBits(&cntBits) );
    
    std::vector<unsigned char> buff(256);
    
    unsigned int c = 0;
    while (c < cntBits)
    {
        unsigned char value = 0;
        unsigned char repeats = 0;
        check_true( r.ReadBits(&value, table.GetValueLength()) );
        check_true( r.ReadBits(&repeats, table.GetRepeatsLength()) );
        
        check_true( ((unsigned int)value + table.GetMinValue()) <= 255 );
        check_true( ((unsigned int)repeats + table.GetMinRepeats()) <= 255 );
        
        value += table.GetMinValue();
        repeats += table.GetMinRepeats();
        
        for (unsigned int i = 0; i < repeats; ++i) buff[i] = value;
        
        check_true( dest.Write(&buff[0], repeats) == repeats );
        
        c += table.GetValueLength() + table.GetRepeatsLength();
    }
    
    check_true( c == cntBits );
}
