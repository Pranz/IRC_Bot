#ifndef __LOLIROFLE_IRCBOT_API_COMMANDPARAMETER_H_INCLUDED__
#define __LOLIROFLE_IRCBOT_API_COMMANDPARAMETER_H_INCLUDED__

#include <lolie/Stringp.h>
#include <lolie/TypeAliases.h>

enum CommandParameterSeparation{
	COMMAND_PARAMETER_SEPARATION_NONE,
	COMMAND_PARAMETER_SEPARATION_CHARACTER,
	COMMAND_PARAMETER_SEPARATION_CHARACTERLIST,
	COMMAND_PARAMETER_SEPARATION_CUSTOMPARSER
};

enum CommandParameterType{
	COMMAND_PARAMETER_TYPE_INTEGER,
	COMMAND_PARAMETER_TYPE_STRING,
	COMMAND_PARAMETER_TYPE_FLOATINGPOINT,
	COMMAND_PARAMETER_TYPE_CHARACTER,
	COMMAND_PARAMETER_TYPE_BOOL,
	COMMAND_PARAMETER_TYPE_VALUES,
	COMMAND_PARAMETER_TYPE_CUSTOMPARSER
};

enum CommandParameterRequirement{
	COMMAND_PARAMETER_REQUIRED,
	COMMAND_PARAMETER_OPTIONAL,
	COMMAND_PARAMETER_VARARG
};

struct CommandParameter{
	Stringp name;
	Stringp helpType;
	enum CommandParameterType type;
	union{
		struct{
			bool(*restrictFunc)(long value);
		}integer;
		
		struct{
			bool(*restrictFunc)(Stringcp value);
		}string;

		struct{
			bool(*restrictFunc)(double value);
		}floatingPoint;

		struct{
			bool(*restrictFunc)(uint value);
		}character;

		struct{
			void*(*parse)(Stringcp arg);
		}customParser;
	}typeData;
	enum CommandParameterRequirement requirement;
	enum CommandParameterSeparation separation;
	union{
		char character;
		char* characterList;
		//TODO: Custom parser
	}separationData;
};

#endif
