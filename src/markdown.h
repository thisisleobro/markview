#ifndef MARKVIEW_MARKDOWN_H
#define MARKVIEW_MARKDOWN_H

#include <cmark-gfm.h>

void register_extension_by_name(cmark_parser* parser, cmark_llist* list, char* name);

char* markview_markdown_to_html(char* markdown, size_t lenght, int cmark_option);

#endif // MARKVIEW_MARKDOWN_H