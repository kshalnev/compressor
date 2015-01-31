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
    
    virtual unsigned int Write(const void* data, unsigned int size);
    
private:
    ByteArraySequentialWriteStream(const ByteArraySequentialWriteStream&);
    ByteArraySequentialWriteStream& operator=(const ByteArraySequentialWriteStream&);
    
    std::vector<unsigned char>* const m_buff;
};

//
//
//

class ByteArrayReadStream : public IReadStream
{
public:
    ByteArrayReadStream(std::vector<unsigned char>* buff);
    
    virtual unsigned int Read(void* data, unsigned int size);
    virtual unsigned int GetPos();
    virtual bool Seek(unsigned int pos);

private:
    ByteArrayReadStream(const ByteArrayReadStream&);
    ByteArrayReadStream& operator=(const ByteArrayReadStream&);
    
    std::vector<unsigned char>* const m_buff;
    unsigned int m_index;
};

//
//
//

class FileSequentialWriteStream : public ISequentialWriteStream
{
public:
    FileSequentialWriteStream(FILE* file);
    
    virtual unsigned int Write(const void* data, unsigned int size);
    
private:
    FileSequentialWriteStream(const FileSequentialWriteStream&);
    FileSequentialWriteStream& operator=(const FileSequentialWriteStream&);
    
    FILE* const m_file;
};

//
//
//

class FileReadStream : public IReadStream
{
public:
    FileReadStream(FILE* file);
    
    virtual unsigned int Read(void* data, unsigned int size);
    virtual unsigned int GetPos();
    virtual bool Seek(unsigned int pos);
    
private:
    FileReadStream(const FileReadStream&);
    FileReadStream& operator=(const FileReadStream&);
    
    FILE* const m_file;
};
