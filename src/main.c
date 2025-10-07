#include <stdio.h>
#include <markview.h>

int main(int argc, char** argv) {
	markview_t app = markview_create(argv[1]);

	if (NULL == app) {
		fprintf(stderr,"Error creating app");
		return -1;
	}

	return markview_run(app);
}
