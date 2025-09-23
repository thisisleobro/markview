#include <stdio.h>
#include <stdlib.h>
#include <webview/api.h>
#include <webview/webview.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <cmark.h>
#include <string.h>

#define PROGRAM_NAME "markview"
#define TITLE_MAX_SIZE 256

// #ifdef _WIN32
// #include <windows.h>
// #endif

char* read_file(const char* filename) {
    FILE *file = NULL;
    errno_t err = fopen_s(&file, filename, "rb");
    if (err != 0 || file == NULL) {
        perror("Failed to open file");
        return NULL;
    }

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

char* loadScriptContent(char* filename)
{
	return read_file(filename);
}

int main(int argc, char** argv)
{
	char* filename = argv[1];
	char* html = NULL;
	char windowTitle[TITLE_MAX_SIZE];

	if (filename)
	{
		// 1. Load the markdow file
		char* markdown = read_file(argv[1]);
		if (!markdown) {
			return 1;
		}

		// 2. Parse markdown to html
		char* rawHtml = cmark_markdown_to_html(markdown, strlen(markdown), CMARK_OPT_DEFAULT);

		char* highlightJsLibTheme = loadScriptContent("default.min.css");

		char styleTag[] = "<style>%s</style>";
		char* styles = malloc(strlen(styleTag) + strlen(highlightJsLibTheme) + 1);
		snprintf(styles, strlen(styleTag) + strlen(highlightJsLibTheme) + 1, styleTag, highlightJsLibTheme);

		// printf("styles: %s", styles);

		size_t htmlSize = strlen(rawHtml) + strlen(styles) + 1;

		html = malloc(htmlSize);

		snprintf(html, htmlSize, "%s\n%s", styles, rawHtml);

		printf("%s", html);

		if (styles)
		{
			free(styles);
		}

		if (markdown)
		{
			free(markdown);
		}

	}

	snprintf(windowTitle, TITLE_MAX_SIZE - 1, "%s -  %s", PROGRAM_NAME, filename);

	printf("html: %s\n", html);

	// 3. webview_set_html(w, the_html);
	webview_t w = webview_create(1, NULL);
	webview_set_title(w, windowTitle);
	// webview_set_size(w, 480, 320, WEBVIEW_HINT_NONE);
	webview_set_html(w, html);

	// webview_init(w, "hljs.highlightAll()");


	printf("evaluate scripts\n");
	char* highlightJsLib = loadScriptContent("highlight.min.js");
	char* highlightJsLibJavascript = loadScriptContent("javascript.min.js");

	webview_eval(w, highlightJsLib);
	webview_eval(w, highlightJsLibJavascript);
	webview_eval(w, "hljs.highlightAll()");
	// webview_eval(w, "alert(1)");
	
	webview_run(w);
	webview_destroy(w);

	if(highlightJsLib)
	{
		free(highlightJsLib);
	}

	// if(highlightJsLibTheme)
	// {
	// 	free(highlightJsLibTheme);
	// }

	if(highlightJsLibJavascript)
	{
		free(highlightJsLibJavascript);
	}
	
	if (html)
	{
		free(html);
	}

	return 0;
}
