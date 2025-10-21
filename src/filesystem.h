#ifndef MARKVIEW_FILESYSTEM_H
#define MARKVIEW_FILESYSTEM_H

#include <stdbool.h>


bool markview_folder_exists(const char *folderpath);
bool markview_folder_create(const char* folderpath);
bool markview_file_exists(const char* filepath);
char* markview_file_read(const char* filepath);
bool markview_file_write(const char* filenpath, char* content);

#endif // MARKVIEW_FILESYSTEM_H
