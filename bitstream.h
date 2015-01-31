#pragma once

#include "istream.h"

//
//
//

enum { BitsPerByte = 8 };

//
//
//

class BitStreamReader
{
public:
    BitStreamReader(ISequentialReadStream* stream);
    
    bool ReadBytes(void* data, unsigned int countBytes);
    bool ReadBits(void* data, unsigned int countBits);

private:
    bool Read(void* data, unsigned int size);
    
private:
    BitStreamReader(const BitStreamReader&);
    BitStreamReader& operator=(const BitStreamReader&);
    
    unsigned char m_holder;
    unsigned char m_offset;
    ISequentialReadStream* const m_stream;
};

//
//
//

class BitStreamWriter
{
public:
    BitStreamWriter(ISequentialWriteStream* stream);
    
    bool WriteBytes(const void* data, unsigned int countBytes);
    bool WriteBits(const void* data, unsigned int countBits);
    bool CompleteByte();
    bool IsByteComplete() const;
    
private:
    bool Write(const void* data, unsigned int size);
    
private:
    BitStreamWriter(const BitStreamWriter&);
    BitStreamWriter& operator=(const BitStreamWriter&);
    
    unsigned char m_holder;
    unsigned char m_offset;
    ISequentialWriteStream* const m_stream;
};

//
//
//

inline unsigned char CountBits(unsigned int value)
{
    if (0 == value) return 1;
    unsigned char res = 0;
    for (; 0 != value; ++res, value >>= 1);
    return res;
}
