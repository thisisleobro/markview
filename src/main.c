#include <stdio.h>
#include "markview.h"
#include "welcome_md.h"


int main(int argc, char** argv) {
	markview_t app = markview_create();

	if (argc > 1) {
		markview_render_from_file(app, argv[1], argv[1]);
	} else {
		markview_render_from_string(app, "Welcome", (char*)welcome_md_data);
	}

	if (NULL == app) {
		fprintf(stderr,"Error creating app");
		return -1;
	}

	return markview_run(app);
}
