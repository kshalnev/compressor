#pragma once

class BitRle
{
public:
    static bool Compress(const char* source, const char* dest);
    static bool Decompress(const char* source, const char* dest);
};
