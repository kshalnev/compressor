#pragma once

//
//
//

enum { BitsPerByte = 8 };

//
//
//

struct CodeLength
{
public:
    CodeLength() : code(0), length(0) {}
    CodeLength(unsigned int Code, unsigned int Length) : code(Code), length(Length) {}
    unsigned int code;
    unsigned int length;
};

inline bool operator==(const CodeLength& a, const CodeLength& b)
{
    return (a.code == b.code && a.length == b.length);
}

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
