#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>

#define MAX_PATH_LEN 512

void recurse(char *path) {
    char tmp[MAX_PATH_LEN];
    struct stat buffer;
    DIR* FD = opendir(path);

    if(FD == NULL) {
        return;
    }
    
    struct dirent *entry = readdir(FD);

    /* Loop through entries in directory */
    while(entry != NULL) {
        /* Skip "." and ".." */
        if(!(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)) {
            strcpy(tmp, path);
            if(strcmp(tmp, "/") != 0) strcat(tmp, "/"); 
            strcat(tmp, entry->d_name);
            if(access(tmp, R_OK) == 0) { // Make sure file/dir is readable
                if (lstat(tmp, &buffer) == 0) { // Make sure lstat doesn't error return
                    if (S_ISDIR(buffer.st_mode)) { // Check if we're at a directory
                        recurse(tmp);
                    } else if (S_ISREG(buffer.st_mode)) { // Otherwise, if we're at a file
                        printf("%s\n", tmp);
                    }
                }
            }
        }
        entry = readdir(FD);
    }
    closedir(FD);
}

void errorCheck(char *path) {
    assert(path != NULL);

    struct stat s;
    if(lstat(path, &s) == 0) {
        if(!s.st_mode & S_IFDIR) { // Make sure first path is to a directory
            fprintf(stderr, "Not a directory\n");
            exit(2);
        }
    } else { // lstat error'd exit here
        fprintf(stderr, "No access to path\n");
        exit(1);
    }

    if(access(path, R_OK) != 0) { // Make sure there is access to the first directory
        fprintf(stderr, "Can't open first directory\n");
        exit(3);
    }
}

int main(int argc, char **argv) {
    assert(argc <= 2);
    char *path;
    char buff[MAX_PATH_LEN];

    if(argc == 2) {
        path = argv[1];
    } else {
        path = getcwd(buff, MAX_PATH_LEN);
    }

    errorCheck(path);
    recurse(path);
    
    return 0;
}
