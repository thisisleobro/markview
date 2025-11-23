#include <webview/types.h>
#include "markview.h"
#include "constants.h"
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_video.h>
#include <WebView2.h>
#include "markdown.h"
#include "shell_html.h"
#include <webview/errors.h>
#include <stdio.h>
#include <stdlib.h>
#include <webview/api.h>
#include <webview/webview.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <cmark-gfm.h>
#include <cmark_ctype.h>
#include <cmark-gfm_version.h>
#include <cmark-gfm_export.h>
#include <cmark-gfm-extension_api.h>
#include <cmark-gfm-core-extensions.h>
#include <string.h>
#include "filesystem.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_system.h>
#include <welcome_md.h>
#include <purify_min_js.h>
#include <prism_css.h>
#include <prism_min_js.h>
#include "settings.h"
#include "utils.h"
#include <cJSON.h>

#define FUNCTION_SIGNATURE "%s(\"%s\");"

typedef struct {
	markview_settings_t settings;
	char* title;
	webview_t webview;
	SDL_Window* window;
	SDL_Renderer* renderer;
} markview_detail;


char* _markview_format_javasript_call(markview_t app, char* functionName, char* content) {
	markview_detail* markview = app;

	const char* base64 = (char*)base64_encode((const unsigned char*)content, strlen(content), NULL);

	const size_t len = strlen(FUNCTION_SIGNATURE) + strlen(functionName) + strlen(base64) + 1;

	char* fullScript = malloc(len);

	snprintf(fullScript, len, FUNCTION_SIGNATURE, functionName, base64);

	// printf("javascript: %s\n", fullScript);

	if (base64)
		free((void*)base64);

	return fullScript;
}

bool markview_webview_run_javascript(markview_t app, char* content, size_t lenght) {
	markview_detail* markview = app;

	char* tempContent = malloc(lenght);
	strncpy(tempContent, content, lenght);

	tempContent[lenght] = '\0';

	printf("running js: '%s'\n", tempContent);
	webview_error_t evalError = webview_eval(markview->webview, tempContent);
	if (tempContent)
		free((void *) tempContent);

	if (evalError == WEBVIEW_ERROR_OK) {
		return true;
	}

	fprintf(stderr, "some error running js: '%s'\n",content);

	return false;
}

bool markview_webview_render_html(markview_t app, char* content) {
	markview_detail* markview = app;

	char* functionCall = _markview_format_javasript_call(markview, "markview_api_render_html", content);

	webview_error_t evalError = webview_eval(markview->webview, functionCall);

	if (evalError == WEBVIEW_ERROR_OK) {
		return true;
	}

	fprintf(stderr, "some error applying css\n");

	return false;
}

bool markview_webview_apply_css(markview_t app, char* content) {
	markview_detail* markview = app;

	char* functionCall = _markview_format_javasript_call(markview, "markview_api_apply_css_from_base64", content);

	webview_error_t evalError = webview_eval(markview->webview, functionCall);

	if (evalError == WEBVIEW_ERROR_OK) {
		return true;
	}

	fprintf(stderr, "some error applying css\n");

	return false;
}

// TODO: handle cross platform
void show_webview(markview_detail* markview) {
	HWND widget_handle = (HWND)webview_get_native_handle(markview->webview, WEBVIEW_NATIVE_HANDLE_KIND_UI_WIDGET);
	if (widget_handle) {
		RECT r = {};
		if (GetClientRect(GetParent(widget_handle), &r)) {
			MoveWindow(widget_handle, r.left, r.top, r.right - r.left, r.bottom - r.top, TRUE);
		}
	}
}

void hide_webview(markview_detail* markview) {
	HWND widget_handle = (HWND)webview_get_native_handle(markview->webview, WEBVIEW_NATIVE_HANDLE_KIND_UI_WIDGET);
	if (widget_handle) {
		RECT r = {};
		if (GetClientRect(GetParent(widget_handle), &r)) {
			MoveWindow(widget_handle, r.left, r.top, 0, 0, TRUE);
		}
	}
}

void clear_window(markview_detail* markview) {
	if (!SDL_SetRenderDrawColor(markview->renderer, 255, 255, 255, 255)
		|| !SDL_RenderClear(markview->renderer)
		|| !SDL_RenderPresent(markview->renderer))
	{
		SDL_Log("Could not clear window! SDL_Error: %s\n", SDL_GetError());
	}
}

void markview_toggle_fullscreen(const char *id, const char *req, void *arg) {
	markview_detail* markview = (markview_detail*)arg;
	SDL_SetWindowFullscreen(markview->window, !(SDL_GetWindowFlags(markview->window) & SDL_WINDOW_FULLSCREEN));
}

void markview_open_file(const char *id, const char *req, void *arg) {
	markview_detail* markview = (markview_detail*)arg;

	cJSON* argumentsArray = cJSON_Parse(req);
	cJSON* filenameField = cJSON_GetArrayItem(argumentsArray, 0);

	if (!cJSON_IsString(filenameField)) {
		fprintf(stderr, "markview_open_file called with invalid argument filename\n");
		return;
	}

	char* filename = malloc(strlen(filenameField->valuestring) + 1);
	strcpy(filename, filenameField->valuestring);

	cJSON_Delete(argumentsArray);

	markview_render_from_file(markview, filename, filename);
}

void markview_hide_webview(const char *id, const char *req, void *arg) {
	markview_detail* markview = (markview_detail*)arg;

	hide_webview(markview);
}

