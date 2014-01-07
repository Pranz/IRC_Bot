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
#include <lolie/String.h>
#include <lolie/Stringp.h>

const char plugin_version[] = "1.0";
const char plugin_author[]  = "Lolirofle";

static struct Command* c=NULL;

bool plugin_onLoad(struct IRCBot* bot){
	if(!(c=malloc(sizeof(struct Command)*11))){
		fputs("Error: Cannot allocate memory for commands",stderr);
		return false;
	}

	c[0]=(struct Command){
		Stringcp_from_cstr("test"),
		Stringcp_from_cstr("Test command"),
		function(bool,(struct IRCBot* bot,Stringcp target,union CommandArgument* arg){
			#define TEST_STRING "Test command has been executed"
			irc_send_message(&bot->connection,target,STRINGCP(TEST_STRING,sizeof(TEST_STRING)));
			return true;
		}),
		COMMAND_PARAMETER_TYPE_NONE
	};
	c[1]=(struct Command){
		Stringcp_from_cstr("version"),
		Stringcp_from_cstr("Outputs version of bot"),
		function(bool,(struct IRCBot* bot,Stringcp target,union CommandArgument* arg){
			irc_send_message(&bot->connection,target,IRCBot_signature);
			return true;
		}),
		COMMAND_PARAMETER_TYPE_NONE
	};
	c[2]=(struct Command){
		Stringcp_from_cstr("bot"),
		Stringcp_from_cstr("Call me"),
		function(bool,(struct IRCBot* bot,Stringcp target,union CommandArgument* arg){
			irc_send_message(&bot->connection,target,STRINGCP("IT'S A ME, TOABOTU",18));
			return true;
		}),
		COMMAND_PARAMETER_TYPE_NONE
	};
	c[3]=(struct Command){
		Stringcp_from_cstr("echo"),
		Stringcp_from_cstr("Echoes the arguments"),
		function(bool,(struct IRCBot* bot,Stringcp target,union CommandArgument* arg){
			irc_send_message(&bot->connection,target,STRINGCP(arg->free.begin,arg->free.end-arg->free.begin));
			return true;
		}),
		COMMAND_PARAMETER_TYPE_NONE
	};
	c[4]=(struct Command){
		Stringcp_from_cstr("help"),
		Stringcp_from_cstr("Outputs a list of commands and their usage"),
		function(bool,(struct IRCBot* bot,Stringcp target,union CommandArgument* arg){
			char str[512];
			size_t strLength=0;

			memcpy(str,"Commands: ",10);
			strLength+=10;

			DynamicArray_forEach(bot->commands,commandLengthList){
				LinkedList_forEach(*(LinkedList**)commandLengthList,node){//TODO: Could overflow
					memcpy(str+strLength,((struct Command*)node->ptr)->name.ptr,((struct Command*)node->ptr)->name.length);
					strLength+=((struct Command*)node->ptr)->name.length;
					memcpy(str+strLength,", ",2);
					strLength+=2;
				}
			}

			irc_send_message(&bot->connection,target,STRINGCP(str,strLength));
			return true;
		}),
		COMMAND_PARAMETER_TYPE_NONE
	};
	c[5]=(struct Command){
		Stringcp_from_cstr("shutdown"),
		Stringcp_from_cstr("Shutdown the bot"),
		function(bool,(struct IRCBot* bot,Stringcp target,union CommandArgument* arg){
			IRCBot_shutdown(bot);
			irc_send_message(&bot->connection,target,STRINGCP("Shutting down...",16));
			return true;
		}),
		COMMAND_PARAMETER_TYPE_NONE
	};
	c[6]=(struct Command){
		Stringcp_from_cstr("restart"),
		Stringcp_from_cstr("Restart the bot"),
		function(bool,(struct IRCBot* bot,Stringcp target,union CommandArgument* arg){
			IRCBot_restart(bot);
			irc_send_message(&bot->connection,target,STRINGCP("Restarting...",13));
			return true;
		}),
		COMMAND_PARAMETER_TYPE_NONE
	};
	c[7]=(struct Command){
		Stringcp_from_cstr("reload"),
		Stringcp_from_cstr("Reload the bot"),
		function(bool,(struct IRCBot* bot,Stringcp target,union CommandArgument* arg){
			IRCBot_reloadPlugins(bot);
			irc_send_message(&bot->connection,target,STRINGCP("Modules is reloaded",19));
			return true;
		}),
		COMMAND_PARAMETER_TYPE_NONE
	};
	c[8]=(struct Command){
		Stringcp_from_cstr("join"),
		Stringcp_from_cstr("Makes the bot join specified channel"),
		function(bool,(struct IRCBot* bot,Stringcp target,union CommandArgument* arg){
			IRCBot_joinChannel(bot,STRINGCP(arg->free.begin,arg->free.end-arg->free.begin));
			return true;
		}),
		COMMAND_PARAMETER_TYPE_NONE
	};
	c[9]=(struct Command){
		Stringcp_from_cstr("part"),
		Stringcp_from_cstr("Makes the bot part from specified channel"),
		function(bool,(struct IRCBot* bot,Stringcp target,union CommandArgument* arg){
			IRCBot_partChannel(bot,STRINGCP(arg->free.begin,arg->free.end-arg->free.begin));
			return true;
		}),
		COMMAND_PARAMETER_TYPE_NONE
	};
	c[10]=(struct Command){
		Stringcp_from_cstr("channels"),
		Stringcp_from_cstr("List the channels the bot currently resides in"),
		function(bool,(struct IRCBot* bot,Stringcp target,union CommandArgument* arg){//TODO: List the channels instead of echoing a message for each channel
			LinkedList_forEach(bot->channels,node){
				irc_send_message(&bot->connection,target,Stringcp_from_string((String*)node->ptr));
			}
			return true;
		}),
		COMMAND_PARAMETER_TYPE_NONE
	};
	return registerCommandsFromArray(&bot->commands,c,11);
}

bool plugin_onUnload(struct IRCBot* bot){
	for(uint i=0;i<11;++i)
		if(!unregisterCommandByName(&bot->commands,c[i].name))
			fprintf(stderr,"Module: mod_base: Warning: Command couldn't be freed: %s\n",c[i].name.ptr);

	if(c){
		free(c);
		c=NULL;
	}
	return true;
}
