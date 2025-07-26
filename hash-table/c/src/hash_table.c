#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "hash_table.h"
#include "prime.h"

static hash_table_item_t DELETED_ITEM = {NULL, NULL};

static hash_table_item_t *create_item(const char *key, const char *value)
{
    hash_table_item_t *item = malloc(sizeof(hash_table_item_t));
    item->key = strdup(key);
    item->value = strdup(value);
    return item;
}

static void delete_item(hash_table_item_t *item)
{
    free(item->key);
    free(item->value);
    free(item);
}

static int32_t hash(const char *str, const int32_t prime, const int32_t mod)
{
    int64_t hash = 0LL;
    const int32_t length = strlen(str);

    for (int32_t i = 0; i < length; ++i)
    {
        hash += (int64_t)pow(prime, length - (i + 1)) * str[i];
    }
    hash = hash % mod;

    return (int32_t)hash;
}

static int32_t double_hash(const char *str, const int32_t num_buckets, const int32_t attempt)
{
    const int32_t hash_a = hash(str, 131, num_buckets);
    const int32_t hash_b = hash(str, 137, num_buckets - 1);
    return (hash_a + (attempt * (hash_b + 1))) % num_buckets;
}

hash_table_t *create_hash_table(const int32_t size)
{
    hash_table_t *table = malloc(sizeof(hash_table_t));
    table->base_size = size;
    table->size = next_prime(size);
    table->count = 0;
    table->items = calloc((size_t)table->size, sizeof(hash_table_item_t *));
    return table;
}

void delete_hash_table(hash_table_t *table)
{
    for (int32_t i = 0; i < table->size; ++i)
    {
        hash_table_item_t *item = table->items[i];
        if (item != NULL && item != &DELETED_ITEM)
        {
            delete_item(item);
        }
    }

    free(table->items);
    free(table);
}

static void resize_hash_table(hash_table_t *table, const int32_t size)
{
    if (size < 53)
    {
        return;
    }

    hash_table_t *new_table = create_hash_table(size);
    for (int32_t i = 0; i < table->size; ++i)
    {
        hash_table_item_t *item = table->items[i];
        if (item != NULL && item != &DELETED_ITEM)
        {
            hash_table_insert(new_table, item->key, item->value);
        }
    }

    table->base_size = new_table->base_size;
    table->count = new_table->count;

    const int32_t temp_size = table->size;
    table->size = new_table->size;
    new_table->size = temp_size;

    hash_table_item_t **temp_items = table->items;
    table->items = new_table->items;
    new_table->items = temp_items;

    delete_hash_table(new_table);
}

static void resize_up_hash_table(hash_table_t *table)
{
    resize_hash_table(table, table->base_size * 2);
}

static void resize_down_hash_table(hash_table_t *table)
{
    resize_hash_table(table, table->base_size / 2);
}

void hash_table_insert(hash_table_t *table, const char *key, const char *value)
{
    const int32_t load = table->count * 100 / table->size;
    if (load > 70)
    {
        resize_up_hash_table(table);
    }

    hash_table_item_t *item = create_item(key, value);
    int32_t index = double_hash(key, table->size, 0);
    hash_table_item_t *cur_item = table->items[index];

    int32_t i = 1;
    while (cur_item != NULL)
    {
        if (cur_item != &DELETED_ITEM && strcmp(cur_item->key, key) == 0)
        {
            delete_item(cur_item);
            table->items[index] = item;
            return;
        }
        index = double_hash(key, table->size, i++);
        cur_item = table->items[index];
    }

    table->items[index] = item;
    ++table->count;
}

char *hash_table_search(hash_table_t *table, const char *key)
{
    int32_t index = double_hash(key, table->size, 0);
    hash_table_item_t *item = table->items[index];

    int32_t i = 1;
    while (item != NULL)
    {
        if (item != &DELETED_ITEM && strcmp(item->key, key) == 0)
        {
            return item->value;
        }

        index = double_hash(key, table->size, i++);
        item = table->items[index];
    }

    return NULL;
}

void hash_table_delete(hash_table_t *table, const char *key)
{
    const int32_t load = table->count * 100 / table->size;
    if (load < 30)
    {
        resize_down_hash_table(table);
    }

    int32_t index = double_hash(key, table->size, 0);
    hash_table_item_t *item = table->items[index];

    int32_t i = 1;
    while (item != NULL)
    {
        if (item != &DELETED_ITEM && strcmp(item->key, key) == 0)
        {
            delete_item(item);
            table->items[index] = &DELETED_ITEM;
            --table->count;
            return;
        }

        index = double_hash(key, table->size, i++);
        item = table->items[index];
    }
}