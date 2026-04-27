#ifndef STR_H_
#define STR_H_

#include <stdio.h>

#include "common.h"
#include "ht.h"


struct about_hash {
  const char* name = NULL;
  hash_func_t hash_func = NULL;
};

unsigned int HashStrSum(const char* key, int capacity);

unsigned int GNUHash(const char* key, int capacity);


#define HASH_FUNC(func) \
  {#func, func},

static const about_hash hash_func_table[] {
  HASH_FUNC(GNUHash)
  HASH_FUNC(HashStrSum)
};


#endif // STR_H_