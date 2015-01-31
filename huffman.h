#pragma once

#include <vector>
#include <unordered_map>
#include "objstorage.h"


//
//
//

enum { ByteTypeCountValues = 256 };

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

bool operator==(const CodeLength& a, const CodeLength& b);

//
//
//

class HuffmanCodeTable
{
public:
    HuffmanCodeTable();
    HuffmanCodeTable(std::unordered_map<unsigned char, CodeLength>&& codes);
    HuffmanCodeTable(HuffmanCodeTable&& other);
    HuffmanCodeTable(const HuffmanCodeTable& other);
    HuffmanCodeTable& operator=(HuffmanCodeTable&& other);
    HuffmanCodeTable& operator=(const HuffmanCodeTable& other);
    
    void SetCodeLength(unsigned char b, const CodeLength& codeLength);
    const CodeLength& GetCodeLength(unsigned char b) const;

    void swap(HuffmanCodeTable& other);
    
private:
    std::unordered_map<unsigned char, CodeLength> m_codes;
};

//
//
//

class HuffmanScanner
{
public:
    HuffmanScanner();
    
    void BeginScan();
    void Scan(unsigned char b);
    void EndScan(HuffmanCodeTable& table, unsigned int& totalLen);
    
private:
    HuffmanScanner(const HuffmanScanner&);
    HuffmanScanner& operator=(const HuffmanScanner&);
    
    HuffmanCodeTable BuildCodesTable();
    
    enum State { state_none, state_scanning, state_scanned };
    struct Node;
    struct NodeCodeLength;
    
    State m_state;
    std::vector<unsigned int> m_bytes;
    unsigned int m_count;
};

//
//
//

class HuffmanReader
{
public:
    HuffmanReader(const HuffmanCodeTable& codes);
    ~HuffmanReader();
    
    enum Result { Success = 0, ReadMoreBits, NoData };

    Result ReadBit(unsigned int bit, unsigned char* value);
    
private:
    HuffmanReader(const HuffmanReader&);
    HuffmanReader& operator=(const HuffmanReader&);
    
    struct Node;
    
    ObjectStorage<Node> m_storage;
    Node* m_root;
    Node* m_current;
};
