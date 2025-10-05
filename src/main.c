#include <markview/constants.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_video.h>
#include <WebView2.h>
#include <markview/markdown.h>
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
#include <markview/filesystem.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_system.h>
#include <sample_md.h>
#include <prism_min_js.h>
#include <prism_css.h>
#include <windef.h>
#include <markview/markdown.h>
#include <markview/scripting.h>
#include <cJSON.h>

#define TITLE_MAX_SIZE 256
#define STYLE_TAG_FORMAT "<style>%s</style>"

#define CMARK_OPTIONS CMARK_OPT_DEFAULT | CMARK_OPT_FOOTNOTES | CMARK_OPT_LIBERAL_HTML_TAG


void resize_window(webview_t webview) {
	HWND widget_handle = (HWND)webview_get_native_handle(webview, WEBVIEW_NATIVE_HANDLE_KIND_UI_WIDGET);
	if (widget_handle) {
		RECT r = {};
		if (GetClientRect(GetParent(widget_handle), &r)) {
			MoveWindow(widget_handle, r.left, r.top, r.right - r.left, r.bottom - r.top, TRUE);
		}
	}
}

void focus_webview(webview_t webview) {
	ICoreWebView2Controller* controller_ptr =
		(ICoreWebView2Controller *)webview_get_native_handle(webview, WEBVIEW_NATIVE_HANDLE_KIND_BROWSER_CONTROLLER);

	if (controller_ptr) {
		controller_ptr->lpVtbl->MoveFocus(controller_ptr, COREWEBVIEW2_MOVE_FOCUS_REASON_PROGRAMMATIC);
	}
}

