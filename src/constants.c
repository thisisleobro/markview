#include <SDL3/SDL_log.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <markview/constants.h>
#include <stdlib.h>

#define MARKVIEW_CONFIGURATION_FILENAME "configuration.json"

char* markview_configuration_folder_path() {
	char* appdata = getenv("APPDATA");

	if (NULL == appdata) {
		SDL_Log("APPDATA is not set");
		return NULL;
	}

	size_t pathLenght = strlen(appdata) + 1 + strlen(MARKVIEW_PROGRAM_NAME) + 1;
	char* configurationFilePath = malloc(pathLenght);

	if (NULL == configurationFilePath) {
		SDL_Log("malloc error configurationFilePath is NULL");
		return NULL;
	}

	snprintf(configurationFilePath, pathLenght, "%s\\%s\\", appdata, MARKVIEW_PROGRAM_NAME);
	
	return configurationFilePath;	
}

char* markview_configuration_file_path() {
	char* configurationFolder = markview_configuration_folder_path();

	if (NULL == configurationFolder) {
		SDL_Log("Erro getting configuration folder");
	}

	size_t pathLenght = strlen(configurationFolder) + 1 + strlen(MARKVIEW_CONFIGURATION_FILENAME) + 1;
	char* configurationFilePath = malloc(pathLenght);

	if (NULL == configurationFilePath) {
		SDL_Log("malloc error configurationFilePath is NULL");
		return NULL;
	}

	snprintf(configurationFilePath, pathLenght, "%s\\%s", configurationFolder, MARKVIEW_CONFIGURATION_FILENAME);

	if (configurationFolder) {
		free(configurationFolder);
	}

	return configurationFilePath;
}
