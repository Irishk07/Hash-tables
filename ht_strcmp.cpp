#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <immintrin.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ht.h"

#include "common.h"


extern "C" size_t My_strlen(const char* str);


static unsigned int HashCrc32(const char* str, int capacity) {
    assert(str);

    unsigned long long hash = 0xFFFFFFFF;
    size_t len = My_strlen(str);

    size_t i = 0;
    for (; i + 8 <= len; i += 8) {
        unsigned long long cur_ptr = *(const unsigned long long*)(str + i);
        hash = _mm_crc32_u64(hash, cur_ptr);
    }

    for (; i < len; ++i)
        hash = _mm_crc32_u8((unsigned int)hash, (unsigned char)str[i]);

    return (unsigned int)hash % (unsigned int)capacity;
}

static int My_strcmp(const char* str1, const char* str2) {
    if (str1 == NULL || str2 == NULL) 
        return 1;

    int result = 0;

    asm volatile (
        ".intel_syntax noprefix           \n\t"
        
        "mov rdx, %[s1]                   \n\t" // address of str1
        "mov rcx, %[s2]                   \n\t" // address of str2

        "mov al, [rdx]                    \n\t" // check first bytes
        "cmp al, [rcx]                    \n\t"
        "jne 2f                           \n\t" 

        "vmovdqu ymm0, [rdx]              \n\t" // mov ymm0, 32 bytes of str1
        "vpcmpeqb ymm0, ymm0, [rcx]       \n\t" // ymm0 = compare of ymm0 and 32 bytes of str2
        "vpmovmskb eax, ymm0              \n\t" // make mask
        
        "xor %[res], %[res]               \n\t" 
        "cmp eax, -1                      \n\t" // cmp with FFFFFFFF (equal)
        "jne 2f                           \n\t" 

        "vmovdqu ymm1, [rdx + 32]         \n\t" // mov ymm1, next 32 bytes of str1
        "vpcmpeqb ymm1, ymm1, [rcx + 32]  \n\t" // ymm1 = compare of ymm1 and next 32 bytes of str2
        "vpmovmskb eax, ymm1              \n\t" // make mask
        
        "cmp eax, -1                      \n\t" // cmp with FFFFFFFF (equal)
        "je 1f                            \n\t" 

        "2:                               \n\t" 
        "mov %[res], 1                    \n\t" 

        "1:                               \n\t"
        "vzeroupper                       \n\t"
        ".att_syntax                      \n\t"

        : [res] "=&r" (result)                                  // exit args (& uniq) (= only write)
        : [s1]  "r"   (str1),                                   // enter args (r any)
          [s2]  "r"   (str2)
        : "ymm0", "ymm1", "rax", "rdx", "rcx", "cc", "memory"   // destroyed
    );

    return result;
}

static chain_node_t* CreateNode(about_word* key, status* status_of_work) {
    assert(key);

    chain_node_t* new_node = (chain_node_t*)calloc(1, sizeof(chain_node_t));
    if (new_node == NULL) {
        *status_of_work = NOT_ENOUGH_MEMORY;

        return NULL;
    }

    new_node->key = key;
    new_node->next = NULL;

    return new_node;
}

chain_table_t* ChainInit(int capacity, float max_load_factor, status* status_of_work) {
    chain_table_t* hash_table = (chain_table_t*)calloc(1, sizeof(chain_table_t));
    if (hash_table == NULL) {
        *status_of_work = NOT_ENOUGH_MEMORY;

        return NULL;
    }

    hash_table->table = (chain_node_t**)calloc((size_t)capacity, sizeof(chain_node_t*));
    if (hash_table->table == NULL) {
        free(hash_table);
        *status_of_work = NOT_ENOUGH_MEMORY;

        return NULL;
    }

    hash_table->capacity = capacity;
    hash_table->size = 0;
    hash_table->max_load_factor = max_load_factor;
    
    return hash_table;
}

status ChainInsert(chain_table_t* hash_table, about_word* key) {
    assert(hash_table);
    assert(hash_table->table);
    assert(key);

    if ((float)hash_table->size / (float)hash_table->capacity >= hash_table->max_load_factor) {
        status status_rehash = ChainRehash(hash_table);
        if (status_rehash != SUCCESS)
            return status_rehash;
    }
    
    unsigned int index = HashCrc32(key->point, hash_table->capacity);

    chain_node_t* cur_node = hash_table->table[index];
    while (cur_node != NULL) {
        if (cur_node->key->size == key->size && My_strcmp(cur_node->key->point, key->point) == 0)
            return SUCCESS;

        cur_node = cur_node->next;
    }

    status status_of_work = SUCCESS;
    chain_node_t* new_node = CreateNode(key, &status_of_work);
    if (new_node == NULL)
        return status_of_work;

    new_node->next = hash_table->table[index];
    hash_table->table[index] = new_node;
    hash_table->size++;

    return SUCCESS;
}


status ChainRehash(chain_table_t* hash_table) {
    assert(hash_table);
    assert(hash_table->table); 

    int old_capacity = hash_table->capacity;
    chain_node_t** old_table = hash_table->table;

    hash_table->capacity *= 2;
    hash_table->table = (chain_node_t**)calloc((size_t)hash_table->capacity, sizeof(chain_node_t*));
    if (hash_table->table == NULL)
        return NOT_ENOUGH_MEMORY;
    hash_table->size = 0;

    for (int i = 0; i < old_capacity; ++i) {
        chain_node_t* cur_node = old_table[i];
        while (cur_node != NULL) {
            status status_rehash = ChainInsert(hash_table, cur_node->key);
            if (status_rehash != SUCCESS){
                free(hash_table->table);
                hash_table->table = old_table;

                return status_rehash;
            } 

            chain_node_t* temp = cur_node;
            cur_node = cur_node->next;
            free(temp);
        }
    }

    free(old_table);

    return SUCCESS;
}

bool ChainSearch(chain_table_t* hash_table, about_word* key) {
    assert(hash_table);
    assert(hash_table->table);

    unsigned int index = HashCrc32(key->point, hash_table->capacity);
    chain_node_t* cur_node = hash_table->table[index];
    while (cur_node != NULL) {
        if (cur_node->key->size == key->size && My_strcmp(cur_node->key->point, key->point) == 0) 
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