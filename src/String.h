#ifndef __LOLIROFLE_IRC_STRING_H_INCLUDED__
#define __LOLIROFLE_IRC_STRING_H_INCLUDED__

#include <stdlib.h>
#include <stdio.h>
#include "Bool.h"

/**
 * String with length type
 */
typedef struct string{
	char* chrs;
	size_t len;
}string;
#define STRING(chrs,len) (string){chrs,len}

#define fputstringn(str,stream) fputsn((str).chrs,(str).len,stream)
#define putstringn(str) fputstringn((str),stdout)

bool strneq(const char* str1,const char* str2,size_t len);
void fputsn(const char* str,size_t len,FILE* stream);
size_t sputstring(string out,unsigned short count, ...);

#endif
