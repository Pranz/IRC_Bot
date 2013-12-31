#include "IRCBot.h"

#include <stdlib.h>
#include <string.h>
#include "ircinterface/irc.h"
#include "Commands.h"
#include <lolie/Stringp.h>

/**
 * Allocates a string wrapped in a Stringp, copying the string from `src` and null-terminates it.
 * The length of the new Stringp will not include the null-termination character.
 *
 * @param src The string we copies from
 * @return    The copied string
 */
static Stringp Stringp_from_malloc_copy_nt(Stringcp src){
	Stringp out;

	//Mallocate string
	out=(Stringp){malloc(src.length+1),src.length};
	//malloc error check
	if(!out.ptr)
		return Stringp_init;
	//Copy string
	memcpy(out.ptr,src.ptr,src.length);
	//Set null termination for compatibility
	out.ptr[src.length]='\0';

	return out;
}

/**
 * Frees a allocated string wrapped in Stringp, setting the pointer to NULL and length to zero.
 *
 * @param str Stringp to free
 */
static void Stringp_free_malloc(Stringp* str){
	if(str && str->ptr){
		free(str->ptr);
		str->ptr=NULL;
		str->length=0;
	}
}

bool IRCBot_initialize(struct IRCBot* bot){
	if(!bot)
		return false;

	bot->hostname=Stringp_init;
	bot->nickname=Stringp_init;
	bot->username=Stringp_init;
	bot->realname=Stringp_init;
	bot->connected=false;

	bot->error=IRCBot_Error_init;
	
	bot->commandPrefix=Stringp_init;
	initCommands(&bot->commands);

	return true;
}

bool IRCBot_free(struct IRCBot* bot){
	if(!bot)
		return false;

	//Disconnect connection if connected
	IRCBot_disconnect(bot);

	//Free mallocated strings
	Stringp_free_malloc(&bot->hostname);
	Stringp_free_malloc(&bot->nickname);
	Stringp_free_malloc(&bot->username);
	Stringp_free_malloc(&bot->realname);
	Stringp_free_malloc(&bot->error.message);
	Stringp_free_malloc(&bot->commandPrefix);
	freeCommands(&bot->commands);

	//Free the mallocated structure
	free(bot);

	return true;
}

bool IRCBot_connect(struct IRCBot* bot,Stringcp host,unsigned short port){
	if(!bot || bot->connected)
		return false;

	//Mallocate and copy string
	bot->hostname=Stringp_from_malloc_copy_nt(host);

	//malloc error check
	if(!bot->hostname.ptr){
		bot->error.code=IRCBOT_ERROR_MEMORY;
		return false;
	}

	//Connect using arguments
	bot->connection=irc_connect(bot->hostname.ptr,port);

	//Error checking
	if(bot->connection.id<0){
		bot->error.code=IRCBOT_ERROR_CONNECT;
		bot->error.message.length=46*sizeof(char)+8;
		char* str=malloc(bot->error.message.length);
		if(!str){
			bot->error.code=IRCBOT_ERROR_MEMORY;
			return false;
		}
		bot->error.message.length=snprintf(str,bot->error.message.length,"Error: IRC connection id is a negative value: %i",bot->connection.id);
		bot->error.message.ptr=str;
		return false;
	}

	//Successful connection
	bot->connected=true;

	return true;
}

bool IRCBot_disconnect(struct IRCBot* bot){
	if(!bot || !bot->connected)
		return false;

	//Disconnect connection
	bot->connected=false;
	if(!irc_disconnect(&bot->connection)){
		static const Stringcp disconnectError={"IRC disconnect failed",21};

		bot->error.code=IRCBOT_ERROR_DISCONNECT;
		bot->error.message.length=disconnectError.length;
		memcpy(bot->error.message.ptr,disconnectError.ptr,disconnectError.length);
		return false;
	}

	return true;
}

void IRCBot_setNickname(struct IRCBot* bot,Stringcp name){//TODO: Some kind of error when the nickname is occupied. Doesn't have to be in the setNickname function.
	//Free existing string if any
	if(!Stringp_isEmpty(bot->nickname))
		Stringp_free_malloc(&bot->nickname);

	//Mallocate and copy string
	bot->nickname=Stringp_from_malloc_copy_nt(name);

	//malloc error check
	if(!bot->nickname.ptr){
		bot->error.code=IRCBOT_ERROR_MEMORY;
		return;
	}

	//Send to server
	irc_set_nickname(&bot->connection,bot->nickname.ptr);
}

void IRCBot_setUsername(struct IRCBot* bot,Stringcp name){
	//Free existing string if any
	if(!Stringp_isEmpty(bot->username))
		Stringp_free_malloc(&bot->username);

	//Mallocate and copy string
	bot->username=Stringp_from_malloc_copy_nt(name);

	//malloc error check
	if(!bot->username.ptr){
		bot->error.code=IRCBOT_ERROR_MEMORY;
		return;
	}

	//Send to server
	irc_set_username(&bot->connection,bot->username.ptr,bot->realname.ptr);
}

void IRCBot_setRealname(struct IRCBot* bot,Stringcp name){
	//Free existing string if any
	if(!Stringp_isEmpty(bot->realname))
		Stringp_free_malloc(&bot->realname);

	//Mallocate and copy string
	bot->realname=Stringp_from_malloc_copy_nt(name);

	//malloc error check
	if(!bot->realname.ptr){
		bot->error.code=IRCBOT_ERROR_MEMORY;
		return;
	}

	//Send to server
	irc_set_username(&bot->connection,bot->username.ptr,bot->realname.ptr);
}

void IRCBot_setCommandPrefix(struct IRCBot* bot,Stringcp prefix){
	//Free existing string if any
	if(!Stringp_isEmpty(bot->commandPrefix))
		Stringp_free_malloc(&bot->commandPrefix);

	//Mallocate and copy string
	bot->commandPrefix=Stringp_from_malloc_copy_nt(prefix);

	//malloc error check
	if(!bot->commandPrefix.ptr){
		bot->error.code=IRCBOT_ERROR_MEMORY;
		return;
	}
}

void IRCBot_setCommandPrefixc(struct IRCBot* bot,char prefix){
	//Free existing string if any
	if(!Stringp_isEmpty(bot->commandPrefix))
		Stringp_free_malloc(&bot->commandPrefix);

	//Mallocate
	bot->commandPrefix.ptr=malloc(2);

	//malloc error check
	if(!bot->commandPrefix.ptr){
		bot->error.code=IRCBOT_ERROR_MEMORY;
		return;
	}

	//Copy string
	bot->commandPrefix.ptr[0]=prefix;
	bot->commandPrefix.ptr[1]='\0';
	bot->commandPrefix.length=1;
}
