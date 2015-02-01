#include "compressor.h"
#include <iostream>

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

    bool compress = false;
    if (0 == strcmp(argv[1], "-c"))
    {
        compress = true;
    }
    else if (0 == strcmp(argv[1], "-d"))
    {
        compress = false;
    }
    else
    {
        PrintUsage();
        return -1;
    }
    
    bool res = false;
    
    try
    {
        if (compress)
        {
            Compress(argv[2], argv[3], argv[4]);
        }
        else
        {
            Decompress(argv[2], argv[3], argv[4]);
        }
        
        res = true;
        
        std::cout << "Succeeded" << std::endl;
    }
    catch (std::exception& e)
    {
        std::cout << "Exception: " << e.what() << std::endl;
    }
    
    return res ? 0 : -1;
}
