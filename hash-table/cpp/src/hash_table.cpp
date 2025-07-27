#include <cstdint>
#include <cmath>
#include <string>
#include <string_view>
#include <vector>

#include "hash_table.hpp"
#include "prime.hpp"

static HashTableItem DeletedItem{};

HashTable::HashTable(const size_t size)
{
    baseSize_ = size;
    size_ = nextPrime(size);
    count_ = 0;
    items_.resize(size_, nullptr);
}

HashTable::~HashTable()
{
    for (auto item : items_)
    {
        if (item != nullptr && item != &DeletedItem)
        {
            delete item;
        }
    }
}

void HashTable::Insert(const std::string_view key, const std::string_view value)
{
    const size_t load = count_ * 100 / size_;
    if (load > 70)
    {
        Resize(baseSize_ * 2);
    }

    size_t index = DoubleHash(key, size_, 0);
    HashTableItem *item = items_[index];

    size_t i = 1;
    while (item != nullptr)
    {
        if (item != &DeletedItem && item->key.compare(key) == 0)
        {
            item->value = value;
            return;
        }

        index = DoubleHash(key, size_, i++);
        item = items_[index];
    }

    items_[index] = new HashTableItem{std::string(key), std::string(value)};
    ++count_;
}

const std::string_view HashTable::Search(const std::string_view key)
{
    size_t index = DoubleHash(key, size_, 0);
    HashTableItem *item = items_[index];

    size_t i = 1;
    while (item != nullptr)
    {
        if (item != &DeletedItem && item->key.compare(key) == 0)
        {
            return item->value;
        }

        index = DoubleHash(key, size_, i++);
        item = items_[index];
    }

    return {};
}

void HashTable::Delete(const std::string_view key)
{
    const size_t load = count_ * 100 / size_;
    if (load < 30)
    {
        Resize(baseSize_ / 2);
    }

    size_t index = DoubleHash(key, size_, 0);
    HashTableItem *item = items_[index];

    size_t i = 1;
    while (item != nullptr)
    {
        if (item != &DeletedItem && item->key.compare(key) == 0)
        {
            delete item;
            items_[index] = &DeletedItem;
            --count_;
            return;
        }

        index = DoubleHash(key, size_, i++);
        item = items_[index];
    }
}

void HashTable::Resize(const size_t size)
{
    if (size < DefaultSize)
    {
        return;
    }

    HashTable temp(size);
    for (size_t i = 0; i < size_; ++i)
    {
        if (items_[i] != nullptr && items_[i] != &DeletedItem)
        {
            temp.Insert(items_[i]->key, items_[i]->value);
        }
    }

    baseSize_ = temp.baseSize_;
    size_ = temp.size_;
    count_ = temp.count_;
    items_.swap(temp.items_);
}

int32_t HashTable::Hash(std::string_view key, int32_t prime, int32_t mod)
{
    int64_t hash{};
    for (size_t i = 0; i < key.length(); ++i)
    {
        hash += static_cast<int64_t>(pow(prime, key.length() - (i + 1)) * key[i]);
    }
    hash = hash % mod;

    return static_cast<int32_t>(hash);
}

int32_t HashTable::DoubleHash(std::string_view key, int32_t numBuckets, int32_t attempt)
{
    const int32_t Prime1 = 131;
    const int32_t Prime2 = 137;
    const int32_t hash_a = Hash(key, Prime1, numBuckets);
    const int32_t hash_b = Hash(key, Prime2, numBuckets - 1);
    return (hash_a + (attempt * (hash_b + 1))) % numBuckets;
}