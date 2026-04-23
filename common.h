#ifndef COMMON_H_
#define COMMON_H_

#include <stdio.h>

#ifdef IF_NOINLINE
#define INLINE inline __attribute__((noinline))
#else // IF_NOINLINE
#define INLINE
#endif // IF_NOINLINE

#ifdef VALGRIND
const int CNT_REPEATS = 1;
#else // VALGRIND
const int CNT_REPEATS = 5;
#endif // VALGRIND

const int CNT_TESTS = 10;


const float LOAD_FACTOR  = 10.0;
const int START_CAPACITY = 1024;

enum status {
    SUCCESS               = 0,
    OPEN_ERROR            = 1,
    STAT_ERROR            = 2,
    NOT_ENOUGH_MEMORY     = 3,
    READ_ERROR            = 4,
    CLOSE_ERROR           = 5
};

struct about_word {
    int size = 0;
    char* point = NULL;
};

struct about_text {
    char* buffer = NULL;
    char* aligned_buffer = NULL;
    int cnt_words = 0;
    int text_size = 0;
    about_word* pointers_on_words = NULL;
};


#endif //COMMON_H_