#ifndef __LOLIROFLE_IRCBOT_COMMANDS_H_INCLUDED__
#define __LOLIROFLE_IRCBOT_COMMANDS_H_INCLUDED__

#include <lolie/TypeAliases.h>
#include <lolie/Stringp.h>
struct Command;
struct DynamicArray;

bool initCommands(struct DynamicArray* commands);
void freeCommands(struct DynamicArray* commands);
bool registerCommand(struct DynamicArray* commands,const struct Command* command);
bool registerCommandsFromArray(struct DynamicArray* commands,const struct Command* cmds,size_t count);
const struct Command* getCommand(struct DynamicArray* commands,Stringcp name);
bool unregisterCommand(struct DynamicArray* commands,const struct Command* command);
bool unregisterCommandByName(struct DynamicArray* commands,Stringcp commandName);

#endif
