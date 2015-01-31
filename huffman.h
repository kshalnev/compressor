#pragma once

#include <vector>
#include "objstorage.h"


//
//
//

enum { ByteTypeCountValues = 256 };

//
//
//

typedef std::pair<unsigned int, unsigned int> CodeLength;

//
//
//

class HuffmanCodeTable
{
public:
    HuffmanCodeTable();
    HuffmanCodeTable(std::vector<CodeLength>&& codes);
    HuffmanCodeTable(HuffmanCodeTable&& other);
    HuffmanCodeTable(const HuffmanCodeTable& other);
    HuffmanCodeTable& operator=(HuffmanCodeTable&& other);
    HuffmanCodeTable& operator=(const HuffmanCodeTable& other);
    
    void SetCodeLength(unsigned char b, const CodeLength& codeLength);
    const CodeLength& GetCodeLength(unsigned char b) const;

    void swap(HuffmanCodeTable& other);
    
private:
    std::vector<CodeLength> m_codes;
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
