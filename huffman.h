#pragma once

#include <vector>
#include <unordered_map>
#include "common.h"
#include "objstorage.h"

//
//
//

enum { ByteTypeCountValues = 256 };

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

    struct Node;
    struct NodeCodeLength;
    
    HuffmanCodeTable BuildCodesTable() const;
    Node* BuildTree(ObjectStorage<Node>& storage) const;
    HuffmanCodeTable BuildCodesTableFromTree(Node* root) const;
    
    enum State { state_none, state_scanning };
    
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

    static Node* RestoreTree(const HuffmanCodeTable& codes, ObjectStorage<Node>& storage);
    
    ObjectStorage<Node> m_storage;
    Node* m_root;
    Node* m_current;
};
