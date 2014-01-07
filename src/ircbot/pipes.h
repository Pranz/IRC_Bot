#ifndef __LOLIROFLE_ESSENTIALS_PIPES_H_INCLUDED__
#define __LOLIROFLE_ESSENTIALS_PIPES_H_INCLUDED__

#include <stdio.h>
#include <sys/types.h>

struct PipedStream{
	FILE* in;
	FILE* out;
	pid_t pid;
};

struct PipedStream p2open(char* path,char* const argv[]);
int p2close(struct PipedStream stream);

inline void p2flushWrite(struct PipedStream stream){
	if(stream.in)
		fclose(stream.in);
}

inline void p2flushRead(struct PipedStream stream){
	if(stream.out)
		fclose(stream.out);
}

#endif
