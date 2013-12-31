#ifndef __LOLIROFLE_IRCBOT_COMMANDS_H_INCLUDED__
#define __LOLIROFLE_IRCBOT_COMMANDS_H_INCLUDED__

#include <lolie/TypeAliases.h>
#include <lolie/Stringp.h>
struct Command;

bool initCommands();
void freeCommands();
bool registerCommand(const struct Command* command);
bool registerCommandsFromArray(const struct Command* cmds,size_t count);
const struct Command* getCommand(Stringcp name);

#endif
