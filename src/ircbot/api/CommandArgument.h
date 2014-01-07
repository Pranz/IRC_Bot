#ifndef __LOLIROFLE_IRCBOT_API_COMMANDARGUMENT_H_INCLUDED__
#define __LOLIROFLE_IRCBOT_API_COMMANDARGUMENT_H_INCLUDED__

#include <lolie/Stringp.h>

union CommandArgument{
	struct{
		unsigned int argCount;
		Stringp* args;
	}value;

	struct{
		const char* begin;
		const char* end;
	}free;
};

#endif
