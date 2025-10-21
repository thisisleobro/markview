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
#include <prism_min_js.h>
#include <prism_css.h>
#include <markview_js.h>
#include <windef.h>
#include "markdown.h"
#include "settings.h"
#include "utils.h"
#include <cJSON.h>


typedef struct {
	markview_settings_t settings;
	char* title;
	char* html;
	webview_t webview;
	SDL_Window* window;
	SDL_Renderer* renderer;
} markview_detail;

bool _markview_run_javascript(markview_t app, char* content) {
	markview_detail* markview = app;

	// fprintf(stderr, "running js: '%s'\n",content);
	webview_error_t evalError = webview_eval(markview->webview, content);
	
	if (evalError == WEBVIEW_ERROR_OK) {
		return true;
	}

	fprintf(stderr, "some error running js: '%s'\n",content);

	return false;
}

bool _markview_apply_css(markview_t app, char* content) {
	markview_detail* markview = app;

	const char* script = "applyCss(\"%s\");\n";

	const char* base64 = (char*)base64_encode((const unsigned char*)content, strlen(content), NULL);

	// fprintf(stdout, "\tbase64: '%s'\n%s\n", base64, markview->title);

	const size_t len = strlen(script) + strlen(base64) + 1;

	char* fullScript = malloc(len);

	snprintf(fullScript, len, script, base64);

	// fprintf(stdout, "\tinjecting: \n%s\n", fullScript);

	webview_error_t evalError = webview_eval(markview->webview, fullScript);

	if (evalError == WEBVIEW_ERROR_OK) {
		return true;
	}

	fprintf(stderr, "some error applying css\n");

	return false;
}

// TODO: handle cross platform
void resize_window(webview_t webview) {
	HWND widget_handle = (HWND)webview_get_native_handle(webview, WEBVIEW_NATIVE_HANDLE_KIND_UI_WIDGET);
	if (widget_handle) {
		RECT r = {};
		if (GetClientRect(GetParent(widget_handle), &r)) {
			MoveWindow(widget_handle, r.left, r.top, r.right - r.left, r.bottom - r.top, TRUE);
		}
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

void focus_webview(webview_t webview) {
	ICoreWebView2Controller* controller_ptr =
		(ICoreWebView2Controller *)webview_get_native_handle(
			webview, WEBVIEW_NATIVE_HANDLE_KIND_BROWSER_CONTROLLER);

	if (controller_ptr) {
		controller_ptr->lpVtbl->MoveFocus(controller_ptr, COREWEBVIEW2_MOVE_FOCUS_REASON_PROGRAMMATIC);
	}
}

markview_t markview_create() {
	fprintf(stdout, "starting %s\n", MARKVIEW_PROGRAM_NAME);
	markview_detail* markview = malloc(sizeof(markview_detail));
	markview->html = NULL;
	markview->title = NULL;
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
	if (!SDL_SetRenderDrawColor(markview->renderer, 255, 255, 255, 255)
		|| !SDL_RenderClear(markview->renderer)
		|| !SDL_RenderPresent(markview->renderer))
	{
		SDL_Log("Could not clear window! SDL_Error: %s\n", SDL_GetError());
	}

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

	resize_window(markview->webview);
	focus_webview(markview->webview);

	// bind functions
	webview_bind(markview->webview, "markview_toggle_fullscreen", markview_toggle_fullscreen, markview);
	webview_bind(markview->webview, "markview_open_file", markview_open_file, markview);

	return markview;
}

bool markview_render_from_file(markview_t app, char* title, char* filename) {
	if (!markview_file_exists(filename)) {
		fprintf(stderr, "file does not exist or you dont have access to it\n");
		return false;
	}

	char* content = markview_file_read(filename);

	return markview_render_from_string(app, title, content);
}

bool markview_render_from_string(markview_t app, char* title, char* content) {
	markview_detail* markview = app;

	// if (markview->html) {
	// 	// navigate everytime but first. This allows us to use the nativa navigation
	// 	webview_navigate(markview->webview, "about:blank");	
	// }


	// deallocate in case we call multiple times
	if (markview->html)
		free(markview->html);

	if (markview->title)
		free(markview->title);

	size_t titleSize = strlen(MARKVIEW_PROGRAM_NAME) + strlen(MARKVIEW_TITLE_SEPARATOR) + strlen(title) + 1;
	
	markview->title = malloc(titleSize);
	// TODO: assert memory is valid
	snprintf(markview->title, titleSize , "%s%s%s", MARKVIEW_PROGRAM_NAME, MARKVIEW_TITLE_SEPARATOR, title);

	markview->html = markview_markdown_to_html(content, strlen(content), MARKVIEW_CMARK_OPTIONS);

	webview_set_html(markview->webview, markview->html);
	webview_set_title(markview->webview, markview->title);

	fprintf(stdout, "apply scripts and css for '%s'\n", markview->title);
	// _markview_run_javascript(markview, "alert(1)");
	_markview_run_javascript(markview, (char*)markview_js_data);
	_markview_run_javascript(markview, (char*)prism_min_js_data);
	_markview_apply_css(markview, (char*)prism_css_data);

	return true;
}

int markview_run(markview_t app) {
	markview_detail* markview = app;
	int running = 1;
	SDL_Event event;

	while (running) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_EVENT_QUIT) {
				SDL_Log("Quit, please!");
				running = 0;
				continue;
			}

			if (event.type == SDL_EVENT_WINDOW_RESIZED) {
				resize_window(markview->webview);
				continue;
			}

			if (event.type == SDL_EVENT_DROP_FILE) {
				// TODO: handle open files by droping file inside
				continue;
			}

			if (event.type == SDL_EVENT_KEY_DOWN) {
				if (event.key.scancode == SDL_SCANCODE_F11) {
					SDL_SetWindowFullscreen(
						markview->window,
						!(SDL_GetWindowFlags(markview->window) & SDL_WINDOW_FULLSCREEN));
				}
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

	if (markview->title) {
		free(markview->title);
	}

	if (markview->html) {
		free(markview->html);
	}

	return 0;
}

