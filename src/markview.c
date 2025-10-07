#include <webview/types.h>
#include <markview.h>
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
#include <SDL3/SDL_system.h>
#include <sample_md.h>
#include <prism_min_js.h>
#include <prism_css.h>
#include <windef.h>
#include <markview/markdown.h>
#include <markview/scripting.h>
#include <cJSON.h>

#define STYLE_TAG_FORMAT "<style>%s</style>"
#define TITLE_SEPARATOR " - "
#define FILENAME_EMPTY_FILE "Welcome"
#define WINDOW_DEFAULT_WIDTH 900
#define WINDOW_DEFAULT_HEIGHT 600
#define DEFAULT_WINDOW_SIZE SDL_WINDOW_RESIZABLE

#define CMARK_OPTIONS CMARK_OPT_DEFAULT | CMARK_OPT_FOOTNOTES | CMARK_OPT_LIBERAL_HTML_TAG

typedef struct {
	char* title;
	char* html;
	webview_t webview;
	SDL_Window* window;
	SDL_Renderer* renderer;
	int windowWidth;
	int windowHeight;
} markview_detail;

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

void focus_webview(webview_t webview) {
	ICoreWebView2Controller* controller_ptr =
		(ICoreWebView2Controller *)webview_get_native_handle(webview, WEBVIEW_NATIVE_HANDLE_KIND_BROWSER_CONTROLLER);

	if (controller_ptr) {
		controller_ptr->lpVtbl->MoveFocus(controller_ptr, COREWEBVIEW2_MOVE_FOCUS_REASON_PROGRAMMATIC);
	}
}


