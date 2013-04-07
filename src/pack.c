#include "winclude.h"

#include "pack.h"
#include "util.h"
#include "main.h"

#if _DEBUG
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "demo.h"

#endif

struct data_t {
	const char * name;
	const char * data;
	unsigned long size;
};

#if _DEBUG
struct data_t * files = NULL;
int num_files = 0;

static void add_file(const char * name, const char * data, unsigned long size) {
	struct data_t entry;
	entry.name = _strdup(name);
	entry.data = data;
	entry.size = size;

	num_files++;

	files = (struct data_t *) realloc(files, sizeof(struct data_t) * num_files);
	files[num_files-1] = entry;
}

#else
#include "data.h"
#endif

#if _DEBUG
char * find_path(const char * name) {
	char * path;
	const char * fmt_shared = "data/shared/%s";
	const char * fmt_special = "data/" DEMO_FOLDER "/%s"; 

	int len = _scprintf(fmt_special	, name) + 1; /* Include null terminator */
	DWORD dwAttrib;

	path = (char*) malloc(len);
	sprintf_s(path, len, fmt_special	, name);

	dwAttrib = GetFileAttributes(path);
	if(dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY)) {
		return path;
	}

	len = _scprintf(fmt_shared, name) + 1; /* Include null terminator */

	path = (char*) malloc(len);
	sprintf_s(path, len, fmt_shared, name);

	dwAttrib = GetFileAttributes(path);
	if(dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY)) {
		return path;
	} else {
		char buffer[128];
		sprintf(buffer, "Could not find file: %s", name);
		MessageBox(NULL, buffer, "File error", MB_OK | MB_ICONEXCLAMATION);
		terminate();
	}
}
#endif

struct file_data_t read_data(const char * name) {
	int i;
	for(i=0; i<num_files; ++i) {
		if(frob_strcmp(files[i].name, name) == 0) {
			struct file_data_t ret = { files[i].data, files[i].size };
			return ret;
		}
	}

#if	_DEBUG
	{
		char * _name = find_path(name);	
		char * data;
		unsigned long size, res;
		FILE * file = fopen(_name, "rb");
		struct file_data_t ret;
		free(_name);

		if(file == NULL) {
			printf("Couldn't open file `%s'\n", name);
			terminate();
		}

		{
			const long cur = ftell(file);
			fseek (file , 0 , SEEK_END);
			size = ftell(file);
			fseek(file, cur, SEEK_SET);
		}

		data = (char*) malloc(size+1);
		data[size] = 0;
		res = fread(data, 1, size, file);
		if ( res != size ) {
			if ( ferror(file) ){
				printf("Error when reading file `%s': %s\n", name, strerror(errno));
			} else {
				printf("Error when reading file `%s': read size was not the expected size (read %lu bytes, expected %lu)\n", name, res, size);
			}
		}
		fclose(file);
		add_file(name, data, size);
		ret.data = data;
		ret.size = size;
		return ret;
	}
#endif
	MessageBox(NULL, "File failure", name, MB_OK | MB_ICONEXCLAMATION);
	terminate(); /* Failed to open file */
}