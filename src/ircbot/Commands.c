#include "Commands.h"

#include "Command.h"
#include <lolie/LinkedList.h>
#include <string.h>
#include <stdlib.h>

static struct LinkedList/*<struct Command*>*/** commands=NULL;
static size_t commandsLength=0;

bool initCommands(){
	if(!commands){
		commandsLength=20;
		commands=malloc(sizeof(struct Command*)*commandsLength);
		if(commands==NULL){
			commandsLength=0;
			return false;
		}
		memset(commands,'\0',sizeof(struct Command*)*commandsLength);
	}
	return true;
}

inline void freeCommands(){
	free(commands);
}

static bool resizeCommandList(size_t size){
	commands=realloc(commands,sizeof(struct Command*)*size);
	if(commands==NULL){
		commandsLength=0;
		return false;
	}
	memset(commands+commandsLength,'\0',sizeof(struct Command*)*(size-commandsLength));
	commandsLength=size;
	return true;
}

const struct Command* getCommand(Stringp name){
	if(name.length>commandsLength || commands[name.length]==NULL)
		return NULL;

	LinkedList_forEach(commands[name.length],node){
		if(memcmp(((struct Command*)node->ptr)->name.ptr,name.ptr,name.length)==0)
			return (struct Command*)node->ptr;
	}
	return NULL;
}
