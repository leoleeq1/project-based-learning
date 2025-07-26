#ifndef HASH_TABLE_H_
#define HASH_TABLE_H_

#include <stdint.h>

typedef struct
{
    char *key;
    char *value;
} hash_table_item_t;

typedef struct
{
    int32_t base_size;
    int32_t size;
    int32_t count;
    hash_table_item_t **items;
} hash_table_t;

hash_table_t *create_hash_table(const int32_t size);
void delete_hash_table(hash_table_t *table);
void hash_table_insert(hash_table_t *table, const char *key, const char *value);
char *hash_table_search(hash_table_t *table, const char *key);
void hash_table_delete(hash_table_t *table, const char *key);

#endif