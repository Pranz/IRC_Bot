#include "Commands.h"

#include "Command.h"
#include <lolie/LinkedList.h>
#include <lolie/DynamicArray.h>
#include <lolie/Array.h>
#include <string.h>
#include <stdlib.h>

/* ///////////////////////////////////////////////////
 * Type of commands list:
 *   struct DynamicArray<LinkedList<struct Command*>*>
 */

bool initCommands(struct DynamicArray* commands){
	*commands=DynamicArray_init;

	//Use length for checking if capacity changes when using registerCommand()
	commands->length=commands->capacity;
	DynamicArray_forEach(*commands,list){//Set all LinkedLists to the initial value
		*list=LinkedList_init;
	}

	return commands->array!=NULL;
}

void freeCommands(struct DynamicArray* commands){
	commands->length=0;//TODO: Free all the other resources
}

const struct Command* getCommand(struct DynamicArray* commands,Stringcp name){
	if(name.length>commands->capacity || DynamicArray__get(*commands,name.length)==NULL)
		return NULL;

	LinkedList_forEach((LinkedList*)DynamicArray__get(*commands,name.length),node){
//printf("Search command: %s\n",((struct Command*)node->ptr)->name.ptr);
		if(memcmp(((struct Command*)node->ptr)->name.ptr,name.ptr,name.length)==0)
			return (struct Command*)node->ptr;
	}
	return NULL;
}

bool registerCommand(struct DynamicArray* commands,const struct Command* command){
	if(!command || command->name.length==0)
		return false;

	if(command->name.length>commands->capacity){//If the array needs to be resized for the new command
		DynamicArray_resize(commands,command->name.length);

		Array_forEach(((Array){commands->array+commands->length,commands->capacity-commands->length}),list){
			*(LinkedList**)list=LinkedList_init;//Set all new LinkedLists to the initial value
		}
		commands->length=commands->capacity;
	}
	LinkedList_push((LinkedList**)&DynamicArray__get(*commands,command->name.length),command);
//printf("Size: %u, {%p,%p} -> %p\n",LinkedList_size(DynamicArray__get(*commands,command->name.length)),((LinkedList*)DynamicArray__get(*commands,command->name.length))->ptr,DynamicArray__get(*commands,command->name.length),((LinkedList*)DynamicArray__get(*commands,command->name.length))->next);

	return true;
}

bool registerCommandsFromArray(struct DynamicArray* commands,const struct Command* cmds,size_t count){
	if(!cmds)
		return false;

	while(count-->0){
		if(cmds->name.length==0)
			return false;

		if(cmds->name.length>commands->capacity){//If the array needs to be resized for the new commands
			DynamicArray_resize(commands,cmds->name.length);

			Array_forEach(((Array){commands->array+commands->length,commands->capacity-commands->length}),list){
				*(LinkedList**)list=LinkedList_init;//Set all new LinkedLists to the initial value
			}
			commands->length=commands->capacity;
		}
		LinkedList_push((LinkedList**)&DynamicArray__get(*commands,cmds->name.length),cmds);

		++cmds;
	}

	return true;
}
