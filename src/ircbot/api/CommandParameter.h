#ifndef __LOLIROFLE_IRCBOT_API_COMMANDPARAMETER_H_INCLUDED__
#define __LOLIROFLE_IRCBOT_API_COMMANDPARAMETER_H_INCLUDED__

#include <lolie/Stringp.h>
#include <lolie/TypeAliases.h>

/**
 * Represents how an argument is splitted/separated from the next
 */
enum CommandParameterSeparation{
	//No separation. It means that it is the last argument
	COMMAND_PARAMETER_SEPARATION_NONE,

	//Default separation, usually with the blankspace character, but it can vary depending on various user settings
	COMMAND_PARAMETER_SEPARATION_DEFAULT,

	//Separate by a character
	COMMAND_PARAMETER_SEPARATION_CHARACTER,

	//Separate by a one of the list's character
	COMMAND_PARAMETER_SEPARATION_CHARACTERLIST,

	//Separate using a custom parser
	COMMAND_PARAMETER_SEPARATION_CUSTOMPARSER
};

/**
 * Represents different types for the argument values.
 */
enum CommandParameterType{
	//Integer ([0-9]+)
	COMMAND_PARAMETER_TYPE_INTEGER,

	//String ([:print:])
	COMMAND_PARAMETER_TYPE_STRING,

	//Floating point number ([0-9]+(\.[0-9]+)?)
	COMMAND_PARAMETER_TYPE_FLOATINGPOINT,

	//Character (.)
	COMMAND_PARAMETER_TYPE_CHARACTER,

	//Boolean (true|on|1|yes | false|off|0|no)
	COMMAND_PARAMETER_TYPE_BOOLEAN,

	//Different values ([:graph:])
	COMMAND_PARAMETER_TYPE_VALUES,

	//Custom parser for parameter (.+)
	COMMAND_PARAMETER_TYPE_CUSTOMPARSER,

	//Free parsing. Passes all arguments as one string (.+)
	COMMAND_PARAMETER_TYPE_FREE
};

/**
 * Represents the type of requirement this parameter has
 */
enum CommandParameterRequirement{
	//The parameter is required
	COMMAND_PARAMETER_REQUIRED,

	//The parameter is optional. This option must be after all the required ones.
	COMMAND_PARAMETER_OPTIONAL,

	//The parameter is a variable length argument. It means that it is separated and can handle the rest of the arguments. This option must be the last.
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
			const char** list;
		}values;

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
