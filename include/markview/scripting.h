#ifndef MARKVIEW_SCRIPTING_H
#define MARKVIEW_SCRIPTING_H

#include <webview/types.h>
#include <stdbool.h>


bool markview_run_script(webview_t webview, char* content);
bool markview_run_script_from_file(webview_t webview, char* filename);

#endif // MARKVIEW_SCRIPTING_H