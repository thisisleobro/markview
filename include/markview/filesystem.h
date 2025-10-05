#ifndef MARKVIEW_FILESYSTEM_H
#define MARKVIEW_FILESYSTEM_H

#include <stdbool.h>


bool markview_folder_exists(const char *folder_path);
bool markview_file_exists(const char* filepath);
char* markview_file_read(const char* filepath);

#endif // MARKVIEW_FILESYSTEM_H
