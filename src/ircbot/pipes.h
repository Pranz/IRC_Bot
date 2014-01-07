#ifndef __LOLIROFLE_ESSENTIALS_PIPES_H_INCLUDED__
#define __LOLIROFLE_ESSENTIALS_PIPES_H_INCLUDED__

#include <stdio.h>

struct PipedStream{
	FILE* in;
	FILE* out;
};

struct PipedStream p2open(char* path,char* const argv[]);
void p2close(struct PipedStream stream);

inline void p2flushWrite(struct PipedStream stream){
	fclose(stream.in);
}

inline void p2flushRead(struct PipedStream stream){
	fclose(stream.out);
}

#endif
