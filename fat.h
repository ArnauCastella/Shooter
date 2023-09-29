//
// Created by arnau on 09/06/2022.
//

#ifndef THESHOOTER_FAT_H
#define THESHOOTER_FAT_H

#include "values.h"

typedef struct {
    char name[8];
    unsigned short size;
    unsigned char sectors_per_cluster;
    unsigned short reserved_sectors;
    unsigned char fats_num;
    unsigned short max_root_entries;
    unsigned short sectors_per_fat;
    char label[11];
} FatInfo;

int checkFat(int fd);
void printFatInfo(FatInfo fatInfo);
FatInfo readFatFile(int fd);
void findFileFat(int fd, char * filename, FatInfo fatInfo, int delete);
void deleteFileFat(int fd, char* filename, FatInfo fatInfo);


#endif //THESHOOTER_FAT_H
