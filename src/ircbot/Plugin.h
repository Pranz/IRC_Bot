#ifndef __LOLIROFLE_IRCBOT_PLUGIN_H_INCLUDED__
#define __LOLIROFLE_IRCBOT_PLUGIN_H_INCLUDED__

#include <lolie/LinkedList.h>
#include <lolie/TypeAliases.h>
#include <lolie/Stringp.h>

/**
 * The list of loaded plugins
 *
 * Type: LinkedList<struct Plugin*>*
 */
LinkedList*/*<struct Plugin*>*/ plugins=LinkedList_init;

struct Plugin{
	void* lib;
	char* error;
	struct{
		void(*print)();
	}functions;
};

/**
 * Loads a plugin library by filename
 *
 * @param filename Filename of the dynamic library that contains the plugin
 * @return         NULL if error, the plugin structure if successful
 */
struct Plugin* Plugin_load(const char* filename);

/**
 * Unloads a plugin, freeing all the resources allocated
 *
 * @param plugin  The plugin structure to be unloaded
 */
bool Plugin_unload(struct Plugin* plugin);

#endif
