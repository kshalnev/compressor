#include "huffmancompressor.h"
#include "bitrlecompressor.h"
#include "lzwcompressor.h"
#include "streamimpl.h"
#include <cassert>

std::shared_ptr<ICompressor> GetCompressor(const char* compressorName)
{
    assert(nullptr != compressorName);
    
    if (0 == strcmp(compressorName, "huffman"))
    {
        return std::make_shared<Huffman>();
    }
    else if (0 == strcmp(compressorName, "bitrle"))
    {
        return std::make_shared<BitRle>();
    }
    else if (0 == strcmp(compressorName, "lzw"))
    {
        return std::make_shared<Lzw>();
    }
    else if (0 == strcmp(compressorName, "bitlzw"))
    {
        return std::make_shared<BitLzw>();
    }
    return std::shared_ptr<ICompressor>();
}

template <typename F>
void DoAction(ICompressor* compressor, const char* sourceFile, const char* destFile, F f)
{
    assert(nullptr != compressor);
    assert(nullptr != sourceFile);
    assert(nullptr != destFile);
    
    std::unique_ptr<FILE, decltype(&fclose)> fileIn(fopen(sourceFile, "rb"), &fclose);
    if (!fileIn)
    {
        throw std::invalid_argument("SourceFile");
    }
    
    std::unique_ptr<FILE, decltype(&fclose)> fileOut(fopen(destFile, "wb"), &fclose);
    if (!fileOut)
    {
        throw std::invalid_argument("DestinationFile");
    }
    
    FileReadStream rs(fileIn.get());
    FileSequentialWriteStream ws(fileOut.get());
    
    f(compressor, rs, ws);
}

template <typename F>
void DoAction(const char* compressorName, const char* sourceFile, const char* destFile, F f)
{
    assert(nullptr != compressorName);
    
    std::shared_ptr<ICompressor> compressor = GetCompressor(compressorName);
    if (!compressor)
    {
        throw std::invalid_argument("CompressorName");
    }
    
    DoAction(compressor.get(), sourceFile, destFile, f);
}

auto compressLambda = [](ICompressor* compressor, FileReadStream& rs, FileSequentialWriteStream& ws)
{ compressor->Compress(rs, ws); };

auto decompressLambda = [](ICompressor* compressor, FileReadStream& rs, FileSequentialWriteStream& ws)
{ compressor->Decompress(rs, ws); };

void Compress(ICompressor* compressor, const char* sourceFile, const char* destFile)
{
    DoAction(compressor, sourceFile, destFile, compressLambda);
}

void Decompress(ICompressor* compressor, const char* sourceFile, const char* destFile)
{
    DoAction(compressor, sourceFile, destFile, decompressLambda);
}

void Compress(const char* compressorName, const char* sourceFile, const char* destFile)
{
    DoAction(compressorName, sourceFile, destFile, compressLambda);
}

void Decompress(const char* compressorName, const char* sourceFile, const char* destFile)
{
    DoAction(compressorName, sourceFile, destFile, decompressLambda);
}
