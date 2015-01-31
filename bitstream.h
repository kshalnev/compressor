#pragma once

#include "istream.h"
#include "common.h"

//
//
//

class BitStreamReader
{
public:
    BitStreamReader(ISequentialReadStream* stream);
    
    bool ReadBytes(void* data, unsigned int countBytes);
    bool ReadBits(void* data, unsigned int countBits);

    template <typename T>
    bool ReadBits(T* value) { return ReadBits(value, sizeof(T) * BitsPerByte); }
    
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
    
    template <typename T>
    bool WriteBits(const T& value) { return WriteBits(&value, sizeof(T) * BitsPerByte); }
    
private:
    bool Write(const void* data, unsigned int size);
    
private:
    BitStreamWriter(const BitStreamWriter&);
    BitStreamWriter& operator=(const BitStreamWriter&);
    
    unsigned char m_holder;
    unsigned char m_offset;
    ISequentialWriteStream* const m_stream;
};
