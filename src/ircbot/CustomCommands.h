#ifndef __LOLIROFLE_IRCBOT_CUSTOMCOMMANDS_H_INCLUDED__
#define __LOLIROFLE_IRCBOT_CUSTOMCOMMANDS_H_INCLUDED__

#include <lolie/TypeAliases.h>
struct DynamicArray;

bool initCustomCommands(struct DynamicArray* commands);
bool freeCustomCommands(struct DynamicArray* commands);

#endif
