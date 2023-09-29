#include "values.h"
#include "fat.h"
#include "ext.h"

int main(int argc, char * argv[]) {
    char * volume_name;
    char * operation;
    int fd, ext = 0, fat = 0;
    ExtInfo extInfo;
    FatInfo fatInfo;

    volume_name = argv[2];
    operation = argv[1];

    //Check correct number of parameters
    if (strcmp(operation, "/info") == 0) {
        if (argc != 3) {
            printf("\nError: Invalid parameter number.\n\n");
            return -1;
        }
    } else {
        if (argc != 4) {
            printf("\nError: Invalid parameter number.\n\n");
            return -1;
        }
    }

    //Check correct operation
    if (strcmp(operation, "/info") != 0 && strcmp(operation, "/find") != 0 && strcmp(operation, "/delete") != 0) {
        printf("\nError: Wrong operation\n\n");
        return -1;
    }
    fd = open(volume_name, O_RDWR);
    if (fd < 0){
        printf("\nError: Non-existent volume.\n\n");
        return -1;
    }

    if (checkExt(fd)) {
        extInfo = readExtFile(fd);
        ext = 1;
    } else if (checkFat(fd)) {
        fatInfo = readFatFile(fd);
        fat = 1;
    } else {
        printf("\nFile system is neither EXT2 nor FAT16.\n\n");
        return 0;
    }

    if (strcmp(operation, "/info") == 0) {
        if (ext) {
            printExtInfo(extInfo);
        } else if (fat){
            printFatInfo(fatInfo);
        }
    } else if (strcmp(operation, "/find") == 0) {
        if (ext) {
            findFileExt(fd, argv[3], extInfo);
        } else if (fat){
            findFileFat(fd, argv[3], fatInfo, 0);
        }
    } else if (strcmp(operation, "/delete") == 0) {
        if (ext) {
            deleteFileExt(fd, argv[3], extInfo);
        } else if (fat) {
            deleteFileFat(fd, argv[3], fatInfo);
        }
    }

    return 0;
}
