#include <assert.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "read.h"

#include "common.h"


static void SkipSpaces(char** str) {
    assert(str != NULL);
    assert(*str != NULL);

    while (**str != '\0' && isspace(**str))
        (*str)++;
}

int SizeOfText(const char *filename) {
    struct stat text_info = {};

    if (stat(filename, &text_info) == -1) {
        perror("Error is");

        return -1;
    }

    return (int)text_info.st_size;
}

status Read(about_text *text, const char* filename) {
    assert(text != NULL);
    assert(filename != NULL);

    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error is");
        return OPEN_ERROR;
    }

    text->text_size = SizeOfText(filename);
    if (text->text_size == -1)
        return STAT_ERROR;

    text->buffer = (char*)calloc((size_t)(text->text_size + 1), sizeof(char));
    if (text->buffer == NULL) {
        perror("Error is");

        return NOT_ENOUGH_MEMORY;
    }

    char* temp = text->buffer;

    size_t read_size = fread((char *)text->buffer, sizeof(char), (size_t)text->text_size, file);
    if (read_size != (size_t)text->text_size) {
        if (ferror(file))
            fprintf(stderr, "Error: problem with reading file\n");
        else
            fprintf(stderr, "Error: unexpected end of file (read %zu of %zu)\n", 
                    read_size, (size_t)text->text_size);
        
        free(text->buffer);
        fclose(file);

        return READ_ERROR;
    }

    SkipSpaces(&temp);
    for ( ; *temp != '\0'; ++temp) {
        if (isspace(*temp)) {
            ++text->cnt_words;

            SkipSpaces(&temp);
            if (*temp == '\0')
                break;
        }
        
    }

    if (fclose(file) == EOF) {
        perror("Error is");
        return CLOSE_ERROR; 
    }

    return SUCCESS;
}

status Fragmentation(about_text *text) {
    assert(text->buffer != NULL);

    size_t cell_size = 64; 
    text->aligned_buffer = (char*)aligned_alloc(32, (size_t)text->cnt_words * cell_size);
    if (text->aligned_buffer == NULL)
        return NOT_ENOUGH_MEMORY;
    memset(text->aligned_buffer, 0, (size_t)text->cnt_words * cell_size);

    text->pointers_on_words = (about_word*)calloc((size_t)text->cnt_words, sizeof(about_word));
    if (text->pointers_on_words == NULL) {
        free(text->aligned_buffer);

        return NOT_ENOUGH_MEMORY;
    }

    char* temp = text->buffer; 
    for (int i = 0; i < text->cnt_words; ++i) {
        SkipSpaces(&temp);
        if (*temp == '\0') 
            break;

        char* word_start = temp;
        int len = 0;
        while (temp[len] != '\0' && !isspace(temp[len]))
            ++len;

        char* cell_ptr = text->aligned_buffer + ((size_t)i * cell_size);

        int copy_len = (len > 63) ? 63 : len;
        memcpy(cell_ptr, word_start, (size_t)copy_len);
        cell_ptr[copy_len] = '\0';

        text->pointers_on_words[i].point = cell_ptr;
        text->pointers_on_words[i].size = len;

        temp += len;
    }

    return SUCCESS;
}

void TextDtor(about_text* text) {
    if (text == NULL) 
        return;

    free(text->aligned_buffer);
    free(text->buffer);
    free(text->pointers_on_words);
}