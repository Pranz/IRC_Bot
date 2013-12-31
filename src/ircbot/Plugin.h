#ifndef __LOLIROFLE_IRCBOT_PLUGIN_H_INCLUDED__
#define __LOLIROFLE_IRCBOT_PLUGIN_H_INCLUDED__

#include <lolie/LinkedList.h>
#include <lolie/TypeAliases.h>
#include <lolie/Stringp.h>

LinkedList*/*<struct Plugin*>*/ plugins=LinkedList_init;

struct Plugin{
	void* lib;
	char* error;
	struct{
		void(*print)();
	}functions;
};

struct Plugin* Plugin_load(const char* filename);
bool Plugin_unload(struct Plugin* plugin);

#endif
