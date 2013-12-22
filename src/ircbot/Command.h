#ifndef __LOLIROFLE_IRCBOT_COMMAND_H_INCLUDED__
#define __LOLIROFLE_IRCBOT_COMMAND_H_INCLUDED__

#include <lolie/Stringp.h>

enum CommandArgumentType{
	COMMAND_ARGUMENT_TYPE_INTEGER,
	COMMAND_ARGUMENT_TYPE_STRING,
	COMMAND_ARGUMENT_TYPE_FLOATINGPOINT,
	COMMAND_ARGUMENT_TYPE_CHARACTER
};

enum CommandArgumentRequirement{
	COMMAND_ARGUMENT_REQUIRED,
	COMMAND_ARGUMENT_OPTIONAL,
	COMMAND_ARGUMENT_VARARG
};

struct CommandArgument{
	Stringp name;
	enum CommandArgumentType type;
	enum CommandArgumentRequirement requirement;
};

struct Command{
	Stringp name;
	Stringp help;
	Stringp delimiter;
	struct CommandArgument args[];
};

#endif
