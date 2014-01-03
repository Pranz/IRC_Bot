#include <stdio.h> //Input/output
#include <string.h>

#include <lolie/Stringp.h>
#include <lolie/LinkedList.h>
#include <lolie/Memory.h>
#include <lolie/Math.h>
#include <lolie/ControlStructures.h>

#include <ircinterface/irc.h>
#include "Locale.h"
#include "Commands.h"
#include "Command.h"
#include "IRCBot.h"
#include "Plugin.h"

//TODO: "&&"" to combine commands and maybe `command` to insert a command with output as return value to an argument
//TODO: Help pages for a list of commands and syntax, explanation, etc.
//TODO: Command aliases

/**
 * Buffer initialization
 */
#define IRC_WRITE_BUFFER_LEN 512
char write_buffer[IRC_WRITE_BUFFER_LEN];

struct IRCBot bot;

char command_separator = ' ';
enum Languages language = LANG_SWEDISH;

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

void onCommand(struct IRCBot* bot,Stringcp command,Stringcp target,const char* arg_begin,const char* arg_end,const irc_message* message){
	const struct Command* currentCommand;
	if((currentCommand=getCommand(&bot->commands,command)) && currentCommand->func(bot,target,arg_begin,arg_end))
		return;
	else{
		int len = Stringp_vcopy(STRINGP(write_buffer,IRC_WRITE_BUFFER_LEN),4,
			locale[language].unknown_command,
			STRINGP(": \"",3),
			command,
			STRINGP("\"",1)
		);
		irc_send_message(&bot->connection,target,STRINGCP(write_buffer,len));
	}
}

void onMessageFunc(const irc_connection* connection,const irc_message* message){
	switch(message->command_type){
		case IRC_MESSAGE_COMMAND_NUMBER:
			if(message->command_type_number == 1){
				irc_join_channel(connection,"#bot");
				//irc_join_channel(connection,"#toa");
			}
			break;
		case IRC_MESSAGE_COMMAND_PRIVMSG:{
			void onCommandRaw(Stringcp target,char* read_ptr_begin,char* read_ptr_end){
				//Initialize read pointers
				char* read_ptr = read_ptr_begin;

				//Initialize command Stringp
				while(read_ptr<read_ptr_end && *read_ptr!=command_separator)
					++read_ptr;
				Stringcp command = STRINGCP(read_ptr_begin,read_ptr-read_ptr_begin);
				read_ptr_begin=++read_ptr;

				//Commands
				onCommand(&bot,command,target,MIN(read_ptr_begin,read_ptr_end),read_ptr_end,message);
			}

			//If private message command
			if(message->command.privmsg.target.length == bot.nickname.length && Memory_equals(message->command.privmsg.target.ptr,bot.nickname.ptr,bot.nickname.length))
				onCommandRaw(STRINGP_CONST(message->prefix.user.nickname),message->command.privmsg.text.ptr,message->command.privmsg.text.ptr+message->command.privmsg.text.length);

			//If on a channel with a '#' prefix and the private message has the correct prefix
			else if(message->command.privmsg.target.ptr[0] == '#'){
				if(message->command.privmsg.text.length<=bot.commandPrefix.length || !Memory_equals(message->command.privmsg.text.ptr,bot.commandPrefix.ptr,bot.commandPrefix.length))
					break;

				onCommandRaw(STRINGP_CONST(message->command.privmsg.target),message->command.privmsg.text.ptr+bot.commandPrefix.length,message->command.privmsg.text.ptr+message->command.privmsg.text.length);
			}
		}	break;
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

	//Initialize bot structure
	IRCBot_initialize(&bot);

	//Load all plugins
	if(!Plugin_loadAll(&bot,"modules"))
		fputs("Warning: Failed to initialize modules\n",stderr);

	//Connect to server
	IRCBot_connect(&bot,Stringcp_from_cstr("flygande-toalett.tk"),1568);

	IRCBot_setNickname(&bot,Stringcp_from_cstr("Toabot"));
	IRCBot_setUsername(&bot,Stringcp_from_cstr("Toabot"));
	IRCBot_setCommandPrefixc(&bot,'!');

	//While a message is sent from the server
	while(irc_read(&bot.connection,&onMessageFunc));
	
	//Disconnect connection
	IRCBot_disconnect(&bot);

	//Free resources
	IRCBot_free(&bot);

	return EXIT_SUCCESS;
}
