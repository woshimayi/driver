#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define TABLE_SIZE 1024

// 哈希表节点结构
typedef struct HashNode
{
    char *key;
    char *values;
    // 可以添加与 key 相关联的值
    struct HashNode *next;
} HashNode;

// 哈希表结构
typedef struct HashTable
{
    HashNode *table[TABLE_SIZE];
} HashTable;

// DJB2 哈希函数
uint32_t djb2_hash(const char *str)
{
    uint32_t hash = 5381;
    int c;
    while ((c = *str++))
    {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }
    return hash % TABLE_SIZE;
}

// 创建一个新的哈希表
HashTable *create_hash_table()
{
    HashTable *ht = (HashTable *)malloc(sizeof(HashTable));
    if (ht == NULL)
    {
        perror("Failed to allocate hash table");
        return NULL;
    }
    for (int i = 0; i < TABLE_SIZE; i++)
    {
        ht->table[i] = NULL;
    }
    return ht;
}

// 插入键值对到哈希表
int hash_table_insert(HashTable *ht, const char *key, const char *values)
{
    uint32_t index = djb2_hash(key);
    HashNode *new_node = (HashNode *)malloc(sizeof(HashNode));
    if (new_node == NULL)
    {
        perror("Failed to allocate hash node");
        return;
    }
    new_node->key = strdup(key);       // 复制字符串
    new_node->values = strdup(values); // 复制字符串
    new_node->next = ht->table[index];
    ht->table[index] = new_node;
    return index;
}

// 在哈希表中查找键
int hash_table_lookup(HashTable *ht, const char *key, char **values)
{
    uint32_t index = djb2_hash(key);
    HashNode *current = ht->table[index];
    while (current != NULL)
    {
        if (strcmp(current->key, key) == 0)
        {
            *values = current->values;
            return 1; // 找到
        }
        current = current->next;
    }
    return 0; // 未找到
}

// 释放哈希表内存
void free_hash_table(HashTable *ht)
{
    if (ht == NULL)
        return;
    for (int i = 0; i < TABLE_SIZE; i++)
    {
        HashNode *current = ht->table[i];
        while (current != NULL)
        {
            HashNode *temp = current;
            current = current->next;
            free(temp->key);
            free(temp);
        }
    }
    free(ht);
}

int main()
{
    HashTable *my_hash_table = create_hash_table();
    if (my_hash_table == NULL)
    {
        return 1;
    }

    // 插入一些字符串
    printf("%d\n", hash_table_insert(my_hash_table, "apple", "apple_t"));
    printf("%d\n", hash_table_insert(my_hash_table, "banana", "banana_t"));
    printf("%d\n", hash_table_insert(my_hash_table, "cherry", "cherry_t"));
    printf("%d\n", hash_table_insert(my_hash_table, "date", "date_t"));
    printf("%d\n", hash_table_insert(my_hash_table, "elderberry", "elderberry_t"));
    printf("%d\n", hash_table_insert(my_hash_table, "status", "status_t"));

    // 查找字符串
    char *values;
    printf("Lookup 'banana': %s\n", hash_table_lookup(my_hash_table, "banana", &values) ? "Found" : "Not Found");
    printf("values = %s\n", values);
    printf("Lookup 'grape': %s\n", hash_table_lookup(my_hash_table, "grape", &values) ? "Found" : "Not Found");
    printf("values = %s\n", values);
    printf("Lookup 'apple': %s\n", hash_table_lookup(my_hash_table, "apple", &values) ? "Found" : "Not Found");
    printf("values = %s\n", values);
    printf("Lookup 'status': %s\n", hash_table_lookup(my_hash_table, "status", &values) ? "Found" : "Not Found");
    printf("values = %s\n", values);

    // 释放哈希表内存
    free_hash_table(my_hash_table);

    return 0;
}