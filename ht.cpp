#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ht.h"

#include "common.h"


static unsigned int HashCrc32(const char* key, int capacity) {
    assert(key);

    uint32_t crc = 0xFFFFFFFF;
    while (*key != '\0')
        crc = (crc << 8) ^ crc32_table[((crc >> 24) ^ (uint8_t)*key++) & 0xFF];

    return (crc ^ 0xFFFFFFFF) % (unsigned int)capacity;
}


static chain_node_t* CreateNode(about_word* key) {
    assert(key);

    chain_node_t* new_node = (chain_node_t*)calloc(1, sizeof(chain_node_t));

    new_node->key = key;
    new_node->next = NULL;

    return new_node;
}

chain_table_t* ChainInit(int capacity, float max_load_factor) {
    chain_table_t* hash_table = (chain_table_t*)calloc(1, sizeof(chain_table_t));
    if (hash_table == NULL) 
        return NULL;

    hash_table->table = (chain_node_t**)calloc((size_t)capacity, sizeof(chain_node_t*));
    if (hash_table->table == NULL) {
        free(hash_table);

        return NULL;
    }

    hash_table->capacity = capacity;
    hash_table->size = 0;
    hash_table->max_load_factor = max_load_factor;
    
    return hash_table;
}

void ChainInsert(chain_table_t* hash_table, about_word* key) {
    assert(hash_table);
    assert(hash_table->table);
    assert(key);

    if ((float)hash_table->size / (float)hash_table->capacity >= hash_table->max_load_factor) 
        ChainRehash(hash_table);
    
    unsigned int index = HashCrc32(key->point, hash_table->capacity);

    chain_node_t* cur_node = hash_table->table[index];
    while (cur_node != NULL) {
        if (cur_node->key->size == key->size && strcmp(cur_node->key->point, key->point) == 0)
            return;

        cur_node = cur_node->next;
    }

    chain_node_t* new_node = CreateNode(key);
    new_node->next = hash_table->table[index];
    hash_table->table[index] = new_node;
    hash_table->size++;
}


void ChainRehash(chain_table_t* hash_table) {
    assert(hash_table);
    assert(hash_table->table); 

    int old_capacity = hash_table->capacity;
    chain_node_t** old_table = hash_table->table;

    hash_table->capacity *= 2;
    hash_table->table = (chain_node_t**)calloc((size_t)hash_table->capacity, sizeof(chain_node_t*));
    if (hash_table->table == NULL)
        return;
    hash_table->size = 0;

    for (int i = 0; i < old_capacity; ++i) {
        chain_node_t* cur_node = old_table[i];
        while (cur_node != NULL) {
            ChainInsert(hash_table, cur_node->key);

            chain_node_t* temp = cur_node;
            cur_node = cur_node->next;
            free(temp);
        }
    }

    free(old_table);
}

bool ChainSearch(chain_table_t* hash_table, about_word* key) {
    assert(hash_table);
    assert(hash_table->table);

    unsigned int index = HashCrc32(key->point, hash_table->capacity);
    chain_node_t* cur_node = hash_table->table[index];
    while (cur_node != NULL) {
        if (cur_node->key->size == key->size && strcmp(cur_node->key->point, key->point) == 0) 
            return true;

        cur_node = cur_node->next;
    }

    return false;
}

void ChainFree(chain_table_t* hash_table) {
    if (hash_table == NULL) return;

    if (hash_table->table != NULL) {
        for (int i = 0; i < hash_table->capacity; ++i) {
            chain_node_t* cur_node = hash_table->table[i];
            while (cur_node != NULL) {
                chain_node_t* temp = cur_node;
                cur_node = cur_node->next;
                
                free(temp); 
            }
        }
    }

    free(hash_table->table);
    free(hash_table);
}