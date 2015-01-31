#include "huffmancompressor.h"
#include "huffman.h"
#include "streamimpl.h"
#include "bitstream.h"
#include <cassert>

inline void check_true(bool expr)
{
    if (!expr) throw std::exception();
}

static void CompressHuffmanCodesTable(BitStreamWriter& w, const HuffmanCodeTable& codes)
{
    unsigned int cnt = 0;
    
    // scan codes to get min and max
    unsigned char minValue = (unsigned char)-1, maxValue = 0;
    unsigned int minCode = (unsigned int)-1, maxCode = 0;
    unsigned int minLen = (unsigned int)-1, maxLen = 0;
    for (unsigned int i = 0; i < ByteTypeCountValues; ++i)
    {
        const unsigned char b = static_cast<unsigned char>(i);
        const CodeLength& codeLen = codes.GetCodeLength(b);
        if (codeLen.length != 0)
        {
            ++cnt;
            
            if (b < minValue) minValue = b;
            else if (maxValue < b) maxValue = b;
            
            if (codeLen.code < minCode) minCode = codeLen.code;
            else if (maxCode < codeLen.code) maxCode = codeLen.code;
            
            if (codeLen.length < minLen) minLen = codeLen.length;
            else if (maxLen < codeLen.length) maxLen = codeLen.length;
        }
    }
    
    // bit lengths are
    unsigned char valueBits = CountBits(maxValue - minValue);
    unsigned char codeBits = CountBits(maxCode - minCode);
    unsigned char lenBits = CountBits(maxLen - minLen);
    
    // write "header" of codes table
    check_true( w.WriteBits(&cnt, BitsPerByte * sizeof(cnt)) );
    check_true( w.WriteBits(&valueBits, BitsPerByte * sizeof(valueBits)) );
    check_true( w.WriteBits(&codeBits, BitsPerByte * sizeof(codeBits)) );
    check_true( w.WriteBits(&lenBits, BitsPerByte * sizeof(lenBits)) );
    
    // write min values
    check_true( w.WriteBits(&minValue, BitsPerByte * sizeof(minValue)) );
    check_true( w.WriteBits(&minCode, BitsPerByte * sizeof(minCode)) );
    check_true( w.WriteBits(&minLen, BitsPerByte * sizeof(minLen)) );
    
    // write codes table
    for (unsigned int i = 0; i < ByteTypeCountValues; ++i)
    {
        const unsigned char b = static_cast<unsigned char>(i);
        const CodeLength& codeLen = codes.GetCodeLength(b);
        if (codeLen.length != 0)
        {
            unsigned char wb = b - minValue;
            unsigned int wc = codeLen.code - minCode;
            unsigned int wl = codeLen.length - minLen;
            check_true( w.WriteBits(&wb, valueBits) );
            check_true( w.WriteBits(&wc, codeBits) );
            check_true( w.WriteBits(&wl, lenBits) );
        }
    }
}

static void DecompressHuffmanCodesTable(BitStreamReader& r, HuffmanCodeTable& codes)
{
    unsigned int cnt = 0;
    
    unsigned char valueBits = 0;
    unsigned char codeBits = 0;
    unsigned char lenBits = 0;
    
    unsigned char minValue = 0;
    unsigned int minCode = 0;
    unsigned int minLen = 0;
    
    // read "header" of codes table
    check_true( r.ReadBits(&cnt, BitsPerByte * sizeof(cnt)) );
    check_true( r.ReadBits(&valueBits, BitsPerByte * sizeof(valueBits)) );
    check_true( r.ReadBits(&codeBits, BitsPerByte * sizeof(codeBits)) );
    check_true( r.ReadBits(&lenBits, BitsPerByte * sizeof(lenBits)) );
    
    // write min values
    check_true( r.ReadBits(&minValue, BitsPerByte * sizeof(minValue)) );
    check_true( r.ReadBits(&minCode, BitsPerByte * sizeof(minCode)) );
    check_true( r.ReadBits(&minLen, BitsPerByte * sizeof(minLen)) );
    
    // read codes table
    for (unsigned int i = 0; i < cnt; ++i)
    {
        unsigned char b = 0;
        unsigned int c = 0, l = 0;
        check_true( r.ReadBits(&b, valueBits) );
        check_true( r.ReadBits(&c, codeBits) );
        check_true( r.ReadBits(&l, lenBits) );
        codes.SetCodeLength(b + minValue, CodeLength(c + minCode, l + minLen));
    }
}

static void Compress(IReadStream& source, ISequentialWriteStream& dest)
{
    HuffmanCodeTable codes;
    unsigned int cntBits = 0;

    check_true( source.Seek(0) );
    
    {
        HuffmanScanner scanner;
        scanner.BeginScan();
        for (unsigned char b = 0; source.Read(&b, sizeof(b));) scanner.Scan(b);
        scanner.EndScan(codes, cntBits);
    }
    
    check_true( source.Seek(0) );
    
    {
        BitStreamWriter w(&dest);
        
        CompressHuffmanCodesTable(w, codes);
        
        check_true( w.WriteBits(&cntBits, BitsPerByte * sizeof(cntBits)) );
        
        for (unsigned char b = 0; source.Read(&b, sizeof(b));)
        {
            const CodeLength& cl = codes.GetCodeLength(b);
            check_true( w.WriteBits(&cl.code, cl.length) );
        }
        
        check_true( w.CompleteByte() );
    }
}

static void Decompress(ISequentialReadStream& source, ISequentialWriteStream& dest)
{
    BitStreamReader r(&source);
    
    HuffmanCodeTable codes;
    DecompressHuffmanCodesTable(r, codes);
    
    unsigned int cntBits = 0;
    check_true( r.ReadBits(&cntBits, BitsPerByte * sizeof(cntBits)) );
    
    HuffmanReader reader(codes);
    HuffmanReader::Result res = HuffmanReader::Success;
    for (unsigned int i = 0; i < cntBits; ++i)
    {
        unsigned int bit = 0;
        check_true( r.ReadBits(&bit, 1) );
        unsigned char value = 0;
        if (HuffmanReader::Success == (res = reader.ReadBit(bit, &value)))
        {
            check_true( dest.Write(&value, sizeof(value)) );
        }
    }
    
    check_true( HuffmanReader::Success == res );
}

bool Huffman::Compress(const char* source, const char* dest)
{
    std::unique_ptr<FILE, decltype(&fclose)> fileIn(fopen(source, "rb"), &fclose);
    
    if (!fileIn)
        return false;
    
    std::unique_ptr<FILE, decltype(&fclose)> fileOut(fopen(dest, "wb"), &fclose);
    
    if (!fileOut)
        return false;
    
    try
    {
        FileReadStream rs(fileIn.get());
        FileSequentialWriteStream ws(fileOut.get());
        ::Compress(rs, ws);
    }
    catch (std::exception&)
    {
        return false;
    }
    
    return true;
}

bool Huffman::Decompress(const char* source, const char* dest)
{
    std::unique_ptr<FILE, decltype(&fclose)> fileIn(fopen(source, "rb"), &fclose);
    
    if (!fileIn)
        return false;
    
    std::unique_ptr<FILE, decltype(&fclose)> fileOut(fopen(dest, "wb"), &fclose);
    
    if (!fileOut)
        return false;
    
    try
    {
        FileReadStream rs(fileIn.get());
        FileSequentialWriteStream ws(fileOut.get());
        ::Decompress(rs, ws);
    }
    catch (std::exception&)
    {
        return false;
    }
    
    return true;
}
