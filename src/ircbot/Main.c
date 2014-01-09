#include <stdio.h> //Input/output
#include <string.h>

#include <lolie/Stringp.h>
#include <lolie/LinkedList.h>
#include <lolie/Memory.h>
#include <lolie/Math.h>
#include <lolie/ControlStructures.h>

#include <ircinterface/irc.h>
#include <ircinterface/irc_messagenumbers.h>
#include "Commands.h"
#include "IRCBot.h"
#include "api/Plugin.h"

//TODO: "&&"" to combine commands and maybe `command` to insert a command with output as return value to an argument
//TODO: Help pages for a list of commands and syntax, explanation, etc.
//TODO: Command aliases

struct IRCBot bot;

Stringp string_splitted(Stringp str,size_t(*delimiterFunc)(Stringp str),bool(*onSplitFunc)(const char* begin,const char* end)){
	const char* arg_begin=str.ptr;

	loop{
		if(str.length<1){
			onSplitFunc(arg_begin,str.ptr);
			break;
		}else{
			size_t delim = delimiterFunc(str);
			if(delim>0){
				str.length-=delim;
				if(!onSplitFunc(arg_begin,str.ptr))
					return STRINGP(str.ptr+delim,str.length);
				str.ptr+=delim;
				arg_begin=str.ptr;
				continue;
			}
		}

		++str.ptr;
		--str.length;
	}

	return str;
}

Stringp string_splitted_delim(Stringp str,Stringp delimiter,bool(*onSplitFunc)(const char* begin,const char* end)){
	return string_splitted(str,function(size_t,(Stringp str){
		if(str.length>=delimiter.length && Memory_equals(str.ptr,delimiter.ptr,delimiter.length))
			return delimiter.length;
		else
			return 0;
	}),onSplitFunc);
}

Stringp Stringp_find_substr(Stringp str,bool(*findFunc)(Stringp str));

void onMessageFunc(const irc_connection* connection,const irc_message* message){
	//Check with plugin hooks
	LinkedList_forEach(bot.pluginHooks.onMessage,node){
		if(!((typeof(((struct Plugin*)0)->functions.onMessage))(node->ptr))(&bot,message));
			return;
	}

	switch(message->command_type){
		case IRC_MESSAGE_TYPE_NUMBER:
			switch(message->command_type_number){
				case 1:
					IRCBot_joinChannel(&bot,STRINGCP("#bot",4));
					//IRCBot_joinChannel(&bot,STRINGCP("#toa",4));
					break;
				case IRC_MESSAGE_TYPENO_ERR_NONICKNAMEGIVEN:
				case IRC_MESSAGE_TYPENO_ERR_ERRONEUSNICKNAME:
				case IRC_MESSAGE_TYPENO_ERR_NICKNAMEINUSE:
				case IRC_MESSAGE_TYPENO_ERR_NICKCOLLISION:
					//In case of infinite loop
					if(bot.nickname.length>32)
						break;

					//Reallocate and copy string
					bot.nickname.ptr=realloc(bot.nickname.ptr,++bot.nickname.length);
					bot.nickname.ptr[bot.nickname.length-1]='_';

					//realloc error check
					if(!bot.nickname.ptr){
						bot.error.code=IRCBOT_ERROR_MEMORY;
						return;
					}

					//Send to server
					irc_set_nickname(bot.connection,bot.nickname.ptr);
					break;

				default:
					break;
			}

			break;
		case IRC_MESSAGE_TYPE_PRIVMSG:
			//If on a channel with a '#' prefix and the private message has the correct prefix
			if(message->command.privmsg.target.ptr[0] == '#'){
				if(message->command.privmsg.text.length==bot.commandPrefix.length || memcmp(message->command.privmsg.text.ptr,bot.commandPrefix.ptr,bot.commandPrefix.length)==0)
					IRCBot_performCommand(
						&bot,
						//Target is to the channel that the command was requested in
						STRINGP_CONST(message->command.privmsg.target),
						//Command begins after the command prefix
						message->command.privmsg.text.ptr + bot.commandPrefix.length,
						//Command ends at the same position (end of the message)
						message->command.privmsg.text.ptr + message->command.privmsg.text.length
					);
			}

			//If private message
			else if(message->command.privmsg.target.length == bot.nickname.length && memcmp(message->command.privmsg.target.ptr,bot.nickname.ptr,bot.nickname.length)==0)
				IRCBot_performCommand(
					&bot,
					//Target is the nickname that sent the command
					STRINGP_CONST(message->prefix.user.nickname),
					//Command begins at the beginning of the message
					message->command.privmsg.text.ptr,
					//Command ends at the end of the message
					message->command.privmsg.text.ptr + message->command.privmsg.text.length
				);
			break;
	}
}

int main(){
	//Top border
	for(size_t i=IRCBot_signature.length;i>0;--i)
		putchar('=');
	putchar('\n');

	//Text
	printf("%s\nCopyright (C) 2014, Lolirofle\n",IRCBot_signature.ptr);

	//Bottom border
	for(size_t i=IRCBot_signature.length;i>0;--i)
		putchar('=');
	putchar('\n');
	putchar('\n');

	int botExit;
	Bot:{
		//Initialize bot structure
		IRCBot_initialize(&bot);

		//Load all plugins
		if(!Plugin_loadAll(&bot,"modules"))
			fputs("Warning: Failed to initialize modules\n",stderr);

		//Connect to server
		IRCBot_connect(&bot,Stringcp_from_cstr("server"),1568);

		Stringcp name=Stringcp_from_cstr("Toabot");
		IRCBot_setNickname(&bot,name);
		IRCBot_setUsername(&bot,name);
		IRCBot_setCommandPrefixc(&bot,'!');

		//While a message is sent from the server
		ReadLoop: while(bot.exit==IRCBOT_EXIT_FALSE && irc_read_message(bot.connection,&onMessageFunc));

		if(botExit==IRCBOT_EXIT_RELOADPLUGINS){
			bot.exit=IRCBOT_EXIT_FALSE;
			Plugin_unloadAll(&bot);
			if(!Plugin_loadAll(&bot,"modules"))
				fputs("Warning: Failed to initialize modules\n",stderr);
			goto ReadLoop;
		}

		//Disconnect connection
		IRCBot_disconnect(&bot);

		//Unload all plugins
		Plugin_unloadAll(&bot);

		botExit=bot.exit;

		//Free resources
		IRCBot_free(&bot);

		if(botExit==IRCBOT_EXIT_RESTART)
			goto Bot;
	}

	return EXIT_SUCCESS;
}
