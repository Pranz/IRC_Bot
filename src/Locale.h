#ifndef __LOLIROFLE_IRC_LOCALE_H_INCLUDED__
#define __LOLIROFLE_IRC_LOCALE_H_INCLUDED__

#include <lolie/Stringp.h>

struct Locale{
	Stringp lang_name;

	Stringp boolean[2];

	struct{
		Stringp set;
		Stringp unknown;
	}language;

	struct{
		Stringp ask;
		Stringp yes[10];
		Stringp maybe[5];
		Stringp no[5];
		Stringp failure;
	}magic8ball;

	struct{
		Stringp set;
		Stringp get;
	}prefix;

	Stringp unknown_command;
};

enum Languages{
	LANG_SWEDISH,
	LANG_ENGLISH,
	
	LANG_COUNT
};

extern const struct Locale locale[2];

#endif
