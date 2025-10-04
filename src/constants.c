#include "SDL3/SDL_log.h"
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <markview/constants.h>
#include <stdlib.h>

#define MARKVIEW_CONFIGURATION_FILENAME "configuration.json"


char* markview_configuration_file_path() {
	char* appdata;
	size_t szAppdata = 0;

	SDL_Log("markview_configuration_file_path");

	if (_dupenv_s(&appdata, &szAppdata, "APPDATA") != 0) {
		SDL_Log("_dupenv_s erroed");
		return NULL;
	}

	if (NULL == appdata) {
		SDL_Log("APPDATA is not set");
		return NULL;
	}

	size_t pathLenght = strlen(appdata) + 1 + strlen(MARKVIEW_PROGRAM_NAME) + 1 + strlen(MARKVIEW_CONFIGURATION_FILENAME) + 1;
	char* configurationFilePath = malloc(pathLenght);

	if (NULL == configurationFilePath) {
		SDL_Log("malloc error configurationFilePath is NULL");
		return NULL;
	}

	snprintf(configurationFilePath, pathLenght, "%s\\%s\\%s", appdata, MARKVIEW_PROGRAM_NAME, MARKVIEW_CONFIGURATION_FILENAME);

	if (appdata) {
		free(appdata);
	}

	return configurationFilePath;
}
