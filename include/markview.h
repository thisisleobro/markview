#ifndef MARKVIEW_H
#define MARKVIEW_H

typedef void* markview_t;

markview_t markview_create(char* filename);

int markview_run(markview_t app);

// void markview_destroy(markview_t app);

#endif // MARKVIEW_H
