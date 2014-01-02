#include "Plugin.h"

#include <stdlib.h>
#include <dlfcn.h>
#include <dirent.h>
#include <string.h>
#include <stdio.h>

LinkedList* plugins=LinkedList_init;

bool Plugin_loadAll(const char* directoryPath){
	DIR* directory;
	struct dirent* dir;

	//Open directory and initialize `directory`. If specified path cannot be opened as a directory: return false
	if(!(directory=opendir(directoryPath)))
		return false;

	//Length of `directoryPath` string
	size_t directoryPathLen=strlen(directoryPath);

	//File path string buffer
	char filePath[directoryPathLen+sizeof(((struct dirent*)0)->d_name)/sizeof(char)+2];

	//Copy `directoryPath` to `filePath` string buffer
	strcpy(filePath,directoryPath);

	//Append ´/´ path separator if `directoryPath` is specified and there isn't already a file separator in `directoryPath`
	if(directoryPathLen>0 && filePath[directoryPathLen-1]!='/')
		filePath[directoryPathLen++]='/';

	//Set pointer to the filename in the file path
	char* filename=filePath+directoryPathLen;

	//For each file in the directory
	while((dir=readdir(directory))){
		//If file doesn't start with "mod_", don't use this file
		if(memcmp("mod_",dir->d_name,4)!=0)
			continue;

		//Copy filename to `filePath` string buffer
		strcpy(filename,dir->d_name);

		//Load plugin by file path
		Plugin_load(filePath);
		printf("Modules: Loaded `%s`\n",filename);
	}

	//Free resources
	closedir(directory);

	return true;
}

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
