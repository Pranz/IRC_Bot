#ifndef __LOLIROFLE_IRCBOT_COMMANDS_H_INCLUDED__
#define __LOLIROFLE_IRCBOT_COMMANDS_H_INCLUDED__

#include <lolie/TypeAliases.h>
#include <lolie/Stringp.h>
#include "Commands.h"

bool initCommands();
void freeCommands();
bool registerCommands(unsigned short count,...);
const struct Command* getCommand(Stringp name);

#endif
