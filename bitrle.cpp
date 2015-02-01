#include "bitrle.h"
#include <cassert>

//
//
//

BitRleTable::BitRleTable()
: m_minB(0), m_maxB(0), m_minRepeats(0), m_maxRepeats(0), m_valueLength(0), m_repeatsLength(0)
{}

BitRleTable::BitRleTable(unsigned char minB, unsigned int maxB, unsigned int minRepeats, unsigned char maxRepeats)
: m_minB(minB), m_maxB(maxB), m_minRepeats(minRepeats), m_maxRepeats(maxRepeats)
, m_valueLength(CountBits(maxB - minB))
, m_repeatsLength(CountBits(maxRepeats - minRepeats))
{
    assert(minB <= maxB);
    assert(minRepeats <= maxRepeats);
}

//
//
//

BitRleScanner::BitRleScanner()
: m_state(state_none)
, m_cnt(0)
, m_b(0)
, m_repeats(0)
, m_minRepeats(0)
, m_maxRepeats(0)
, m_minB(0)
, m_maxB(0)
{
}

void BitRleScanner::BeginScan()
{
    assert(state_none == m_state);
    
    m_state = state_scanning;

    m_cnt = 0;
    m_b = 0;
    m_repeats = 0;
    m_minRepeats = 255;
    m_maxRepeats = 0;
    m_minB = 0;
    m_maxB = 0;
}

void BitRleScanner::Scan(unsigned char b)
{
    assert(state_scanning == m_state);
    
    if (0 == m_repeats)
    {
        m_b = b;
        m_repeats = 1;
        m_minB = m_maxB = b;
    }
    else
    {
        if (m_b == b)
        {
            if (255 == m_repeats)
            {
                m_maxRepeats = 255;

                m_b = b;
                m_repeats = 1;
                ++m_cnt;
            }
            else
            {
                m_repeats += 1;
            }
        }
        else
        {
            if (m_b < m_minB) m_minB = m_b;
            else if (m_maxB < m_b) m_maxB = m_b;
            
            if (m_repeats < m_minRepeats) m_minRepeats = m_repeats;
            else if (m_maxRepeats < m_repeats) m_maxRepeats = m_repeats;
            
            m_b = b;
            m_repeats = 1;
            ++m_cnt;
        }
    }
}

void BitRleScanner::EndScan(BitRleTable& table, unsigned int& totalLen)
{
    assert(state_scanning == m_state);
    
    if (m_repeats == 0)
    {
        table = BitRleTable();
        totalLen = 0;
    }
    else
    {
        ++m_cnt;
        
        if (m_b < m_minB) m_minB = m_b;
        else if (m_maxB < m_b) m_maxB = m_b;
        
        if (m_repeats < m_minRepeats) m_minRepeats = m_repeats;
        else if (m_maxRepeats < m_repeats) m_maxRepeats = m_repeats;
        
        table = BitRleTable(m_minB, m_maxB, m_minRepeats, m_maxRepeats);
        totalLen = m_cnt * (table.GetValueLength() + table.GetRepeatsLength());
    }
    
    m_state = state_none;
}

//
//
//

BitRleCompressor::BitRleCompressor(const BitRleTable& table)
: m_state(state_none)
, m_minB(table.GetMinValue())
, m_minRepeats(table.GetMinRepeats())
, m_lengthB(table.GetValueLength())
, m_lengthRepeats(table.GetRepeatsLength())
, m_b(0)
, m_repeats(0)
{
}

void BitRleCompressor::BeginCompress(std::function<void(const CodeLength& b, const CodeLength& repeats)> sink)
{
    assert(state_none == m_state);

    m_b = 0;
    m_repeats = 0;
    m_sink = sink;

    m_state = state_compressing;
}

void BitRleCompressor::Compress(unsigned char b)
{
    assert(state_compressing == m_state);
    
    if (0 == m_repeats)
    {
        m_b = b;
        m_repeats = 1;
    }
    else
    {
        if (m_b == b)
        {
            if (m_repeats == 255)
            {
                Notify();
                
                m_b = b;
                m_repeats = 1;
            }
            else
            {
                m_repeats += 1;
            }
        }
        else
        {
            Notify();
            
            m_b = b;
            m_repeats = 1;
        }
    }
}

void BitRleCompressor::EndCompress()
{
    assert(state_compressing == m_state);
    
    if (0 != m_repeats)
    {
        Notify();
    }
    
    m_state = state_none;
}

void BitRleCompressor::Notify()
{
    CodeLength cb(m_b - m_minB, m_lengthB);
    CodeLength cr(m_repeats - m_minRepeats, m_lengthRepeats);
    m_sink(cb, cr);
}
