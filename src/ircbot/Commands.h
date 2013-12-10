#ifndef __LOLIROFLE_IRCBOT_COMMANDS_H_INCLUDED__
#define __LOLIROFLE_IRCBOT_COMMANDS_H_INCLUDED__

#include <lolie/TypeAliases.h>
#include <lolie/Stringp.h>
#include "Commands.h"

bool initCommands();
bool freeCommands();
bool registerCommand(struct Command* command);
bool unregisterCommand(Stringp name);
struct Command* getCommand(Stringp name);

#endif
