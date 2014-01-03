//gcc -o mod_testcommands.o -c -fPIC mod_testcommands.c
//gcc -shared -Wl,-soname,mod_testcommands.so -o mod_testcommands.so mod_testcommands.o
// OR
//gcc -fPIC -shared -Wl,-soname,mod_testcommands.so -o mod_testcommands.so mod_testcommands.c

#include <stdio.h>
#include <string.h>
#include <ircbot/IRCBot.h>
#include <ircbot/Command.h>
#include <ircbot/Commands.h>
#include <lolie/TypeAliases.h>
#include <lolie/DynamicArray.h>

const char plugin_version[] = "1.0";
const char plugin_author[]  = "Lolirofle";

static struct Command* c=NULL;

bool plugin_onLoad(struct IRCBot* bot){
	if(!(c=malloc(sizeof(struct Command)*5))){
		fputs("Error: Cannot allocate memory for commands",stderr);
		exit(EXIT_FAILURE);
	}

	c[0]=(struct Command){
		Stringcp_from_cstr("test"),
		Stringcp_from_cstr("Test command"),
		function(bool,(struct IRCBot* bot,Stringcp target,const char* arg_begin,const char* arg_end){
			#define TEST_STRING "Test command has been executed"
			irc_send_message(&bot->connection,target,STRINGCP(TEST_STRING,sizeof(TEST_STRING)));
			return true;
		}),
		0
	};
	c[1]=(struct Command){
		Stringcp_from_cstr("version"),
		Stringcp_from_cstr("Outputs version of bot"),
		function(bool,(struct IRCBot* bot,Stringcp target,const char* arg_begin,const char* arg_end){
			irc_send_message(&bot->connection,target,IRCBot_signature);
			return true;
		}),
		0
	};
	c[2]=(struct Command){
		Stringcp_from_cstr("bot"),
		Stringcp_from_cstr("Call me"),
		function(bool,(struct IRCBot* bot,Stringcp target,const char* arg_begin,const char* arg_end){
			irc_send_message(&bot->connection,target,STRINGCP("IT'S A ME, TOABOTU",18));
			return true;
		}),
		0
	};
	c[3]=(struct Command){
		Stringcp_from_cstr("1234567890abcdef"),
		Stringcp_from_cstr("Langt kommando"),
		function(bool,(struct IRCBot* bot,Stringcp target,const char* arg_begin,const char* arg_end){
			puts("Ja: aa");
			return true;
		}),
		0
	};
	c[4]=(struct Command){
		Stringcp_from_cstr("help"),
		Stringcp_from_cstr("Outputs a list of commands and their usage"),
		function(bool,(struct IRCBot* bot,Stringcp target,const char* arg_begin,const char* arg_end){
			char str[512];
			size_t strLength=0;

			memcpy(str,"Commands: ",10);
			strLength+=10;

			DynamicArray_forEach(bot->commands,commandLengthList){
				LinkedList_forEach(*(LinkedList**)commandLengthList,node){
					memcpy(str+strLength,((struct Command*)node->ptr)->name.ptr,((struct Command*)node->ptr)->name.length);
					strLength+=((struct Command*)node->ptr)->name.length;
					memcpy(str+strLength,", ",2);
					strLength+=2;
				}
			}

			irc_send_message(&bot->connection,target,STRINGCP(str,strLength));
			return true;
		}),
		0
	};
	return registerCommandsFromArray(&bot->commands,c,5);
}

bool plugin_onUnload(struct IRCBot* bot){
	if(c){
		free(c);
		c=NULL;
	}
	return true;
}
