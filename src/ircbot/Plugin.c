#include "Plugin.h"

#include <stdlib.h>
#include <dlfcn.h>

struct Plugin* Plugin_load(const char* filename){
	struct Plugin* plugin = malloc(sizeof(struct Plugin));

	if(!(plugin->lib = dlopen(filename,RTLD_LAZY))){
		fprintf(stderr,"%s\n",dlerror());
		goto Error;
	}

	plugin->functions.print = dlsym(plugin->lib,"print");
	if((plugin->error = dlerror())!=NULL){
		fprintf(stderr,"%s\n",plugin->error);
		goto Error;
	}

	LinkedList_push(&plugins,plugin);

	return plugin;

	Error:
	free(plugin);
	return NULL;
}

bool Plugin_unload(struct Plugin* plugin){
	LinkedList_remove(&plugins,plugin);
	dlclose(plugin->lib);
	free(plugin);
	return true;
}
