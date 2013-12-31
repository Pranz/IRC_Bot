#include "CustomCommands.h"

#include "Command.h"
#include "Commands.h"
#include <stdio.h>
#include <stdlib.h>

static struct Command* c=NULL;

bool initCustomCommands(struct DynamicArray* commands){
	c=malloc(sizeof(struct Command)*4);
	if(!c){
		fputs("Error: Cannot allocate memory for commands",stderr);
		exit(EXIT_FAILURE);
	}

	c[0]=(struct Command){
		Stringcp_from_cstr("test"),
		Stringcp_from_cstr("Test command"),
		function(bool,(const irc_connection* connection,const char* arg_begin,const char* arg_end){
			puts("okkkk");
			return true;
		}),
		0
	};
	c[1]=(struct Command){
		Stringcp_from_cstr("kant"),
		Stringcp_from_cstr("Kant kommando"),
		function(bool,(const irc_connection* connection,const char* arg_begin,const char* arg_end){
			puts("Min kant");
			return true;
		}),
		0
	};
	c[2]=(struct Command){
		Stringcp_from_cstr("a"),
		Stringcp_from_cstr("Test command"),
		function(bool,(const irc_connection* connection,const char* arg_begin,const char* arg_end){
			puts("Ja: a");
			return true;
		}),
		0
	};
	c[3]=(struct Command){
		Stringcp_from_cstr("1234567890abcdef"),
		Stringcp_from_cstr("Langt kommando"),
		function(bool,(const irc_connection* connection,const char* arg_begin,const char* arg_end){
			puts("Ja: aa");
			return true;
		}),
		0
	};
	return registerCommandsFromArray(commands,c,4);
}

bool freeCustomCommands(struct DynamicArray* commands){
	if(c){
		free(c);
		c=NULL;
		return true;
	}
	return false;
}
