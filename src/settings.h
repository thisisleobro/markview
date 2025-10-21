#ifndef MARKVIEW_SETTINGS_H
#define MARKVIEW_SETTINGS_H

#include <stddef.h>
#include <stdbool.h>


typedef struct {
	int width;
	int height;
	bool borderless;
	bool maximized;
} markview_settings_t;

markview_settings_t markview_settings_deserialize();

void markview_settings_serialize(markview_settings_t);

#endif // MARKVIEW_SETTINGS_H