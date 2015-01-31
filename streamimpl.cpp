#include "streamimpl.h"
#include <algorithm>
#include <cassert>

//
//
//

ByteArraySequentialWriteStream::ByteArraySequentialWriteStream(std::vector<unsigned char>* buff)
: m_buff(buff)
{
    assert(nullptr != buff);
}

unsigned int ByteArraySequentialWriteStream::Write(const void* data, unsigned int size)
{
    assert(nullptr != data);
    
    const unsigned char* b = reinterpret_cast<const unsigned char*>(data);
    std::copy(b, b + size, std::back_inserter(*m_buff));
    return size;
}

//
//
//

ByteArrayReadStream::ByteArrayReadStream(std::vector<unsigned char>* buff)
: m_buff(buff)
, m_index(0)
{
    assert(nullptr != buff);
}

unsigned int ByteArrayReadStream::Read(void* data, unsigned int size)
{
    assert(nullptr != data);
    
    if (m_index >= m_buff->size())
        return 0;
    
    if ((m_buff->size() - m_index) < size)
        size = static_cast<unsigned int>(m_buff->size() - m_index);
    
    unsigned char* b = reinterpret_cast<unsigned char*>(data);
    std::copy(m_buff->begin() + m_index, m_buff->begin() + m_index + size, b);
    m_index += size;
    return size;
}

unsigned int ByteArrayReadStream::GetPos()
{
    return m_index;
}

bool ByteArrayReadStream::Seek(unsigned int pos)
{
    if (pos >= m_buff->size())
        return false;
    m_index = pos;
    return true;
}

//
//
//

FileSequentialWriteStream::FileSequentialWriteStream(FILE* file)
: m_file(file)
{
    assert(nullptr != m_file);
}

unsigned int FileSequentialWriteStream::Write(const void* data, unsigned int size)
{
    assert(nullptr != data);
    
    size_t res = fwrite(data, 1, size, m_file);
    return static_cast<unsigned int>(res);
}

//
//
//

FileReadStream::FileReadStream(FILE* file)
: m_file(file)
{
    assert(nullptr != file);
}

unsigned int FileReadStream::Read(void* data, unsigned int size)
{
    assert(nullptr != data);
    
    size_t res = fread(data, 1, size, m_file);
    return static_cast<unsigned int>(res);
}

unsigned int FileReadStream::GetPos()
{
    const long pos = ftell(m_file);
    if (pos < 0) return false;
    return static_cast<unsigned int>(pos);
}

bool FileReadStream::Seek(unsigned int pos)
{
    return (0 == fseek(m_file, pos, SEEK_SET));
}
