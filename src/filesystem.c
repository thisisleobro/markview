#include <windows.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>


// #include <dirent.h>
// #include <stdbool.h>
// int markview_folder_exists_linux_impl(const char* folderpath) {
//     DIR *dir = opendir(folderpath);
//     if (dir) {
//         closedir(dir);
//         return true;
//     }
//     return false;
// }


// TODO: make cross platform
bool markview_folder_exists(const char* folderpath) {
	DWORD dwAttrib = GetFileAttributesA(folderpath);

    if (dwAttrib == INVALID_FILE_ATTRIBUTES) {
        return false;
    }

    if (dwAttrib & FILE_ATTRIBUTE_DIRECTORY) {
        return true;
    }

    return false;
}

bool markview_file_exists(const char* filepath) {
	FILE *file = fopen(filepath, "r");
	if (file) {
		fclose(file);
		return true;
	}

	return false;
}

char* markview_read_file(const char* filename) {
	// fopen_s(FILE **Stream, const char *FileName, const char *Mode)
	FILE* file = fopen(filename, "r");
	if (!file) {
		perror("Failed to open file");
		return NULL;
	}

	if (fseek(file, 0, SEEK_END) != 0) {
		perror("Failed to seek to end of file");
		fclose(file);
		return NULL;
	}

	long size = ftell(file);
	if (size == -1) {
		perror("Failed to get file size");
		fclose(file);
		return NULL;
	}

	rewind(file);

	if (size == 0) {
		fclose(file);
		return "";
	}

	size_t buffer_size = 1024;
	char* content = malloc(buffer_size);
	if (!content) {
		perror("Failed to allocate memory");
		fclose(file);
		return NULL;
	}

	size_t total_read = 0;
	size_t bytes_read;

	while ((bytes_read = fread(content + total_read, 1, buffer_size - total_read, file)) > 0) {
		total_read += bytes_read;

		if (total_read >= buffer_size) {
			buffer_size *= 2;  // Double the buffer size
			char* temp = realloc(content, buffer_size);
			if (!temp) {
				perror("Failed to reallocate memory");
				free(content);
				fclose(file);
				return NULL;
			}
			content = temp;
		}
	}

	if (bytes_read == 0 && ferror(file)) {
		perror("Failed to read file");
		free(content);
		fclose(file);
		return NULL;
	}

	content[total_read] = '\0';
	fclose(file);
	return content;
}
