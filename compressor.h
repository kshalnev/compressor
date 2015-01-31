#pragma once

class Compressor
{
public:
    enum Result { Success = 0, InvalidInputFile, InvalidOutputFile, InternalError, IOError };
    
    static Result Compress(const char* source, const char* dest);
    
    static Result Decompress(const char* source, const char* dest);
    
private:
    Compressor();
};
