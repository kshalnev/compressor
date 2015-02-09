#pragma once

#include <vector>
#include <functional>
#include <unordered_map>

struct ByteVectorHash
{
    size_t operator()(const std::vector<unsigned char>& key) const;
};

class LzwCompressor
{
public:
    LzwCompressor();
    
    void Begin(std::function<void(unsigned int)> out);
    void Put(unsigned char b);
    void End();
    
private:
    typedef std::vector<unsigned char> Sequence;
    typedef std::unordered_map<Sequence, unsigned int, ByteVectorHash> Sequence2Code;
    
    enum State { state_none, state_compressing } m_state;
    
    Sequence2Code m_codes;
    Sequence m_current;
    
    std::function<void(unsigned int)> m_out;
};

class LzwDecompressor
{
public:
    LzwDecompressor();

    void Begin(std::function<void(const std::vector<unsigned char>&)> out);
    bool Put(unsigned int code);
    void End();
    
private:
    typedef std::vector<unsigned char> Sequence;
    
    class Codes
    {
    public:
        const unsigned int* findCode(const Sequence& s) const;
        const Sequence* findSequence(unsigned int c) const;
        bool insert(const Sequence& s);
        void init();
        void clear();
        size_t size() const;
        
    private:
        typedef std::unordered_map<unsigned int, Sequence> Code2Sequence;
        typedef std::unordered_map<Sequence, unsigned int, ByteVectorHash> Sequence2Code;
        
        Sequence2Code m_codes;
        Code2Sequence m_sequences;
    };
    
    enum State { state_none, state_decompressing } m_state;
    
    Codes m_codes;
    Sequence m_prev;
    
    std::function<void(const std::vector<unsigned char>&)> m_out;
};
