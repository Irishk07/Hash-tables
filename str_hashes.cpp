#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "str_hashes.h"


unsigned int HashStrSum(const char* key, int capacity) {
    assert(key);

    unsigned int sum = 0;
    while (*key != '\0')
        sum += (unsigned char)(*(key++));

    return sum % (unsigned int)capacity;
}

unsigned int GNUHash(const char* key, int capacity) {
    assert(key);

    unsigned int hash = 5381;
    while (*key != '\0')
        hash = hash * 33 ^ (unsigned int)(*(key++));

    return hash % (unsigned int)capacity;
}
