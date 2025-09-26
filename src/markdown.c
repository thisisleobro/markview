#include <cmark-gfm.h>
#include <cmark_ctype.h>
#include <cmark-gfm_version.h>
#include <cmark-gfm_export.h>
#include <cmark-gfm-extension_api.h>
#include <cmark-gfm-core-extensions.h>
#include <markview/markdown.h>



void register_extension_by_name(cmark_parser* parser, cmark_llist* list, char* name) {
	cmark_syntax_extension* extension = cmark_find_syntax_extension(name);

	cmark_llist_append(cmark_get_default_mem_allocator(), list, extension);

	cmark_parser_attach_syntax_extension(parser, extension);
}

char* markdown_to_html(char* markdown, size_t lenght, int cmark_option) {
	// https://medium.com/@krisgbaker/using-cmark-gfm-extensions-aad759894a89
	char* gfm_extension_names[] = {"autolink", "table", "strikethrough", "tasklist", "tagfilter"};
	size_t number_of_extensions = sizeof(gfm_extension_names)/sizeof(gfm_extension_names[0]);

	cmark_parser* parser = cmark_parser_new(cmark_option);
	cmark_llist* extension_list = NULL;

	cmark_gfm_core_extensions_ensure_registered();

	for (int i = 0; i < number_of_extensions; ++i)
	{
		register_extension_by_name(parser, extension_list, gfm_extension_names[i]);
	}

	cmark_parser_feed(parser, markdown, lenght);

	cmark_node* doc = cmark_parser_finish(parser);

	char* html = cmark_render_html(doc, cmark_option, extension_list);

	cmark_llist_free(cmark_get_default_mem_allocator(), extension_list);
	cmark_parser_free(parser);
	cmark_node_free(doc);

	return html;
}
