#ifndef __LOLIROFLE_IRCBOT_COMMAND_H_INCLUDED__
#define __LOLIROFLE_IRCBOT_COMMAND_H_INCLUDED__

#include <lolie/Stringp.h>

struct IRCBot;

enum CommandParameterType{
	COMMAND_PARAMETER_TYPE_SEPARATED,
	COMMAND_PARAMETER_TYPE_FREE
};

enum CommandParameterValueType{
	COMMAND_PARAMETER_VALUE_TYPE_INTEGER,
	COMMAND_PARAMETER_VALUE_TYPE_STRING,
	COMMAND_PARAMETER_VALUE_TYPE_FLOATINGPOINT,
	COMMAND_PARAMETER_VALUE_TYPE_CHARACTER
};

enum CommandParameterValueRequirement{
	COMMAND_PARAMETER_VALUE_REQUIRED,
	COMMAND_PARAMETER_VALUE_OPTIONAL,
	COMMAND_PARAMETER_VALUE_VARARG
};

struct CommandParameterValue{
	Stringp name;
	enum CommandParameterValueType type;
	enum CommandParameterValueRequirement requirement;
};

union CommandArgument{
	struct{
		unsigned int argCount;
		Stringp* args;
	}value;

	struct{
		const char* arg_begin;
		const char* arg_end;
	}free;
};

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