markview_t markview_create(char* filename) {
	SDL_Log("starting %s\n", MARKVIEW_PROGRAM_NAME);
	markview_detail* markview = malloc(sizeof(markview_detail));

	// set window title
	char* actualFilename = NULL == filename ? FILENAME_EMPTY_FILE: filename;
	size_t titleSize = strlen(MARKVIEW_PROGRAM_NAME) + strlen(TITLE_SEPARATOR) + strlen(actualFilename) + 1;
	markview->title = malloc(titleSize);
	snprintf(markview->title, titleSize , "%s%s%s", MARKVIEW_PROGRAM_NAME, TITLE_SEPARATOR, actualFilename);

	markview->windowWidth = WINDOW_DEFAULT_WIDTH;
	markview->windowHeight = WINDOW_DEFAULT_HEIGHT;

	SDL_WindowFlags windowFlags = DEFAULT_WINDOW_SIZE;

	cJSON* width = NULL;
	cJSON* height = NULL;
	cJSON* borderless = NULL;
	cJSON* maximized = NULL;

	char* configurationFilePath = markview_configuration_file_path();
	SDL_Log("Reading configuration file at %s", configurationFilePath);

	if (markview_file_exists(configurationFilePath)) {
		char* configurationContent = markview_file_read(configurationFilePath);
		cJSON* configurationJson = cJSON_Parse(configurationContent);
		const char *error_ptr = NULL;

		if (NULL == configurationJson) {
			error_ptr = cJSON_GetErrorPtr();
			if (error_ptr != NULL) {
				SDL_Log("configurationJson error. Error: %s\n", error_ptr);
			}
		}

		width = cJSON_GetObjectItemCaseSensitive(configurationJson, "width");
		height = cJSON_GetObjectItemCaseSensitive(configurationJson, "height");
		borderless = cJSON_GetObjectItemCaseSensitive(configurationJson, "borderless");
		maximized = cJSON_GetObjectItemCaseSensitive(configurationJson, "maximized");

		if (cJSON_IsTrue(borderless)) {
			windowFlags = windowFlags | SDL_WINDOW_BORDERLESS;
		}

		if (cJSON_IsTrue(maximized)) {
			windowFlags = windowFlags | SDL_WINDOW_MAXIMIZED;
		} else {
			if (cJSON_IsNumber(width)) {
				markview->windowWidth = width->valueint;
			}

			if (cJSON_IsNumber(height)) {
				markview->windowHeight = height->valueint;
			}
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
		return NULL;
	}

	// Create a window
	if (!SDL_CreateWindowAndRenderer(markview->title, markview->windowWidth, markview->windowHeight, windowFlags, &markview->window, &markview->renderer)) {
		SDL_Log("Window or renderer could not be created! SDL_Error: %s\n", SDL_GetError());
		SDL_Quit();
		return NULL;
	}

	// Paint screen according to theme.
	if (!SDL_SetRenderDrawColor(markview->renderer, 255, 255, 255, 255) || !SDL_RenderClear(markview->renderer) || !SDL_RenderPresent(markview->renderer)) {
		SDL_Log("Could not clear window! SDL_Error: %s\n", SDL_GetError());
	}

	size_t styleSize = strlen(STYLE_TAG_FORMAT) + prism_css_size + 1;
	char *styles = malloc(styleSize);

	snprintf(styles, styleSize, STYLE_TAG_FORMAT, prism_css_data);

	if (NULL != filename) {
		printf("reading file: %s\n", filename);

		// 1. Load the markdow file
		char *markdown = markview_file_read(filename);
		if (!markdown) {
			printf("error reading markdown\n");
			return NULL;
		}

		// 2. Parse markdown to html
		char *rawHtml = markview_markdown_to_html(markdown, strlen(markdown), CMARK_OPTIONS);

		size_t htmlSize = strlen(rawHtml) + strlen(styles) + 1;

		markview->html = malloc(htmlSize);

		snprintf(markview->html, htmlSize, "%s\n%s", styles, rawHtml);

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
		char *rawHtml = markview_markdown_to_html((char *)sample_md_data, sample_md_size, CMARK_OPTIONS);

		size_t htmlSize = strlen(rawHtml) + strlen(styles) + 1;

		markview->html = malloc(htmlSize);

		snprintf(markview->html, htmlSize, "%s\n%s", styles, rawHtml);

		if (rawHtml) {
			free(rawHtml);
		}
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
	} else {
		// https://github.com/webview/webview/issues/1195#issuecomment-2380564512
		resize_window(markview->webview);
		focus_webview(markview->webview);

		webview_set_html(markview->webview, markview->html);
	}

	printf("Evaluate scripts\n");

	if (!markview_run_script(markview->webview, (char *)prism_min_js_data)) {
		printf("Error: evaluating prism.js\n");
	}

	return markview;
}

int markview_run(markview_t app) {
	markview_detail* markview = app;
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
				resize_window(markview->webview);
				continue;
			}

			if (event.type == SDL_EVENT_DROP_FILE) {
				// TODO: handle open files by droping file inside
				SDL_Log("File was droped");
				continue;
			}

			if (event.type == SDL_EVENT_KEY_DOWN) {
				if (event.key.scancode == SDL_SCANCODE_F11) {
					SDL_SetWindowFullscreen(markview->window, (fullscreen =! fullscreen));
				}
			}
		}
	}

	// save configuration
	char* configurationFolderPath = markview_configuration_folder_path();
	if (markview_folder_create(configurationFolderPath)) {
		SDL_GetWindowSize(markview->window, &markview->windowWidth, &markview->windowHeight);

		char* configurationFilePath = markview_configuration_file_path();
		char* configurationContent = markview_file_read(configurationFilePath);

		cJSON* configurationJson = cJSON_CreateObject();
		cJSON* width = cJSON_CreateNumber(markview->windowWidth);
		cJSON* height = cJSON_CreateNumber(markview->windowHeight);

		SDL_WindowFlags windowFlags = SDL_GetWindowFlags(markview->window);
		cJSON* borderless = cJSON_CreateBool(windowFlags & SDL_WINDOW_BORDERLESS);
		cJSON* maximized = cJSON_CreateBool(windowFlags & SDL_WINDOW_MAXIMIZED);

		cJSON_AddItemToObject(configurationJson, "width", width);
		cJSON_AddItemToObject(configurationJson, "height", height);
		cJSON_AddItemToObject(configurationJson, "borderless", borderless);
		cJSON_AddItemToObject(configurationJson, "maximized", maximized);

		char* jsonString = cJSON_Print(configurationJson);

		SDL_Log("saving configuration\n");
		SDL_Log("%s", jsonString);

		markview_file_write(configurationFilePath, jsonString);

		if (jsonString) {
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
