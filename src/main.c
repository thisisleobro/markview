#include "markview/markdown.h"
#include <webview/errors.h>
#include <stdio.h>
#include <stdlib.h>
#include <webview/api.h>
#include <webview/webview.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
// #include <cmark.h>
#include <cmark-gfm.h>
#include <cmark_ctype.h>
#include <cmark-gfm_version.h>
#include <cmark-gfm_export.h>
#include <cmark-gfm-extension_api.h>
#include <cmark-gfm-core-extensions.h>
#include <string.h>
#include <markview/file.h>

#define PROGRAM_NAME "markview"
#define TITLE_MAX_SIZE 256


int main(int argc, char** argv) {
	char* html = NULL;
	char windowTitle[TITLE_MAX_SIZE];

	if (argc > 1) {
		char* filename = argv[1];
		// 1. Load the markdow file
		char* markdown = markview_read_file(filename);
		if (!markdown) {
			printf("error reading markdown");
			return 1;
		}

		// 2. Parse markdown to html
		// char* rawHtml = cmark_markdown_to_html(markdown, strlen(markdown), CMARK_OPT_SMART);
		char* rawHtml = markdown_to_html(markdown, strlen(markdown), CMARK_OPT_DEFAULT | CMARK_OPT_FOOTNOTES);

		char* highlightJsLibTheme = markview_read_file("prism.css");

		char* styleTag = "<style>%s</style>";
		char* styles = malloc(strlen(styleTag) + strlen(highlightJsLibTheme) + 1);
		snprintf(styles, strlen(styleTag) + strlen(highlightJsLibTheme) + 1, styleTag, highlightJsLibTheme);

		// printf("styles: %s", styles);

		size_t htmlSize = strlen(rawHtml) + strlen(styles) + 1;

		html = malloc(htmlSize);

		snprintf(html, htmlSize, "%s\n%s", styles, rawHtml);

		//printf("\n\nhtml:\n%s", html);

		if (styles)
		{
			free(styles);
		}

		if (markdown)
		{
			free(markdown);
		}

		snprintf(windowTitle, TITLE_MAX_SIZE - 1, "%s -  %s", PROGRAM_NAME, filename);
	}

	// printf("html: %s\n", html);

	// 3. webview_set_html(w, the_html);
	webview_t w = webview_create(1, NULL);
	webview_set_title(w, windowTitle);
	// webview_set_size(w, 480, 320, WEBVIEW_HINT_NONE);
	webview_set_html(w, html);
	// webview_init(w, "hljs.highlightAll()");

	printf("evaluate scripts\n");
	// printf("%s", highlightingLib);
	// printf("highlightingLib:\n%s", highlightingLib);
	// char* highlightingLibJavascript = loadScriptContent("javascript.min.js");
	// webview_error_t evalError = webview_eval(w, highlightingLib);
	char* highlightingLib = markview_read_file("prism.min.js");
	// char tempBuffer[33];
	// snprintf(tempBuffer, 32, "alert(%lld)", strlen(highlightingLib));
	webview_error_t evalError = webview_eval(w, highlightingLib);
	if (evalError == WEBVIEW_ERROR_OK) {
		printf("eval ok");
	} else {
		printf("eval error: %d", evalError);
	}

	// webview_eval(w, highlightingLibJavascript);
	// webview_eval(w, "hljs.highlightAll()");
	// webview_eval(w, "alert(1)");
	
	webview_run(w);
	webview_destroy(w);

	if(highlightingLib)
	{
		free(highlightingLib);
	}

	// if(highlightJsLibTheme)
	// {
	// 	free(highlightJsLibTheme);
	// }

	// if(highlightJsLibJavascript)
	// {
	// 	free(highlightJsLibJavascript);
	// }
	
	if (html)
	{
		free(html);
	}

	return 0;
}
