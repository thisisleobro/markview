// #include "SDL3/SDL_init.h"
// #include "SDL3/SDL_oldnames.h"
// #include "SDL3/SDL_properties.h"
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


int main(int argc, char** argv) {
	printf("starting %s\n", PROGRAM_NAME);
	char windowTitle[TITLE_MAX_SIZE];

	char* filename = argv[1];

	char* html = NULL;
	// // Initialize SDL
	// if (!SDL_Init(SDL_INIT_EVENTS)) {
	//     printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
	//     return -1;
	// }

	// // Create a window
	// SDL_Window* window = SDL_CreateWindow(
	//     windowTitle,                // Window title
	//     800, 600,                         // Window width and height
	//     SDL_WINDOW_RESIZABLE              // Flags
	// );

	// if (window == NULL) {
	//     printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
	//     SDL_Quit();
	//     return -1;
	// }

	// SDL_PropertiesID properties = SDL_GetWindowProperties(window);
	// HWND hwnd = (HWND)SDL_GetPointerProperty(properties, SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);
	// if (hwnd) {
	//     printf("Window HANDLE (HWND): %p\n", hwnd);
	// } else {
	//     printf("HWND property not found.\n");
	// }

	size_t styleSize = strlen(STYLE_TAG_FORMAT) + prism_css_size + 1;
	char* styles = malloc(styleSize);

	snprintf(styles, styleSize, STYLE_TAG_FORMAT, prism_css_data);

	// printf("styles: %s\n", styles);

	if (argc > 1) {
		printf("reading file: %s\n", filename);
		
		// 1. Load the markdow file
		char* markdown = markview_read_file(filename);
		if (!markdown) {
			printf("error reading markdown");
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

		snprintf(windowTitle, TITLE_MAX_SIZE - 1, "%s -  %s", PROGRAM_NAME, filename);
	} else {
		printf("No file provided. Showing welcome file\n");
		char* rawHtml = markdown_to_html((char*)sample_md_data, sample_md_size, CMARK_OPTIONS);

		size_t htmlSize = strlen(rawHtml) + strlen(styles) + 1;

		html = malloc(htmlSize);

		snprintf(html, htmlSize, "%s\n%s", styles, rawHtml);

		// printf("html: %s", html);

		snprintf(windowTitle, TITLE_MAX_SIZE - 1, "%s -  %s", PROGRAM_NAME, "Welcome");

		if (rawHtml) {
			free(rawHtml);
		}
	}

	// printf("html: %s\n", html);

	// 3. webview_set_html(w, the_html);
	// webview_t w = webview_create(1, hwnd);
	webview_t w = webview_create(1, NULL);
	webview_set_title(w, windowTitle);
	webview_set_html(w, html);

	printf("Evaluate scripts\n");
	webview_error_t evalError = webview_eval(w, (char*)prism_min_js_data);
	if (evalError == WEBVIEW_ERROR_OK) {
		printf("Prism executed sucesfully");
	} else {
		printf("Error: evaluating prism: %d", evalError);
	}

	// int running = 1;
	// SDL_Event event;

	// while (running) {
	//     // Handle events
	//     while (SDL_PollEvent(&event)) {
	//         if (event.type == SDL_EVENT_QUIT) {
	//             running = 0;
	//         }
	//     }

	//     // Clear the window (optional, fill with a color)
	//     // SDL_SetRenderDrawColor(SDL_GetRenderer(window), 0, 0, 0, 255);  // Black
	//     // SDL_RenderClear(SDL_GetRenderer(window));

	//     // Present the renderer (display the window)
	//     // SDL_RenderPresent(SDL_GetRenderer(window));
	// }
	
	webview_run(w);
	webview_destroy(w);

	if (html)
	{
		free(html);
	}

	return 0;
}
