#pragma once

class Huffman
{
public:
    static bool Compress(const char* source, const char* dest);
    static bool Decompress(const char* source, const char* dest);
};
