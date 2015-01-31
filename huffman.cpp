#include "huffman.h"

#include <cassert>
#include <stack>
#include <algorithm>

//
//
//

enum { BitLeft = 1, BitRight = 0 };

//
//
//

HuffmanCodeTable::HuffmanCodeTable()
: m_codes(ByteTypeCountValues)
{}

HuffmanCodeTable::HuffmanCodeTable(std::vector<CodeLength>&& codes)
: m_codes(std::move(codes))
{
    assert(ByteTypeCountValues == m_codes.size());
}

HuffmanCodeTable::HuffmanCodeTable(HuffmanCodeTable&& other)
: m_codes(std::move(other.m_codes))
{
}

HuffmanCodeTable::HuffmanCodeTable(const HuffmanCodeTable& other)
: m_codes(other.m_codes)
{}

HuffmanCodeTable& HuffmanCodeTable::operator=(HuffmanCodeTable&& other)
{
    m_codes = std::move(other.m_codes);
    return *this;
}

HuffmanCodeTable& HuffmanCodeTable::operator=(const HuffmanCodeTable& other)
{
    m_codes = other.m_codes;
    return *this;
}

void HuffmanCodeTable::SetCodeLength(unsigned char b, const CodeLength& codeLength)
{
    m_codes[b] = codeLength;
}

const CodeLength& HuffmanCodeTable::GetCodeLength(unsigned char b) const
{
    return m_codes[b];
}

void HuffmanCodeTable::swap(HuffmanCodeTable& other)
{
    m_codes.swap(other.m_codes);
}

//
//
//

HuffmanScanner::HuffmanScanner()
: m_state(state_none)
, m_bytes(ByteTypeCountValues)
, m_count(0)
{}

void HuffmanScanner::BeginScan()
{
    assert(state_none == m_state);
    
    if (0 != m_count)
    {
        std::fill(m_bytes.begin(), m_bytes.end(), 0);
        m_count = 0;
    }
    
    m_state = state_scanning;
}

void HuffmanScanner::Scan(unsigned char b)
{
    assert(state_scanning == m_state);
    
    if (0 == m_bytes[b]) m_count += 1;
    m_bytes[b] += 1;
}

void HuffmanScanner::EndScan(HuffmanCodeTable& table, unsigned int& totalLen)
{
    assert(state_scanning == m_state);
    
    m_state = state_none;
    
    if (m_count == 0)
    {
        table = HuffmanCodeTable();
        totalLen = 0;
    }
    else
    {
        table = BuildCodesTable();
        
        totalLen = 0;
        for (unsigned int i = 0; i < ByteTypeCountValues; ++i)
        {
            if (0 != m_bytes[i])
            {
                const CodeLength& codeLen = table.GetCodeLength(static_cast<unsigned char>(i));
                totalLen += m_bytes[i] * codeLen.second;
            }
        }
    }
}

struct HuffmanScanner::Node
{
    Node(unsigned int c = 0, unsigned char b = 0) : left(nullptr), right(nullptr), value(b), count(c) {}
    Node* left;
    Node* right;
    unsigned char value;
    unsigned int count;
};

struct HuffmanScanner::NodeCodeLength
{
    Node* node;
    unsigned int len;
    unsigned int code;
};

HuffmanCodeTable HuffmanScanner::BuildCodesTable()
{
    assert(m_count != 0);
    
    ObjectStorage<Node> storage;
    
    std::vector<Node*> v;
    v.reserve(m_count);
    for (size_t i = 0, n = m_bytes.size(); i < n; ++i)
    {
        if (0 != m_bytes[i])
        {
            Node* n = storage.NewObject(m_bytes[i], i);
            v.push_back(n);
        }
    }
    std::sort(v.begin(), v.end(), [](const Node* a, const Node* b) { return a->count < b->count; });
    
    for (size_t i = 0; i < (v.size() - 1);)
    {
        Node* min = storage.NewObject(v[i]->count + v[i + 1]->count);
        min->left = v[i];
        min->right = v[i + 1];
        v[i] = nullptr;
        v[i + 1] = min;
        ++i;
        for (size_t j = i; (j < (v.size() - 1)) && (v[j]->count >= v[j + 1]->count); ++j)
        {
            std::swap(v[j], v[j + 1]);
        }
    }
    
    Node* root = v[v.size() - 1];
    v.clear();
    
    std::vector<CodeLength> codes;
    codes.resize(ByteTypeCountValues);
    
    std::stack<NodeCodeLength> s;
    NodeCodeLength n;
    n.node = root;
    n.len = 0;
    n.code = 0;
    s.push(n);
    while (!s.empty())
    {
        NodeCodeLength n = s.top();
        s.pop();
        if (nullptr != n.node->left)
        {
            NodeCodeLength l;
            l.node = n.node->left;
            l.len = n.len + 1;
            l.code = n.code | (BitLeft << n.len);
            s.push(l);
        }
        if (nullptr != n.node->right)
        {
            NodeCodeLength r;
            r.node = n.node->right;
            r.len = n.len + 1;
            r.code = n.code | (BitRight << n.len);
            s.push(r);
        }
        if (nullptr == n.node->left && nullptr == n.node->right)
        {
            assert(CodeLength(0, 0) == codes[n.node->value]);
            assert(n.len > 0);
            codes[n.node->value] = CodeLength(n.code, n.len);
        }
    }
    
    return HuffmanCodeTable(std::move(codes));
}

//
//
//

struct HuffmanReader::Node
{
    Node(unsigned char v = 0) : left(nullptr), right(nullptr), value(v) {}
    
    Node* left;
    Node* right;
    unsigned char value;
};

HuffmanReader::HuffmanReader(const HuffmanCodeTable& codes)
: m_root(nullptr)
, m_current(nullptr)
{
    ObjectStorage<Node> storage;
    
    Node* root = storage.NewObject();
    
    for (unsigned int i = 0; i < ByteTypeCountValues; ++i)
    {
        Node* n = root;
        
        const unsigned char b = static_cast<unsigned char>(i);
        const CodeLength& codeLength = codes.GetCodeLength(b);
        for (unsigned int j = 0; j < codeLength.second; ++j)
        {
            unsigned int bit = (codeLength.first >> j) & 1;
            if (BitLeft == bit)
            {
                if (nullptr == n->left) n->left = storage.NewObject();
                n = n->left;
            }
            else
            {
                if (nullptr == n->right) n->right = storage.NewObject();
                n = n->right;
            }
        }
        
        n->value = i;
    }
    
    m_storage.swap(storage);
    m_current = m_root = root;
}

HuffmanReader::~HuffmanReader()
{
}

HuffmanReader::Result HuffmanReader::ReadBit(unsigned int bit, unsigned char* value)
{
    assert(nullptr != value);
    assert(BitLeft == bit || BitRight == bit);
    
    m_current = (BitLeft == bit) ? m_current->left : m_current->right;

    if (nullptr == m_current)
    {
        m_current = m_root;

        return HuffmanReader::NoData;
    }
    else if (nullptr == m_current->left && nullptr == m_current->right)
    {
        *value = m_current->value;
        
        m_current = m_root;
        
        return HuffmanReader::Success;
    }
    else
    {
        return HuffmanReader::ReadMoreBits;
    }
}
