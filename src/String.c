#include "String.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

bool strneq(const char* str1,const char* str2,size_t len){
	while(len--)
		if(*str1++!=*str2++)
			return false;
	return true;
}

void fputsn(const char* str,size_t len,FILE* stream){
	while(len--)
		fputc(*str++,stream);
}

size_t sputstring(string out,unsigned short count, ...){//TODO: Test security
	size_t out_len = 0;
	va_list args;

	va_start(args,count);
	while(count-->0){
		string str=va_arg(args,string);
		out_len+=str.len;
		
		if(out_len>out.len){
			memcpy(out.chrs,str.chrs,str.len-(out_len-out.len));
			va_end(args);		
			return out.len;
		}

		memcpy(out.chrs,str.chrs,str.len);
		out.chrs+=str.len;
	}
	va_end(args);

	return out_len;
}
