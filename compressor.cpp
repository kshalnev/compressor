#include "compressor.h"
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
        if (codeLen.second != 0)
        {
            ++cnt;
            
            if (b < minValue) minValue = b;
            else if (maxValue < b) maxValue = b;
            
            if (codeLen.first < minCode) minCode = codeLen.first;
            else if (maxCode < codeLen.first) maxCode = codeLen.first;
            
            if (codeLen.second < minLen) minLen = codeLen.second;
            else if (maxLen < codeLen.second) maxLen = codeLen.second;
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
        if (codeLen.second != 0)
        {
            unsigned char wb = b - minValue;
            unsigned int wc = codeLen.first - minCode;
            unsigned int wl = codeLen.second - minLen;
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

template <typename F>
inline void ForEachByteInFile(FILE* file, F f, size_t buffSize = 2048)
{
    std::vector<unsigned char> buff(buffSize);
    size_t res = 0;
    fseek(file, 0, SEEK_SET);
    while ((res = fread(&buff[0], 1, buff.size(), file)) > 0)
    {
        std::for_each(buff.begin(), buff.begin() + res, f);
    }
}

Compressor::Result Compressor::Compress(const char* source, const char* dest)
{
    std::unique_ptr<FILE, decltype(&fclose)> fileIn(fopen(source, "rb"), &fclose);
    
    if (!fileIn)
        return Compressor::InvalidInputFile;
    
    std::unique_ptr<FILE, decltype(&fclose)> fileOut(fopen(dest, "wb"), &fclose);
    
    if (!fileOut)
        return Compressor::InvalidOutputFile;
    
    try
    {
        HuffmanCodeTable codes;
        unsigned int cntBits = 0;
        
        {
            HuffmanScanner scanner;
            
            scanner.BeginScan();
            
            ForEachByteInFile(fileIn.get(), [&](unsigned char b){ scanner.Scan(b); });
            
            scanner.EndScan(codes, cntBits);
        }
        
        {
            FileSequentialWriteStream ws(fileOut.get());
            BitStreamWriter w(&ws);
            
            CompressHuffmanCodesTable(w, codes);
            
            check_true( w.WriteBits(&cntBits, BitsPerByte * sizeof(cntBits)) );
            
            ForEachByteInFile(fileIn.get(), [&](unsigned char b)
                              {
                                  const CodeLength& cl = codes.GetCodeLength(b);
                                  check_true( w.WriteBits(&cl.first, cl.second) );
                              });
            
            check_true( w.CompleteByte() );
        }
    }
    catch (std::exception&)
    {
        return Compressor::InternalError;
    }
    
    return Compressor::Success;
}

Compressor::Result Compressor::Decompress(const char* source, const char* dest)
{
    std::unique_ptr<FILE, decltype(&fclose)> fileIn(fopen(source, "rb"), &fclose);
    
    if (!fileIn)
        return Compressor::InvalidInputFile;
    
    std::unique_ptr<FILE, decltype(&fclose)> fileOut(fopen(dest, "wb"), &fclose);
    
    if (!fileOut)
        return Compressor::InvalidOutputFile;
    
    try
    {
        FileSequentialReadStream rs(fileIn.get());
        BitStreamReader r(&rs);
        
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
                size_t n = fwrite(&value, 1, sizeof(value), fileOut.get());
                check_true( n == sizeof(value) );
            }
        }
        
        check_true( HuffmanReader::Success == res );
    }
    catch (std::exception&)
    {
        return Compressor::InternalError;
    }
    
    return Compressor::Success;
}
