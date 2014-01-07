//gcc -o mod_test.o -c -fPIC mod_test.c
//gcc -shared -Wl,-soname,mod_test.so -o mod_test.so mod_test.o
// OR
//gcc -fPIC -shared -Wl,-soname,mod_test.so -o mod_test.so mod_test.c

#include <stdio.h>
#include <ircbot/api/IRCBot.h>

const char plugin_version[] = "1.0";
const char plugin_author[]  = "Lolirofle";

bool plugin_onLoad(struct IRCBot* bot){
	puts("Test module is loaded with custom output");
	return true;
}

bool plugin_onUnload(struct IRCBot* bot){
	puts("Test module is unloaded with custom output");
	return true;
}
