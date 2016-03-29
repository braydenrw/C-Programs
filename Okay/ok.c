#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>

void extend(char * str) {
    assert(str != NULL);
    int c = 0;
    c = (int)strlen(str);
    if(c < 15) {
        for(; c < 15; c++) {
            strcat(str, " ");
        }
        strcat(str, "\0");
    }
    assert((int)strlen(str) == 15);
}

void search(int file, char *str) {
    assert(str != NULL);

    int begin, middle, end; //For binary search beginning middle and end
    char line[16];

    begin = 0;
    end = (lseek(file, begin, SEEK_END)/16);
    middle = (begin + end) / 2;
    while (begin <= end) {
        lseek(file, middle*16, SEEK_SET);
        read(file, line, 16);
        line[15] = '\0';
        if (strcmp(line, str) < 0) {
            begin = middle + 1;
        } else if (strcmp(line, str) == 0) {
            printf("yes\n");
            break;
        } else {
            end = middle - 1;
        }
        middle = (begin + end)/2;
    }
    if (begin > end) {
        printf("no\n");
    }
}

int main(int argc, char *argv[]) {
    assert(argc == 2);
    char *str = argv[1]; //Word being passed in,
    const char *fileName = "/encs_share/cs/class/cs360/lib/webster";
    int file = open(fileName, O_RDONLY);
    if(file < 0) {
        fprintf(stderr, "Dictionary not found.");
        exit(1);
    }

    extend(str);
    search(file, str);

    return 0;
}