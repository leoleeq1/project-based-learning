#ifndef HASH_TABLE_H_
#define HASH_TABLE_H_

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include "prime.hpp"

struct HashTableItem
{
    std::string key;
    std::string value;
};

class HashTable
{
public:
    HashTable() : HashTable(DefaultSize) {}
    HashTable(const size_t size);
    ~HashTable();

    void Insert(const std::string_view key, const std::string_view value);
    const std::string_view Search(const std::string_view key);
    void Delete(const std::string_view key);
    size_t GetBaseSize() { return baseSize_; }
    size_t GetSize() { return size_; }

private:
    void Resize(const size_t size);
    int32_t Hash(std::string_view key, int32_t prime, int32_t mod);
    int32_t DoubleHash(std::string_view key, int32_t numBuckets, int32_t attempt);

    static const size_t DefaultSize = 53;

    size_t baseSize_;
    size_t size_;
    size_t count_;
    std::vector<HashTableItem *> items_;
};

#endif