markview_t markview_create() {
	fprintf(stdout, "starting %s\n", MARKVIEW_PROGRAM_NAME);
	markview_detail* markview = malloc(sizeof(markview_detail));
	memset(markview, 0, sizeof(markview_detail));

	markview->settings = markview_settings_deserialize();

	SDL_WindowFlags windowFlags = MARKVIEW_SDL_DEFAULT_WINDOW_SIZE;
	windowFlags = markview->settings.maximized? windowFlags | SDL_WINDOW_MAXIMIZED: windowFlags;
	windowFlags = markview->settings.borderless? windowFlags | SDL_WINDOW_BORDERLESS: windowFlags;

	// Initialize SDL
	if (!SDL_Init(SDL_INIT_EVENTS)) {
		SDL_Log("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
		SDL_Quit();
		return NULL;
	}

	// Create a window
	if (!SDL_CreateWindowAndRenderer(
		MARKVIEW_PROGRAM_NAME,
		markview->settings.width,
		markview->settings.height,
		windowFlags,
		&markview->window,
		&markview->renderer))
	{
		SDL_Log("Window or renderer could not be created! SDL_Error: %s\n", SDL_GetError());
		SDL_Quit();
		return NULL;
	}

	// Paint screen according to theme.
	clear_window(markview);

	SDL_PropertiesID properties = SDL_GetWindowProperties(markview->window);
	HWND hwnd = (HWND)SDL_GetPointerProperty(properties, SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);
	if (hwnd) {
		SDL_Log("Window HANDLE (HWND): %p\n", hwnd);
	} else {
		SDL_Log("HWND property not found.\n");
	}

	// webview
	markview->webview = webview_create(1, hwnd);
	if (NULL == markview->webview) {
		SDL_Log("Error creating webview\n");
	}

	webview_set_html(markview->webview, (char*)shell_html_data);
	// bind functions
	webview_bind(markview->webview, "markview_toggle_fullscreen", markview_toggle_fullscreen, markview);
	webview_bind(markview->webview, "markview_open_file", markview_open_file, markview);
	webview_bind(markview->webview, "markview_hide_webview", markview_hide_webview, markview);
	// TODO: pass color scheme before showing
	show_webview(markview);

	// apply some css
	markview_webview_apply_css(markview, (char*)prism_css_data);
	// run some javascript and apply some css
	markview_webview_run_javascript(markview, (char*)prism_min_js_data, prism_min_js_size);
	markview_webview_run_javascript(markview, (char*)purify_min_js_data, purify_min_js_size);

	return markview;
}

bool markview_render_from_file(markview_t app, char* title, char* filename) {
	if (!markview_file_exists(filename)) {
		fprintf(stderr, "file does not exist or you dont have access to it\n");
		return false;
	}

	char* content = markview_file_read(filename);

	bool returnValue = markview_render_from_string(app, title, content);

	if (content)
		free(content);

	return returnValue;
}

bool markview_render_from_string(markview_t app, char* title, char* content) {
	markview_detail* markview = app;

	char* html = markview_markdown_to_html(content, strlen(content), MARKVIEW_CMARK_OPTIONS);

	if (markview->title)
		free(markview->title);

	size_t titleSize = strlen(MARKVIEW_PROGRAM_NAME) + strlen(MARKVIEW_TITLE_SEPARATOR) + strlen(title) + 1;

	markview->title = malloc(titleSize);
	snprintf(markview->title, titleSize , "%s%s%s", title, MARKVIEW_TITLE_SEPARATOR, MARKVIEW_PROGRAM_NAME);

	webview_set_title(markview->webview, markview->title);

	markview_webview_render_html(markview, html);

	return true;
}

int markview_run(markview_t app) {
	markview_detail* markview = app;
	int running = 1;
	SDL_Event event;

	while (running) {
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_EVENT_QUIT:
				SDL_Log("Quit, please!");
				running = 0;
				break;
			case SDL_EVENT_WINDOW_RESIZED:
				show_webview(markview);
				break;
			case SDL_EVENT_DROP_BEGIN:
				// Only catches when hovered over window borders because webview widget eats most events.
				// To circumvent, we resize the webview widget to (0, 0) so that we can get SDL_EVENT_DROP_FILE
				hide_webview(markview);
				break;
			case SDL_EVENT_DROP_FILE:
				markview_render_from_file(markview, (char*)event.drop.data, (char*)event.drop.data);
				break;
			case SDL_EVENT_DROP_COMPLETE:
				// Resize webview widget back now that we already handled SDL_EVENT_DROP_FILE
				show_webview(markview);
				break;
			case SDL_EVENT_KEY_DOWN:
				if (event.key.scancode == SDL_SCANCODE_F11) {
					SDL_SetWindowFullscreen(
						markview->window,
						!(SDL_GetWindowFlags(markview->window) & SDL_WINDOW_FULLSCREEN));
				}
				break;
			}
		}
	}

	// save configuration
	markview->settings.maximized = SDL_GetWindowFlags(markview->window) & SDL_WINDOW_MAXIMIZED;
	markview->settings.borderless = SDL_GetWindowFlags(markview->window) & SDL_WINDOW_BORDERLESS;
	SDL_GetWindowSize(markview->window, &markview->settings.width, &markview->settings.height);

	markview_settings_serialize(markview->settings);

	// deallocations
	webview_destroy(markview->webview);

	SDL_DestroyRenderer(markview->renderer);
	SDL_DestroyWindow(markview->window);
	SDL_Quit();

	return 0;
}

