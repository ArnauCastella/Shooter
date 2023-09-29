//
// Created by arnau on 09/06/2022.
//

#ifndef THESHOOTER_EXT_H
#define THESHOOTER_EXT_H

#include "values.h"

typedef struct {
    unsigned short size; //2 bytes
    unsigned int num_inodes; //4 bytes
    unsigned int first_inode;
    unsigned int inodes_group;
    unsigned int free_inodes;
}InfoInode;

typedef struct {
    unsigned int size;
    unsigned int reserved_blocks;
    unsigned int free_blocks;
    unsigned int total_blocks;
    unsigned int first_block;
    unsigned int group_blocks;
}InfoBlock;

typedef struct {
    char name[16];
    unsigned int last_check;
    unsigned int last_mount;
    unsigned int last_write;
}InfoVolume;

typedef struct {
    InfoInode inode;
    InfoBlock block;
    InfoVolume volume;
}ExtInfo;

typedef struct {
    char nothing[4];
    unsigned int size;
    char nothing2[32];
    unsigned int blocks[15];
    char nothing3[28];
}InodeTable;

typedef struct {
    int inode;
    short rec_len;
    char name_len;
    char file_type;
    char name[EXT_FILE_NAME_LEN];
}DirEntry;

int checkExt(int fd);
void printExtInfo(ExtInfo extInfo);
ExtInfo readExtFile(int fd);
void findFileExt(int fd, char * filename, ExtInfo extInfo);
void deleteFileExt(int fd, char * filename, ExtInfo extInfo);

#endif //THESHOOTER_EXT_H
