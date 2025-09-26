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

#define PROGRAM_NAME "markview"
#define TITLE_MAX_SIZE 256


// #ifdef _WIN32
// #include <windows.h>
// #endif

char* read_file(const char* filename) {
    FILE *file = fopen(filename, "r");

    // Seek to end to find file size
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);

    // Allocate buffer (+1 for null terminator)
    char *content = malloc(size + 1);
    if (!content) {
        perror("Failed to allocate memory");
        fclose(file);
        return NULL;
    }

    fread(content, 1, size, file);
    content[size] = '\0';  // Null-terminate the string
    fclose(file);
    return content;
}

char* loadContentFromDisk(char* filename)
{
	return read_file(filename);
}

int main(int argc, char** argv)
{
	char* html = NULL;
	char windowTitle[TITLE_MAX_SIZE];


	if (argc > 1)
	{
		char* filename = argv[1];
		// 1. Load the markdow file
		char* markdown = read_file(filename);
		if (!markdown) {
			return 1;
		}

		// 2. Parse markdown to html
		// https://medium.com/@krisgbaker/using-cmark-gfm-extensions-aad759894a89
		cmark_gfm_core_extensions_ensure_registered();
		cmark_parser* parser = cmark_parser_new(0);
		cmark_syntax_extension* se = cmark_find_syntax_extension("table");

		cmark_llist* list = NULL;

		cmark_llist_append(cmark_get_default_mem_allocator(), list, se);

		cmark_parser_attach_syntax_extension(parser, se);

		cmark_parser_feed(parser, markdown, strlen(markdown));

		cmark_node* node = cmark_parser_finish(parser);

		char* rawHtml = cmark_render_html(node, 0, list);

		// char* rawHtml = cmark_markdown_to_html(markdown, strlen(markdown), CMARK_OPT_SMART);

		char* highlightJsLibTheme = loadContentFromDisk("prism.css");

		char styleTag[] = "<style>%s</style>";
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

	printf("html: %s\n", html);

	// 3. webview_set_html(w, the_html);
	webview_t w = webview_create(1, NULL);
	webview_set_title(w, windowTitle);
	// webview_set_size(w, 480, 320, WEBVIEW_HINT_NONE);
	webview_set_html(w, html);
	// webview_init(w, "hljs.highlightAll()");

	printf("evaluate scripts\n");
	char* highlightingLib = loadContentFromDisk("prism.min.js");
	// printf("highlightingLib:\n%s", highlightingLib);
	// char* highlightingLibJavascript = loadScriptContent("javascript.min.js");
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
