#ifndef READ_H_
#define READ_H_

#include "common.h"


int SizeOfText(const char *filename);

status Read(about_text *text, const char* filename);

status Fragmentation(about_text *text);

void TextDtor(about_text* text);


#endif //READ_H_