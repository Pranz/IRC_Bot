#ifndef __LOLIROFLE_IRCBOT_API_COMMANDARGUMENT_H_INCLUDED__
#define __LOLIROFLE_IRCBOT_API_COMMANDARGUMENT_H_INCLUDED__

#include <lolie/Stringp.h>

union CommandArgument{
	struct{
		const char* begin;
		const char* end;
	}free;

	long integer;
	Stringcp string;
	double floatingPoint;
	char character;
	void* custom;
};

#endif
