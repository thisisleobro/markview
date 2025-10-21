#ifndef MARKVIEW_H
#define MARKVIEW_H

#include <stdbool.h>


typedef void* markview_t;

markview_t markview_create();

bool markview_render_from_file(markview_t app, char* title, char* filename);

bool markview_render_from_string(markview_t app, char* title, char* content);

int markview_run(markview_t app);

#endif // MARKVIEW_H
