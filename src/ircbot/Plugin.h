#ifndef __LOLIROFLE_IRCBOT_PLUGIN_H_INCLUDED__
#define __LOLIROFLE_IRCBOT_PLUGIN_H_INCLUDED__

#include <lolie/LinkedList.h>
#include <lolie/TypeAliases.h>
#include <lolie/Stringp.h>
struct IRCBot;
union CommandArgument;

struct Plugin{
	void* lib;
	struct{
		bool(*onLoad)      (struct IRCBot* bot);
		bool(*onUnload)    (struct IRCBot* bot);
		bool(*onConnect)   (struct IRCBot* bot);
		bool(*onDisconnect)(struct IRCBot* bot);
		bool(*onCommand)   (struct IRCBot* bot,Stringp command,union CommandArgument* arg);
		bool(*onJoin)      (struct IRCBot* bot,Stringp channel);
		bool(*onPart)      (struct IRCBot* bot,Stringp channel);
		bool(*onUserJoin)  (struct IRCBot* bot,Stringp nickname,Stringp channel);
		bool(*onUserPart)  (struct IRCBot* bot,Stringp nickname,Stringp channel);
	}functions;
	struct{
		const char* version;
		const char* author;
	}constants;
};

/**
 * Loads a plugin library by filename
 *
 * @param filename Filename of the dynamic library that contains the plugin
 * @return         NULL if error, the plugin structure if successful
 */
struct Plugin* Plugin_load(struct IRCBot* bot,const char* filename);

/**
 * Unloads a plugin, freeing all the resources allocated
 *
 * @param plugin  The plugin structure to be unloaded
 */
bool Plugin_unload(struct IRCBot* bot,struct Plugin* plugin);

/**
 * Loads plugin libraries from a directory
 *
 * @param directoryPath Path of the directory where the plugins reside
 * @return              True if successful, false if error
 */
bool Plugin_loadAll(struct IRCBot* bot,const char* directoryPath);

/**
 * Unloads all plugins, freeing all the resources allocated
 */
void Plugin_unloadAll(struct IRCBot* bot);

#endif
