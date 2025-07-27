#include <cstdint>
#include <print>
#include <string>
#include <string_view>
#include <vector>
#include <utility>

#include "hash_table.hpp"

int32_t main(int32_t argc, char **argv)
{
    std::vector<std::pair<std::string, std::string>> pairs = {
        {"a", "1"},
        {"a", "2"}, // Test double insert
        {"aa", "3"},
        {"ab", "4"},
        {"ac", "5"},
        {"ad", "6"},
        {"ae", "7"},
        {"af", "8"},
        {"ag", "9"},
        {"ah", "10"},
        {"ai", "11"},
        {"aj", "12"},
        {"ak", "13"},
        {"al", "14"},
        {"am", "15"},
        {"an", "16"},
        {"ao", "17"},
        {"ap", "18"},
        {"aq", "19"},
        {"ar", "20"},
        {"as", "21"},
        {"at", "22"},
        {"au", "23"},
        {"av", "24"},
        {"aw", "25"},
        {"ax", "26"},
        {"ay", "27"},
        {"az", "28"},
        {"ba", "29"},
        {"bb", "30"},
        {"bc", "31"},
        {"bd", "32"},
        {"be", "33"},
        {"bf", "34"},
        {"bg", "35"},
        {"bh", "36"},
        {"bi", "37"},
        {"bj", "38"},
        {"bk", "39"},
        {"bl", "40"},
        {"bm", "41"},
        {"bn", "42"},
        {"bo", "43"},
        {"bp", "44"},
        {"bq", "45"},
        {"br", "46"},
        {"bs", "47"},
        {"bt", "48"},
        {"bu", "49"},
        {"bv", "50"},
        {"bw", "51"},
        {"bx", "52"},
        {"by", "53"},
        {"bz", "54"},
    };

    HashTable table(50);

    std::println("size: {}", pairs.size());

    for (auto &p : pairs)
    {
        table.Insert(p.first, p.second);
    }

    for (size_t i = pairs.size() / 3; i < pairs.size() / 3 * 2; ++i)
    {
        table.Delete(pairs[i].first);
    }

    for (size_t i = pairs.size() / 3; i < pairs.size() / 3 * 2; ++i)
    {
        table.Delete(pairs[i].first);
    }

    size_t counter{};
    for (auto &p : pairs)
    {
        std::string_view value = table.Search(p.first);
        if (!value.empty())
        {
            std::println("{} pair: {} | {}", counter++, p.first, value);
        }
    }

    std::println("BaseSize: {}, Size: {}", table.GetBaseSize(), table.GetSize());

    return 0;
}