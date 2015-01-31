#include "compressor.h"
#include <iostream>

int testCompressorResult(Compressor::Result res)
{
    switch (res)
    {
        case Compressor::Success:
            std::cout << "Succeeded" << std::endl;
            return 0;
            
        case Compressor::InvalidInputFile:
            std::cout << "Invalid input file" << std::endl;
            return -1;
            
        case Compressor::InvalidOutputFile:
            std::cout << "Invalid output file" << std::endl;
            return -1;
            
        case Compressor::InternalError:
            std::cout << "Internal error" << std::endl;
            return -1;
            
        case Compressor::IOError:
            std::cout << "IO error" << std::endl;
            return -1;
            
        default:
            std::cout << "Unknown error" << std::endl;
            return -1;
    }
}

int main(int argc, const char * argv[])
{
    int res = -1;
    
    bool paramsOk = false;
    if (argc == 4)
    {
        if (0 == strcmp(argv[1], "-c"))
        {
            paramsOk = true;
            res = testCompressorResult( Compressor::Compress(argv[2], argv[3]) );
        }
        else if (0 == strcmp(argv[1], "-d"))
        {
            paramsOk = true;
            res = testCompressorResult( Compressor::Decompress(argv[2], argv[3]) );
        }
    }
    
    if (!paramsOk)
    {
        std::cout << "Arguments list for compression  : -c <file path source> <file path destination>" << std::endl;
        std::cout << "Arguments list for decompression: -d <file path source> <file path destination>" << std::endl;
    }
    
    return res;
}
