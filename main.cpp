#include "huffmancompressor.h"
#include "bitrlecompressor.h"
#include <iostream>

typedef BitRle CompressorType;

static void PrintUsage()
{
    std::cout << "Arguments list for compression  : -c <compressor> <file path source> <file path destination>" << std::endl;
    std::cout << "Arguments list for decompression: -d <compressor> <file path source> <file path destination>" << std::endl;
    std::cout << "<compressor> can be 'bitrle' or 'huffman'" << std::endl;
}

int main(int argc, const char * argv[])
{
    if (argc != 5)
    {
        PrintUsage();
        return -1;
    }
    
    bool doCompressing = false;
    
    if (0 == strcmp(argv[1], "-c"))
    {
        doCompressing = true;
    }
    else if (0 == strcmp(argv[1], "-d"))
    {
        doCompressing = false;
    }
    else
    {
        PrintUsage();
        return -1;
    }
    
    bool res = false;
    
    if (0 == strcmp(argv[2], "bitrle"))
    {
        res = doCompressing ? BitRle::Compress(argv[3], argv[4]) : BitRle::Decompress(argv[3], argv[4]);
    }
    else if (0 == strcmp(argv[2], "huffman"))
    {
        res = doCompressing ? Huffman::Compress(argv[3], argv[4]) : Huffman::Decompress(argv[3], argv[4]);
    }
    else
    {
        PrintUsage();
        return -1;
    }
    
    std::cout << (res ? "Succeeded" : "Failed") << std::endl;
    
    return res ? 0 : -1;
}
