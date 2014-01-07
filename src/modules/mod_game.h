#ifndef __LOLIROFLE_IRCBOT_MODULE_GAME_H_INCLUDED__
#define __LOLIROFLE_IRCBOT_MODULE_GAME_H_INCLUDED__

#include <lolie/Stringp.h>

struct ToaModule_Game{
	Stringcp name;
	Stringcp help;
	void*(*onStart)();
	void*(*onStop)();
	void*(*onMessage)(const struct ToaModule_Game_ListenTarget* target,Stringcp nickname);
};

struct ToaModule_Game_ListenTarget{
	enum{
		TOAMODULE_GAME_LISTENTARGET_BOT,
		TOAMODULE_GAME_LISTENTARGET_CHANNEL
	}type;
	union{
		Stringcp channel;
	};
};

struct ToaModule_GameSession{
	const struct ToaModule_ChannelGame* game;
	void* data;
	LinkedList/*<ToaModule_Game_ListenTarget*>*/* listenTargets;
	LinkedList/*<Stringcp>*/* players;
};

const struct ToaModule_Game* ToaModule_Game_getByName(Stringcp name);

struct ToaModule_GameSession* ToaModule_GameSession_start(struct ToaModule_Game* game);
bool ToaModule_GameSession_stop(struct ToaModule_GameSession* game);

#endif
