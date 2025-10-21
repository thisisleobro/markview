#include "settings.h"
#include "filesystem.h"
#include "cJSON.h"
#include "constants.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


char* _markview_settings_folder() {
	char* appdata = getenv("APPDATA");

	if (NULL == appdata) {
		fprintf(stderr, "APPDATA is not set");
		return NULL;
	}

	size_t pathLenght = strlen(appdata) + 1 + strlen(MARKVIEW_PROGRAM_NAME) + 1;
	char* configurationFilePath = malloc(pathLenght);

	if (NULL == configurationFilePath) {
		fprintf(stderr, "malloc error configurationFilePath is NULL");
		return NULL;
	}

	snprintf(configurationFilePath, pathLenght, "%s\\%s\\", appdata, MARKVIEW_PROGRAM_NAME);
	
	return configurationFilePath;	
}

char* _markview_settings_file() {
	char* configurationFolder = _markview_settings_folder();

	if (NULL == configurationFolder) {
		fprintf(stderr, "Erro getting configuration folder");
	}

	size_t pathLenght = strlen(configurationFolder) + 1 + strlen(MARKVIEW_SETTINGS_FILE_NAME) + 1;
	char* configurationFilePath = malloc(pathLenght);

	if (NULL == configurationFilePath) {
		fprintf(stderr, "malloc error configurationFilePath is NULL");
		return NULL;
	}

	snprintf(configurationFilePath, pathLenght, "%s\\%s", configurationFolder, MARKVIEW_SETTINGS_FILE_NAME);

	if (configurationFolder) {
		free(configurationFolder);
	}

	return configurationFilePath;
}

bool _markview_settings_read_bool(cJSON* json, char* fieldName, bool defaultValue) {
	if (NULL == json) {
		fprintf(stdout, "invalid value found for %s. returning default: %d", fieldName, defaultValue);
		return defaultValue;
	}

	cJSON* field = cJSON_GetObjectItemCaseSensitive(json, fieldName);

	if (cJSON_IsBool(field)){
		return cJSON_IsTrue(field);
	}

	return defaultValue;
}

void _markview_settings_write_bool(cJSON* json, char* fieldName, bool value) {
	cJSON* field = cJSON_CreateBool(value);

	cJSON_AddItemToObject(json, fieldName, field);
}

int _markview_settings_read_int(cJSON* json, char* fieldName, int defaultValue) {
	if (NULL == json) {
		fprintf(stdout, "invalid value found for %s. returning default: %d", fieldName, defaultValue);
		return defaultValue;
	}

	cJSON* field = cJSON_GetObjectItemCaseSensitive(json, fieldName);

	if (cJSON_IsNumber(field) && field->valueint > 0){
		return field->valueint;
	}

	return defaultValue;
}

void _markview_settings_write_int(cJSON* json, char* fieldName, int value) {
	cJSON* field = cJSON_CreateNumber(value);

	cJSON_AddItemToObject(json, fieldName, field);
}

markview_settings_t markview_settings_deserialize() {
	const char *cjsonErrorPtr = NULL;
	char* filepath = _markview_settings_file();
	char* fileContents = markview_file_read(filepath);

	cJSON* configurationJson = cJSON_Parse(fileContents);

	if (NULL == configurationJson) {
		cjsonErrorPtr = cJSON_GetErrorPtr();
		if (cjsonErrorPtr != NULL) {
			fprintf(stderr, "markview settings error '%s'\n", cjsonErrorPtr);
		}
	}

	markview_settings_t settings;
	settings.maximized = _markview_settings_read_bool(configurationJson, "maximized", false);
	settings.borderless = _markview_settings_read_bool(configurationJson, "borderless", false);
	settings.width = _markview_settings_read_int(configurationJson, "width", MARKVIEW_WINDOW_DEFAULT_WIDTH);
	settings.height = _markview_settings_read_int(configurationJson, "height", MARKVIEW_WINDOW_DEFAULT_HEIGHT);

	char* jsonString = cJSON_Print(configurationJson);

	fprintf(stdout,  "settings:\n%s\n", jsonString);

	cJSON_Delete(configurationJson);

	if (filepath)
		free(filepath);

	return settings;
}

void markview_settings_serialize(markview_settings_t settings) {
	char* settingsFolder = _markview_settings_folder();
	char* filepath = _markview_settings_file();

	if (!markview_folder_exists(settingsFolder)) {
		markview_folder_create(settingsFolder);
	}

	if (settingsFolder)
		free(settingsFolder);

	cJSON* configurationJson = cJSON_CreateObject();

	_markview_settings_write_int(configurationJson, "width", settings.width);
	_markview_settings_write_int(configurationJson, "height", settings.height);
	_markview_settings_write_bool(configurationJson, "maximized", settings.maximized);
	_markview_settings_write_bool(configurationJson, "borderless", settings.borderless);

	char* jsonString = cJSON_Print(configurationJson);

	markview_file_write(filepath, jsonString);

	if (jsonString)
		free(jsonString);

	if (filepath)
		free(filepath);

	cJSON_Delete(configurationJson);
}