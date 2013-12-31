#include "Plugin.h"

#include <stdlib.h>
#include <dlfcn.h>

struct Plugin* Plugin_load(const char* filename){
	//Allocate memory to structure
	struct Plugin* plugin = malloc(sizeof(struct Plugin));

	//Open file, initialising plugin
	if(!(plugin->lib = dlopen(filename,RTLD_LAZY))){
		fprintf(stderr,"%s\n",dlerror());
		goto Error;
	}

	//Load the required functions of plugin
	plugin->functions.print = dlsym(plugin->lib,"print");
	if((plugin->error = dlerror())!=NULL){
		fprintf(stderr,"%s\n",plugin->error);
		goto Error;
	}

	//Push the plugin into the plugin list
	LinkedList_push(&plugins,plugin);

	return plugin;

	//In case of error, label for gotos
	Error:
	free(plugin);
	return NULL;
}

bool Plugin_unload(struct Plugin* plugin){
	//Remove from plugin list
	LinkedList_remove(&plugins,plugin);

	//Close dynamic library
	dlclose(plugin->lib);

	//Free structure
	free(plugin);
	
	return true;
}
