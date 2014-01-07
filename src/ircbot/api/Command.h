#ifndef __LOLIROFLE_IRCBOT_API_COMMAND_H_INCLUDED__
#define __LOLIROFLE_IRCBOT_API_COMMAND_H_INCLUDED__

#include <lolie/Stringp.h>
#include "CommandArgument.h"
#include "CommandParameter.h"

struct IRCBot;

struct Command{
	Stringcp name;
	Stringcp help;
	bool(*func)(struct IRCBot* bot,Stringcp target,union CommandArgument* arg);
	enum CommandParameterType argType;
	union{
		struct{
			unsigned int argCount;
			struct CommandParameter* args;
		}value;
	}args;
};

#endif