int main(int argc, char** argv) {
	SDL_Log("starting %s\n", MARKVIEW_PROGRAM_NAME);
	char windowTitle[TITLE_MAX_SIZE] = {};
	char* html = NULL;
	char* filename = argv[1];
	int windowWidth = 900, windowHeight = 600;
	SDL_WindowFlags windowFlags = SDL_WINDOW_RESIZABLE;
	cJSON* configurationJson = NULL;
	cJSON* width = NULL;
	cJSON* height = NULL;
	cJSON* borderless = NULL;

	snprintf(windowTitle, TITLE_MAX_SIZE - 1, "%s -  %s", MARKVIEW_PROGRAM_NAME, argc > 1? filename :"Welcome");

	char* configurationFilePath = markview_configuration_file_path();
	SDL_Log("Reading configuration file at %s", configurationFilePath);

	if (markview_file_exists(configurationFilePath)) {
		char* configurationContent = markview_file_read(configurationFilePath);
		configurationJson = cJSON_Parse(configurationContent);
		const char* error_ptr =  NULL;

		if (NULL == configurationJson)
		{
			error_ptr = cJSON_GetErrorPtr();
			if (error_ptr != NULL)
			{
				SDL_Log("configurationJson error. Error: %s\n", error_ptr);
			}
		}

		width = cJSON_GetObjectItemCaseSensitive(configurationJson, "width");
		height = cJSON_GetObjectItemCaseSensitive(configurationJson, "height");
		borderless = cJSON_GetObjectItemCaseSensitive(configurationJson, "borderless");

		if (cJSON_IsNumber(width)) {
			windowWidth = width->valueint;
		}

		if (cJSON_IsNumber(height)) {
			windowHeight = height->valueint;
		}

		if (cJSON_IsTrue(borderless)) {
			windowFlags = windowFlags | SDL_WINDOW_BORDERLESS;
		}

		if (configurationContent) {
			free(configurationContent);
		}

		cJSON_Delete(configurationJson);
	}

	SDL_Log("Finished reading configuration file");

	// Initialize SDL
	if (!SDL_Init(SDL_INIT_EVENTS)) {
		SDL_Log("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
		SDL_Quit();
		return -1;
	}

	// Create a window
	SDL_Window* window = NULL;
	SDL_Renderer* renderer = NULL;

	if (!SDL_CreateWindowAndRenderer(windowTitle, windowWidth, windowHeight, windowFlags, &window, &renderer)) {
		SDL_Log("Window or renderer could not be created! SDL_Error: %s\n", SDL_GetError());
		SDL_Quit();
		return -1;
	}

	// Paint screen according to theme.
	if (!SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255) || !SDL_RenderClear(renderer) || !SDL_RenderPresent(renderer)) {
		SDL_Log("Could not clear window! SDL_Error: %s\n", SDL_GetError());
	}

	size_t styleSize = strlen(STYLE_TAG_FORMAT) + prism_css_size + 1;
	char* styles = malloc(styleSize);

	snprintf(styles, styleSize, STYLE_TAG_FORMAT, prism_css_data);

	if (argc > 1) {
		printf("reading file: %s\n", filename);

		// 1. Load the markdow file
		char* markdown = markview_file_read(filename);
		if (!markdown) {
			printf("error reading markdown\n");
			return 1;
		}

		// 2. Parse markdown to html
		char* rawHtml = markview_markdown_to_html(markdown, strlen(markdown), CMARK_OPTIONS);

		size_t htmlSize = strlen(rawHtml) + strlen(styles) + 1;

		html = malloc(htmlSize);

		snprintf(html, htmlSize, "%s\n%s", styles, rawHtml);

		if (rawHtml) {
			free(rawHtml);
		}

		if (styles) {
			free(styles);
		}

		if (markdown) {
			free(markdown);
		}
	} else {
		printf("No file provided. Showing welcome file\n");
		char* rawHtml = markview_markdown_to_html((char*)sample_md_data, sample_md_size, CMARK_OPTIONS);

		size_t htmlSize = strlen(rawHtml) + strlen(styles) + 1;

		html = malloc(htmlSize);

		snprintf(html, htmlSize, "%s\n%s", styles, rawHtml);

		if (rawHtml) {
			free(rawHtml);
		}
	}

	SDL_PropertiesID properties = SDL_GetWindowProperties(window);
	HWND hwnd = (HWND)SDL_GetPointerProperty(properties, SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);
	if (hwnd) {
		SDL_Log("Window HANDLE (HWND): %p\n", hwnd);
	} else {
		SDL_Log("HWND property not found.\n");
	}

	// webview
	webview_t w = webview_create(1, hwnd);
	if (NULL == w) {
		SDL_Log("Error creating webview\n");
	} else {
		// https://github.com/webview/webview/issues/1195#issuecomment-2380564512
		resize_window(w);
		focus_webview(w);

		webview_set_html(w, html);
	}

	printf("Evaluate scripts\n");
	if (!markview_run_script(w, (char*)prism_min_js_data)) {
		printf("Error: evaluating prism.js\n");
	}

	int running = 1;
	SDL_Event event;
	bool fullscreen;

	while (running) {
		// Handle events
		while (SDL_PollEvent(&event)) {
			SDL_Log("event: %d", event.type);
			if (event.type == SDL_EVENT_QUIT) {
				SDL_Log("Quit, please!");
				running = 0;
				continue;
			}

			if (event.type == SDL_EVENT_WINDOW_RESIZED) {
				resize_window(w);
				continue;
			}

			if (event.type == SDL_EVENT_DROP_FILE) {
				// TODO: handle open files by droping file inside
				SDL_Log("File was droped");
				continue;
			}

			if (event.type == SDL_EVENT_KEY_DOWN) {
				if (event.key.scancode == SDL_SCANCODE_F11) {
					SDL_SetWindowFullscreen(window, (fullscreen =! fullscreen));
				}
			}
		}
	}

	// save configuration
	char* configurationFolderPath = markview_configuration_folder_path();
	if (markview_folder_create(configurationFolderPath)) {
		SDL_GetWindowSize(window, &windowWidth, &windowHeight);

		configurationJson = cJSON_CreateObject();
		width = cJSON_CreateNumber(windowWidth);
		height = cJSON_CreateNumber(windowHeight);
		borderless = cJSON_CreateBool(windowFlags & SDL_WINDOW_BORDERLESS);
		cJSON_AddItemToObject(configurationJson, "width", width);
		cJSON_AddItemToObject(configurationJson, "height", height);
		cJSON_AddItemToObject(configurationJson, "borderless", borderless);

		char* jsonString = cJSON_Print(configurationJson);

		SDL_Log("saving configuration\n");
		SDL_Log("%s", jsonString);

		markview_file_write(configurationFilePath, jsonString);

		if (jsonString)
		{
			free(jsonString);
		}

		if (configurationFolderPath) {
			free(configurationFolderPath);
		}
		if (configurationFilePath) {
			free(configurationFilePath);
		}
		
		cJSON_Delete(configurationJson);
	}

	// deallocate stuff
	webview_destroy(w);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	if (html) {
		free(html);
	}	

	return 0;
}
