/* sudo nice -n -20 taskset -c 0 ./hash */

#include <stdio.h>
#include <stdlib.h>
#include <x86intrin.h>

#include "common.h"
#include "ht.h"
#include "read.h"
#include "str_hashes.h"


static int Comparator(const void* a, const void* b) {
    unsigned long long val1 = *(const unsigned long long*)a;
    unsigned long long val2 = *(const unsigned long long*)b;
    return (val1 > val2) - (val1 < val2);
}


int main(int argc, char** argv) {
    const char* input_file  = "input/input.txt";
    const char* search_file = "search/search.txt";
    hash_func_t hash_func   = NULL;
    if (argc >= 2) hash_func   = hash_func_table[atoi(argv[1])].hash_func;    
    if (argc >= 3) input_file  = argv[2];
    if (argc >= 4) search_file = argv[3];

    about_text input_text = {};
    if (Read(&input_text, input_file) != SUCCESS || Fragmentation(&input_text) != SUCCESS) {
        TextDtor(&input_text);
        return 1;
    }  
    about_text search_text = {};
    if (Read(&search_text, search_file) != SUCCESS || Fragmentation(&search_text) != SUCCESS) {
        TextDtor(&input_text); TextDtor(&search_text);
        return 1;
    }

    status status_of_work = SUCCESS;
    chain_table_t* hash_table = ChainInit(START_CAPACITY, LOAD_FACTOR, hash_func, &status_of_work);
    for (int i = 0; i < input_text.cnt_words; ++i){
        if (ChainInsert(hash_table, &input_text.pointers_on_words[i]) != SUCCESS)
            return 1;
    }    

    for (int j = 0; j < 2; ++j) {
        for (int i = 0; i < search_text.cnt_words; ++i)
            ChainSearch(hash_table, &search_text.pointers_on_words[i]);     
    }

    unsigned long long times[CNT_REPEATS] = {};

    for (int k = 0; k < CNT_REPEATS; ++k) {
        volatile unsigned long long start = __rdtsc();
        for (int j = 0; j < CNT_TESTS; ++j) {
            for (int i = 0; i < search_text.cnt_words; ++i)
                ChainSearch(hash_table, &search_text.pointers_on_words[i]);
        }        
        volatile unsigned long long end = __rdtsc();

        times[k] = end - start;
    }   
    
    qsort(times, CNT_REPEATS, sizeof(unsigned long long), Comparator);
    unsigned long long res_time = 0;
    for (int i = 1; i < CNT_REPEATS - 1; ++i) 
        res_time += times[i];

    unsigned long long avg_time = 0;     
    if (CNT_REPEATS > 1) {
        avg_time = res_time / (CNT_REPEATS - 2);
        printf("Time: %llu ticks\n", avg_time);
    }

    #ifdef OUT_FILE
        FILE* file = fopen(OUT_FILE, "w");
        if (file) {
            for (int i = 1; i < CNT_REPEATS - 1; ++i)
                fprintf(file, "%llu\n", times[i]);
            fprintf(file, "%llu\n", avg_time);
            fclose(file);
        }
    #endif

    ChainFree(hash_table);

    TextDtor(&input_text);
    TextDtor(&search_text);

    return 0;
}