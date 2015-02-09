#include "lzw.h"
#include <cassert>

//
//
//

size_t ByteVectorHash::operator()(const std::vector<unsigned char>& key) const {
    std::hash<unsigned char> sh;
    size_t h = 0;
    for (size_t i = 0, n = key.size(); i < n; ++i)
    {
        h = h * 31 + sh(key[i]);
    }
    return h;
};

//
//
//

LzwCompressor::LzwCompressor()
: m_state(state_none)
{}

void LzwCompressor::Begin(std::function<void(unsigned int)> out)
{
    assert(state_none == m_state);
    
    for (unsigned int i = 0; i <= 255; ++i)
    {
        Sequence s = { static_cast<unsigned char>(i) };
        m_codes[s] = i;
    }
    
    m_state = state_compressing;
    
    m_out = out;
}

void LzwCompressor::Put(unsigned char b)
{
    assert(state_compressing == m_state);
    
    m_current.push_back(b);
    
    auto itr = m_codes.find(m_current);
    if (itr == m_codes.end())
    {
        unsigned int c = static_cast<unsigned int>(m_codes.size());
        m_codes.insert(make_pair(m_current, c));
        
        m_current.pop_back();
        
        itr = m_codes.find(m_current);
        assert(m_codes.end() != itr);
        
        m_out(itr->second);
        
        m_current.clear();
        m_current.push_back(b);
    }
}

void LzwCompressor::End()
{
    assert(state_compressing == m_state);
    
    if (!m_current.empty())
    {
        auto itr = m_codes.find(m_current);
        assert(m_codes.end() != itr);
        
        m_out(itr->second);
    }
    
    m_codes.clear();
    m_current.clear();
    
    m_state = state_none;
}

//
//
//

LzwDecompressor::LzwDecompressor()
: m_state(state_none)
{}

void LzwDecompressor::Begin(std::function<void(const std::vector<unsigned char>&)> out)
{
    assert(state_none == m_state);
    
    m_state = state_decompressing;
    
    m_codes.init();
    
    m_out = out;
}

bool LzwDecompressor::Put(unsigned int code)
{
    assert(state_decompressing == m_state);
    
    const Sequence* const s = m_codes.findSequence(code);
    if (s == nullptr)
    {
        if (m_prev.empty() || code > m_codes.size())
            return false;
        m_prev.push_back(m_prev[0]);
        m_out(m_prev);
        m_codes.insert(m_prev);
    }
    else
    {
        m_out(*s);
        m_prev.push_back(s->operator[](0));
        m_codes.insert(m_prev);
        m_prev = *s;
    }
    return true;
}

void LzwDecompressor::End()
{
    assert(state_decompressing == m_state);
    
    m_codes.clear();
    m_prev.clear();
    
    m_state = state_none;
}

//

const unsigned int* LzwDecompressor::Codes::findCode(const Sequence& s) const
{
    auto itr = m_codes.find(s);
    if (m_codes.end() != itr) return &itr->second;
    else return nullptr;
}

const LzwDecompressor::Sequence* LzwDecompressor::Codes::findSequence(unsigned int c) const
{
    auto i = m_sequences.find(c);
    if (i != m_sequences.end()) return &i->second;
    else return nullptr;
}

bool LzwDecompressor::Codes::insert(const Sequence& s)
{
    auto itr = m_codes.find(s);
    const bool res = (m_codes.end() == itr);
    if (res)
    {
        unsigned int c = static_cast<unsigned int>(m_codes.size());
        m_codes[s] = c;
        m_sequences[c] = s;
    }
    return res;
}

void LzwDecompressor::Codes::init()
{
    m_codes.clear();
    m_sequences.clear();
    for (unsigned int i = 0; i <= 255; ++i)
    {
        Sequence s = { static_cast<unsigned char>(i) };
        m_sequences[i] = s;
        m_codes[s] = i;
    }
}

void LzwDecompressor::Codes::clear()
{
    m_codes.clear();
    m_sequences.clear();
}

size_t LzwDecompressor::Codes::size() const
{
    return m_codes.size();
}
