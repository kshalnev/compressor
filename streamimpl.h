#pragma once

#include "istream.h"
#include <vector>

//
//
//

class ByteArraySequentialWriteStream : public ISequentialWriteStream
{
public:
    ByteArraySequentialWriteStream(std::vector<unsigned char>* buff);
    
    virtual bool Write(const void* data, unsigned int size);
    
private:
    ByteArraySequentialWriteStream(const ByteArraySequentialWriteStream&);
    ByteArraySequentialWriteStream& operator=(const ByteArraySequentialWriteStream&);
    
    std::vector<unsigned char>* const m_buff;
};

//
//
//

class ByteArraySequentialReadStream : public ISequentialReadStream
{
public:
    ByteArraySequentialReadStream(std::vector<unsigned char>* buff);
    
    virtual bool Read(void* data, unsigned int size);

private:
    ByteArraySequentialReadStream(const ByteArraySequentialReadStream&);
    ByteArraySequentialReadStream& operator=(const ByteArraySequentialReadStream&);
    
    std::vector<unsigned char>* const m_buff;
    size_t m_index;
};

//
//
//

class FileSequentialWriteStream : public ISequentialWriteStream
{
public:
    FileSequentialWriteStream(FILE* file);
    
    virtual bool Write(const void* data, unsigned int size);
    
private:
    FileSequentialWriteStream(const FileSequentialWriteStream&);
    FileSequentialWriteStream& operator=(const FileSequentialWriteStream&);
    
    FILE* const m_file;
};

//
//
//

class FileSequentialReadStream : public ISequentialReadStream
{
public:
    FileSequentialReadStream(FILE* file);
    
    virtual bool Read(void* data, unsigned int size);
    
private:
    FileSequentialReadStream(const FileSequentialReadStream&);
    FileSequentialReadStream& operator=(const FileSequentialReadStream&);
    
    FILE* const m_file;
};
