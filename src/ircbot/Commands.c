#include "Commands.h"

static struct Command** commands=NULL;
static size_t commandsLength=0;

bool initCommands(){
	if(!commands){
		commandsLength=20;
		commands=malloc(commandsLength);
	}
	return commands!=NULL;
}

inline void freeCommands(){
	free(commands);
}

static bool resizeCommandList(size_t size){
	commandsLength=size;
	commands=realloc(commands,commandsLength);
	return commands!=NULL;
}
