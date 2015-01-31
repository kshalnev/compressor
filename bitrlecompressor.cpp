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
    
    return BitRleTable(minValue, maxValue, minRepeats, maxRepeats);
}

template <typename F>
inline void ForEachByteInFile(FILE* file, F f, size_t buffSize = 2048)
{
    std::vector<unsigned char> buff(buffSize);
    size_t res = 0;
    check_true( 0 == fseek(file, 0, SEEK_SET) );
    while ((res = fread(&buff[0], 1, buff.size(), file)) > 0)
    {
        std::for_each(buff.begin(), buff.begin() + res, f);
    }
    check_true( feof(file) );
}

static void Compress(FILE* source, FILE* dest)
{
    BitRleTable table;
    unsigned int cntBits = 0;
    
    {
        BitRleScanner scanner;
        scanner.BeginScan();
        ForEachByteInFile(source, [&](unsigned char b) { scanner.Scan(b); });
        scanner.EndScan(table, cntBits);
    }
    
    fseek(source, 0, SEEK_SET);
    
    FileSequentialWriteStream ws(dest);
    BitStreamWriter w(&ws);
    
    CompressBitRleTable(w, table);
    check_true( w.WriteBits(cntBits) );
    
    auto sink = [&](const CodeLength& value, const CodeLength& repeats)
    {
        check_true( w.WriteBits(&value.code, value.length) );
        check_true( w.WriteBits(&repeats.code, repeats.length) );
    };
    
    BitRleCompressor compressor(table);
    compressor.BeginCompress(sink);
    ForEachByteInFile(source, [&](unsigned char b) { compressor.Compress(b); });
    compressor.EndCompress();
    
    check_true( w.CompleteByte() );
}

static void Decompress(FILE* source, FILE* dest)
{
    FileSequentialWriteStream ws(dest);
    
    FileSequentialReadStream rs(source);
    BitStreamReader r(&rs);
    
    const BitRleTable& table = DecompressBitRleTable(r);
    
    unsigned int cntBits = 0;
    check_true( r.ReadBits(&cntBits) );
    
    std::vector<unsigned char> buff;
    
    unsigned int c = 0;
    while (c < cntBits)
    {
        unsigned char value = 0;
        unsigned char repeats = 0;
        check_true( r.ReadBits(&value, table.GetValueLength()) );
        check_true( r.ReadBits(&repeats, table.GetRepeatsLength()) );
        
        value += table.GetMinValue();
        repeats += table.GetMinRepeats();
        
        buff.resize(repeats);
        for (unsigned int i = 0; i < repeats; ++i) buff[i] = value;
        
        check_true( ws.Write(&buff[0], repeats) );
        
        c += table.GetValueLength() + table.GetRepeatsLength();
    }
    
    check_true( c == cntBits );
}

bool BitRle::Compress(const char* source, const char* dest)
{
    std::unique_ptr<FILE, decltype(&fclose)> fileIn(fopen(source, "rb"), &fclose);
    
    if (!fileIn)
        return false;
    
    std::unique_ptr<FILE, decltype(&fclose)> fileOut(fopen(dest, "wb"), &fclose);
    
    if (!fileOut)
        return false;
    
    try
    {
        ::Compress(fileIn.get(), fileOut.get());
    }
    catch (std::exception&)
    {
        return false;
    }
    
    return true;
}

bool BitRle::Decompress(const char* source, const char* dest)
{
    std::unique_ptr<FILE, decltype(&fclose)> fileIn(fopen(source, "rb"), &fclose);
    
    if (!fileIn)
        return false;
    
    std::unique_ptr<FILE, decltype(&fclose)> fileOut(fopen(dest, "wb"), &fclose);
    
    if (!fileOut)
        return false;
    
    try
    {
        ::Decompress(fileIn.get(), fileOut.get());
    }
    catch (std::exception&)
    {
        return false;
    }
    
    return true;
}
