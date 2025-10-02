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
#include <markview/file.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_system.h>
#include <sample_md.h>
#include <prism_min_js.h>
#include <prism_css.h>
#include <windef.h>

#define PROGRAM_NAME "markview"
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
	// SDL_Log("starting %s\n", PROGRAM_NAME);
	char windowTitle[TITLE_MAX_SIZE] = {};

	snprintf(windowTitle,
		TITLE_MAX_SIZE - 1, "%s -  %s",
		PROGRAM_NAME,
		argc > 1? argv[1] :"Welcome"
	);

	char* filename = argv[1];

	char* html = NULL;

	// Initialize SDL
	if (!SDL_Init(SDL_INIT_EVENTS)) {
		// SDL_Log("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
		return -1;
	}

	// Create a window
	SDL_Window* window = SDL_CreateWindow(windowTitle, 800, 600, SDL_WINDOW_RESIZABLE);

	if (window == NULL) {
		// SDL_Log("Window could not be created! SDL_Error: %s\n", SDL_GetError());
		SDL_Quit();
		return -1;
	}

	size_t styleSize = strlen(STYLE_TAG_FORMAT) + prism_css_size + 1;
	char* styles = malloc(styleSize);

	snprintf(styles, styleSize, STYLE_TAG_FORMAT, prism_css_data);

	// printf("styles: %s\n", styles);

	if (argc > 1) {
		printf("reading file: %s\n", filename);
		
		// 1. Load the markdow file
		char* markdown = markview_read_file(filename);
		if (!markdown) {
			printf("error reading markdown\n");
			return 1;
		}

		// 2. Parse markdown to html
		// char* rawHtml = cmark_markdown_to_html(markdown, strlen(markdown), CMARK_OPT_SMART);
		char* rawHtml = markdown_to_html(markdown, strlen(markdown), CMARK_OPTIONS);

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
		char* rawHtml = markdown_to_html((char*)sample_md_data, sample_md_size, CMARK_OPTIONS);

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

	webview_t w = webview_create(1, hwnd);

	// webview
	if (NULL == w) {
		SDL_Log("Error creating webview\n");
	} else {
		// https://github.com/webview/webview/issues/1195#issuecomment-2380564512
		// resize widget
		resize_window(w);
		focus_webview(w);

		webview_set_title(w, "windowTitle");
		webview_set_html(w, html);
	}

	printf("Evaluate scripts\n");
	webview_error_t evalError = webview_eval(w, (char*)prism_min_js_data);
	if (evalError == WEBVIEW_ERROR_OK) {
		printf("Prism executed sucesfully\n");
	} else {
		printf("Error: evaluating prism: %d\n", evalError);
	}

	int running = 1;
	SDL_Event event;

	while (running) {
		// Handle events
		while (SDL_PollEvent(&event)) {
			SDL_Log("event: %d", event.type);
			if (event.type == SDL_EVENT_QUIT) {
				// SDL_Log("Quit, please!");
				running = 0;
				continue;
			}

			if (event.type == SDL_EVENT_WINDOW_RESIZED)
			{
				// SDL_Log("windows resized");
				resize_window(w);
				continue;
			}
		}
	}
	
	// webview_run(w);
	webview_destroy(w);

	if (html)
	{
		free(html);
	}

	return 0;
}
