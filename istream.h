#pragma once

//
//
//

class ISequentialReadStream
{
public:
    virtual bool Read(void* data, unsigned int size) = 0;
    virtual ~ISequentialReadStream() {}
};

//
//
//

class IReadStream : public ISequentialReadStream
{
public:
    virtual unsigned int GetPos() = 0;
    virtual bool Seek(unsigned int pos) = 0;
};

//
//
//

class ISequentialWriteStream
{
public:
    virtual bool Write(const void* data, unsigned int size) = 0;
    virtual ~ISequentialWriteStream() {}
};
