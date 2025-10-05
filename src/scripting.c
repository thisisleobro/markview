#include <stddef.h>
#include <webview/api.h>
#include <webview/errors.h>
#include <webview/types.h>
#include <stdbool.h>
#include <markview/filesystem.h>


bool markview_run_script(webview_t webview, char* content) {
	webview_error_t evalError = webview_eval(webview, content);
	if (evalError == WEBVIEW_ERROR_OK) {
		return true;
	}
	return false;
}

bool markview_run_script_from_file(webview_t webview, char* filename) {
	char* content = markview_file_read(filename);
	if (NULL == content) {
		return false;
	}

	return markview_run_script(webview, content);
}
