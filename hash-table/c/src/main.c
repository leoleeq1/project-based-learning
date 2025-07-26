#include <stdint.h>
#include <stdio.h>

#include "hash_table.h"

typedef struct
{
    const char *key;
    const char *value;
} pair_t;

int32_t main(int32_t argc, char **argv)
{
    const pair_t key_value_pairs[] =
        {
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

    hash_table_t *table = create_hash_table(50);

    int32_t size = sizeof(key_value_pairs) / sizeof(key_value_pairs[0]);

    printf("size: %d\n", size);
    for (int32_t i = 0; i < size; ++i)
    {
        hash_table_insert(table, key_value_pairs[i].key, key_value_pairs[i].value);
    }

    for (int32_t i = size / 3; i < size / 3 * 2; ++i)
    {
        hash_table_delete(table, key_value_pairs[i].key);
    }

    // Test double free
    for (int32_t i = size / 3; i < size / 3 * 2; ++i)
    {
        hash_table_delete(table, key_value_pairs[i].key);
    }

    int32_t counter = 0;
    for (int32_t i = 0; i < table->size; ++i)
    {
        hash_table_item_t *item = table->items[i];
        if (item != NULL && item->key != NULL)
        {
            printf("%d pair: %s | %s\n", counter++, item->key, item->value);
        }
    }

    printf("base_size: %d, size: %d\n", table->base_size, table->size);

    delete_hash_table(table);
    return 0;
}