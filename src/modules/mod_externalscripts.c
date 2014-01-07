#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <dlfcn.h>
#include <dirent.h>
#include <ircbot/IRCBot.h>
#include <ircbot/Command.h>
#include <ircbot/Commands.h>
#include <lolie/TypeAliases.h>
#include <lolie/Stringp.h>
#include <ircbot/pipes.h>

const char plugin_version[] ="1.1";
const char plugin_author[]  ="Lolirofle";

#define BUFFER_LENGTH 512
static char buffer[BUFFER_LENGTH];
#define SCRIPT_PATH "external_commands/"
#define SCRIPT_PATH_LEN 18

bool plugin_onCommand(struct IRCBot* bot,Stringcp target,Stringcp command,union CommandArgument* arg){
	//Copy path and command string to a buffer
	char cmd[SCRIPT_PATH_LEN+command.length+1];
	memcpy(cmd,SCRIPT_PATH,SCRIPT_PATH_LEN);
	char* cmd_iterator=cmd+SCRIPT_PATH_LEN;
	for(Stringcp commandStr=command;commandStr.length>0;++commandStr.ptr,--commandStr.length){
		if(!isalnum(*commandStr.ptr))
			return true;
		*cmd_iterator++=*commandStr.ptr;
	}
	*cmd_iterator='\0';

	//Open program
	char* argv[]={cmd+SCRIPT_PATH_LEN,NULL};
	struct PipedStream stream=p2open(cmd,argv);
	if(!stream.in || !stream.out)
		return true;

	//Input to program,arguments
	fwrite(arg->free.begin,sizeof(char),arg->free.end-arg->free.begin,stream.in);
	p2flushWrite(stream);

	//Output from program,output
	size_t len=fread(buffer,sizeof(char),BUFFER_LENGTH,stream.out);
	p2flushRead(stream);
	if(len>1)
		irc_send_message(&bot->connection,target,STRINGCP(buffer,len));

	if(p2close(stream)!=0)
		return true;

	return false;
}
