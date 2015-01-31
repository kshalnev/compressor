#pragma once

#include <functional>
#include "common.h"

//
//
//

class BitRleTable
{
public:
    BitRleTable()
    : m_minB(0), m_maxB(0), m_minRepeats(0), m_maxRepeats(0)
    {}
    BitRleTable(unsigned char minB, unsigned int maxB, unsigned int minRepeats, unsigned char maxRepeats)
    : m_minB(minB), m_maxB(maxB), m_minRepeats(minRepeats), m_maxRepeats(maxRepeats)
    {}

    unsigned char GetMinValue() const { return m_minB; }
    unsigned char GetMaxValue() const { return m_maxB; }
    unsigned char GetMinRepeats() const { return m_minRepeats; }
    unsigned char GetMaxRepeats() const { return m_maxRepeats; }
    unsigned char GetValueLength() const { return CountBits(m_maxB - m_minB); }
    unsigned char GetRepeatsLength() const { return CountBits(m_maxRepeats - m_minRepeats); }
    
private:
    unsigned char m_minB;
    unsigned char m_maxB;
    unsigned char m_minRepeats;
    unsigned char m_maxRepeats;
};

//
//
//

class BitRleScanner
{
public:
    BitRleScanner();
    
    void BeginScan();
    void Scan(unsigned char b);
    void EndScan(BitRleTable& table, unsigned int& totalLen);
    
private:
    BitRleScanner(const BitRleScanner&);
    BitRleScanner& operator=(const BitRleScanner&);
    
    enum State { state_none, state_scanning };
    State m_state;
    
    unsigned int m_cnt;
    unsigned char m_b;
    unsigned char m_repeats;
    
    unsigned char m_minRepeats;
    unsigned char m_maxRepeats;
    unsigned char m_minB;
    unsigned char m_maxB;
};

//
//
//

class BitRleCompressor
{
public:
    BitRleCompressor(const BitRleTable& table);
    
    void BeginCompress(std::function<void(const CodeLength& b, const CodeLength& repeats)> sink);
    void Compress(unsigned char b);
    void EndCompress();
    
private:
    BitRleCompressor(const BitRleCompressor&);
    BitRleCompressor& operator=(const BitRleCompressor&);
    
    void Notify();
    
    enum State { state_none, state_compressing };
    State m_state;

    const unsigned char m_minB;
    const unsigned char m_minRepeats;
    const unsigned char m_lengthB;
    const unsigned char m_lengthRepeats;
    
    unsigned char m_b;
    unsigned char m_repeats;
    
    std::function<void(const CodeLength& b, const CodeLength& repeats)> m_sink;
};
