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
	DynamicArray_free(commands);
	//TODO: Free all the other resources
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

	if(command->name.length+1>commands->capacity){//If the array needs to be resized for the new command
		DynamicArray_resize(commands,command->name.length+1);

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
	if(!commands)
		return false;

	while(count-->0){
		if(cmds){
			if(cmds->name.length==0)
				return false;

			if(cmds->name.length+1>commands->capacity){//If the array needs to be resized for the new commands
				DynamicArray_resize(commands,cmds->name.length+1);

				Array_forEach(((Array){commands->array+commands->length,commands->capacity-commands->length}),list){
					*(LinkedList**)list=LinkedList_init;//Set all new LinkedLists to the initial value
				}
				commands->length=commands->capacity;
			}
			LinkedList_push((LinkedList**)&DynamicArray__get(*commands,cmds->name.length),cmds);
		}
		++cmds;
	}

	return true;
}

bool unregisterCommand(struct DynamicArray* commands,const struct Command* command){
	if(!command || command->name.length==0)
		return false;

	//For every command length list
	for(size_t i=0;i<commands->capacity;++i){
		printf("%lu Size: %lu\n",i,LinkedList_size(DynamicArray__get(*commands,i)));
		if(!DynamicArray__get(*commands,i))
			continue;

		LinkedList_forEach(DynamicArray__get(*commands,i),node){
			printf("%p == %p\n",node->ptr,command);
		}

		//If command is found and removed
		if(LinkedList_remove((LinkedList**)&DynamicArray__get(*commands,i),command))
			return true;
	}

	return false;
}

bool unregisterCommandByName(struct DynamicArray* commands,Stringcp commandName){
	if(commandName.length>commands->capacity)
		return false;

	return LinkedList_removeFirst((LinkedList**)&DynamicArray__get(*commands,commandName.length),
		(bool(*)(void *))function(bool,(struct Command* command){
			return memcmp(command->name.ptr,commandName.ptr,commandName.length)==0;
		})
	);
}
