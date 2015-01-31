#include "bitstream.h"
#include <cassert>

//
//
//

BitStreamReader::BitStreamReader(ISequentialReadStream* stream)
: m_holder(0)
, m_offset(BitsPerByte)
, m_stream(stream)
{
    assert(nullptr != stream);
}

bool BitStreamReader::ReadBytes(void* data, unsigned int countBytes)
{
    assert(nullptr != data);
    
    if (!Read(data, countBytes))
        return false;
    m_holder = 0;
    m_offset = BitsPerByte;
    return true;
}

bool BitStreamReader::ReadBits(void* data, unsigned int countBits)
{
    assert(nullptr != data);
    
    unsigned char* b = reinterpret_cast<unsigned char*>(data);
    
    unsigned int o = 0;
    unsigned int c = countBits;
    
    *b = 0;
    
    while (c > 0)
    {
        if (BitsPerByte == m_offset)
        {
            if (!Read(&m_holder, sizeof(m_holder)))
                return false;
            m_offset = 0;
        }
        
        if (BitsPerByte == o)
        {
            ++b;
            *b = 0;
            o = 0;
        }
        
        unsigned int space = BitsPerByte - m_offset;
        
        unsigned int ss = (c <= space) ? c : space;
        unsigned int ds = BitsPerByte - o;
        unsigned int s = (ss < ds) ? ss : ds;
        
        unsigned int mask = (1 << s) - 1;
        
        unsigned int val = (m_holder >> m_offset) & mask;
        
        *b |= (val << o);
        
        o += s;
        m_offset += s;
        c -= s;
    }
    
    return true;
}

bool BitStreamReader::Read(void* data, unsigned int size)
{
    return m_stream->Read(data, size);
}

//
//
//

BitStreamWriter::BitStreamWriter(ISequentialWriteStream* stream)
: m_holder(0)
, m_offset(0)
, m_stream(stream)
{
    assert(nullptr != stream);
}

bool BitStreamWriter::WriteBytes(const void* data, unsigned int countBytes)
{
    assert(nullptr != data);
    
    if (!IsByteComplete())
    {
        if (!CompleteByte())
            return false;
    }
    return Write(data, countBytes);
}

bool BitStreamWriter::WriteBits(const void* data, unsigned int countBits)
{
    assert(nullptr != data);
    
    const unsigned char* b = reinterpret_cast<const unsigned char*>(data);
    
    unsigned int i = 0;
    unsigned int c = countBits;
    
    while (c > 0)
    {
        if (BitsPerByte == m_offset)
        {
            if (!Write(&m_holder, sizeof(m_holder)))
                return false;
            m_holder = 0;
            m_offset = 0;
        }
        
        if (BitsPerByte == i)
        {
            ++b;
            i = 0;
        }
        
        unsigned int space = BitsPerByte - m_offset;
        
        unsigned int ss = (c <= space) ? c : space;
        unsigned int ds = BitsPerByte - i;
        unsigned int s = (ss < ds) ? ss : ds;
        
        unsigned int mask = (1 << s) - 1;
        
        unsigned int val = (*b >> i) & mask;
        
        m_holder |= (val << m_offset);
        
        i += s;
        m_offset += s;
        c -= s;
    }
    
    return true;
}

bool BitStreamWriter::CompleteByte()
{
    if (0 != m_offset)
    {
        if (!Write(&m_holder, sizeof(m_holder)))
            return false;
        m_holder = 0;
        m_offset = 0;
    }
    return true;
}

bool BitStreamWriter::IsByteComplete() const
{
    return (m_offset == 0);
}

bool BitStreamWriter::Write(const void* data, unsigned int size)
{
    return m_stream->Write(data, size);
}
