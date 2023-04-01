#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>

#define FLAG "-i"
#define MAX_PATH_NAME_LENGTH 128

typedef char string[MAX_PATH_NAME_LENGTH];
typedef char *(*function_ptr)(const char *, const char *);

DIR *open_subdir(DIR *dir_ptr, string subdir);
DIR *
open_subdir(DIR *dir_ptr, string subdir)
{
	int dir_fd = dirfd(dir_ptr);
	if (dir_fd == -1) {
		perror("Failed to get file descriptor");
		return NULL;
	}
	int subdir_fd = openat(dir_fd, subdir, O_DIRECTORY);
	if (subdir_fd == -1) {
		perror("Failed to open sub-directory");
		return NULL;
	}
	DIR *subdir_ptr = fdopendir(subdir_fd);
	if (subdir_ptr == NULL) {
		perror("Failed to assign FD to directory");
		close(subdir_fd);
		return NULL;
	}
	return subdir_ptr;
}

void find_in_dir(DIR *dir_ptr,
                 string current_path,
                 string searchterm,
                 function_ptr strfind);
void
find_in_dir(DIR *dir_ptr, string current_path, string searchterm, function_ptr strfind)
{
	struct dirent *dent;
	while ((dent = readdir(dir_ptr)) != NULL) {
		if (strcmp(dent->d_name, ".") == 0 ||
		    strcmp(dent->d_name, "..") == 0)
			continue;

		string path;
		strcpy(path, current_path);
		strcat(path, dent->d_name);
		if ((*strfind)(dent->d_name, searchterm) != NULL)
			printf("%s\n", path);

		if (dent->d_type == DT_DIR) {
			strcat(path, "/");
			DIR *subdir_ptr = open_subdir(dir_ptr, dent->d_name);
			if (subdir_ptr != NULL) {
				find_in_dir(subdir_ptr, path, searchterm, strfind);
				closedir(subdir_ptr);
			}
		}
	}
}

int
main(int argc, char *argv[])
{
	if (argc != 2 && argc != 3) {
		fprintf(stderr, "Invalid input\n");
		_exit(EXIT_FAILURE);
	}

	function_ptr strfind = strstr;

	if (argc == 3) {
		if (strcmp(argv[1], FLAG) == 0) {
			strfind = strcasestr;
		} else {
			fprintf(stderr, "Invalid flag\n");
			_exit(EXIT_FAILURE);
		}
	}

	string searchterm;
	strcpy(searchterm, argv[argc - 1]);

	DIR *dir_ptr = opendir(".");
	if (dir_ptr == NULL) {
		perror("Failed to open current directory");
		_exit(EXIT_FAILURE);
	}
	string currentpath;
	strcpy(currentpath, "");
	find_in_dir(dir_ptr, currentpath, searchterm, strfind);
	closedir(dir_ptr);

	exit(EXIT_SUCCESS);
}
