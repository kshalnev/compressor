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

class ISequentialWriteStream
{
public:
    virtual bool Write(const void* data, unsigned int size) = 0;
    virtual ~ISequentialWriteStream() {}
};
