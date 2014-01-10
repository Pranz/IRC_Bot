#ifndef __LOLIROFLE_IRCBOT_API_COMMANDARGUMENT_H_INCLUDED__
#define __LOLIROFLE_IRCBOT_API_COMMANDARGUMENT_H_INCLUDED__

#include <lolie/Stringp.h>
#include <lolie/TypeAliases.h>

/**
 * This union represents the parsed argument based on the reflecting CommandParameterType
 */
union CommandArgument{
	long integer;
	Stringcp string;
	double floatingPoint;
	uint character;
	bool boolean;
	ushort values;
	void* custom;
	struct{
		const char* begin;
		const char* end;
	}free;
};

#endif
