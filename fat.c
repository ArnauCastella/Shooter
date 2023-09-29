//
// Created by arnau on 09/06/2022.
//

#include "fat.h"

int checkFat(int fd) {
    char type[8];
    lseek(fd, 54, SEEK_SET);
    read(fd, type, 8);
    if (strncmp(type, "FAT16", 5) == 0) {
        return 1;
    }
    return 0;
}

FatInfo readFatFile(int fd) {
    FatInfo fat;
    lseek(fd, FAT_NAME_OFFSET, SEEK_SET);
    read(fd, &(fat.name), FAT_NAME_SIZE);
    lseek(fd, FAT_SIZE_OFFSET, SEEK_SET);
    read(fd, &(fat.size), FAT_SIZE_SIZE);
    lseek(fd, FAT_SECTORSCLUSTER_OFFSET, SEEK_SET);
    read(fd, &(fat.sectors_per_cluster), FAT_SECTORSCLUSTER_SIZE);
    lseek(fd, FAT_RESERVEDSECTORS_OFFSET, SEEK_SET);
    read(fd, &(fat.reserved_sectors), FAT_RESERVEDSECTORS_SIZE);
    lseek(fd, FAT_NUMFATS_OFFSET, SEEK_SET);
    read(fd, &(fat.fats_num), FAT_NUMFATS_SIZE);
    lseek(fd, FAT_MAXROOTENTRIES_OFFSET, SEEK_SET);
    read(fd, &(fat.max_root_entries), FAT_MAXROOTENTRIES_SIZE);
    lseek(fd, FAT_SECTORSFAT_OFFSET, SEEK_SET);
    read(fd, &(fat.sectors_per_fat), FAT_SECTORSFAT_SIZE);
    lseek(fd, FAT_LABEL_OFFSET, SEEK_SET);
    read(fd, &(fat.label), FAT_LABEL_SIZE);

    return fat;
}

void printFatInfo(FatInfo fatInfo) {
    printf("\n------ Filesystem Information ------\n\n");
    printf("Filesystem: FAT16\n");
    printf("System name: %s\n", fatInfo.name);
    printf("Size: %hu\n", fatInfo.size);
    printf("Sectors per Cluster: %u\n", fatInfo.sectors_per_cluster);
    printf("Reserved sectors: %hu\n", fatInfo.reserved_sectors);
    printf("Number of FATs: %hu\n", fatInfo.fats_num);
    printf("MaxRootEntries: %hu\n", fatInfo.max_root_entries);
    printf("Sectors per FAT: %hu\n", fatInfo.sectors_per_fat);
    printf("Label: %s\n\n", fatInfo.label);
}

int isDirectory(int fd, unsigned int address) {
    unsigned char attr;
    lseek(fd, address, SEEK_SET);
    read(fd, &attr, 1);
    return (attr == 0x10);
}

int isFile(int fd, unsigned int address) {
    unsigned char attr;
    lseek(fd, address, SEEK_SET);
    read(fd, &attr, 1);
    return (attr == 0x20);
}

char * cleanFilename(char * filename) {
    char * new_filename = (char * ) malloc (FAT_FILENAME_SIZE+2); //Size + . + \0
    int j = 0;

    for (int i = 0; i < 8 && filename[i] != ' '; i++) {
        new_filename[j++] = filename[i];
    }
    if (filename[8] != ' ') {
        new_filename[j++] = '.';
        for (int i = 8; i < 11 && filename[i] != ' '; i++) {
            new_filename[j++] = filename[i];
        }
    }
    filename[j] = '\0';
    return new_filename;
}

int findFromAddress(int fd, char * filename, FatInfo fatInfo, unsigned int address, int delete) {
    char new_filename[FAT_FILENAME_SIZE+1];
    int found = 0;

    for (int i = 0; i < fatInfo.max_root_entries; i++) {
        lseek(fd, address + (i * FAT_DIR_ENTRY_SIZE), SEEK_SET);
        read(fd, &new_filename, FAT_FILENAME_SIZE);
        new_filename[FAT_FILENAME_SIZE] = '\0';
        if (new_filename[0] == 0x00) break;
        char * clean_filename = cleanFilename(new_filename);

        if (isFile(fd, address + (i * FAT_DIR_ENTRY_SIZE) + FAT_DIR_ENTRY_ATTR_OFFSET) &&
            strcmp(filename, clean_filename) == 0) {
            unsigned int size;
            lseek(fd, address + (i * FAT_DIR_ENTRY_SIZE) + FAT_DIR_ENTRY_SIZE_OFFSET, SEEK_SET);
            read(fd, &size, FAT_DIR_ENTRY_SIZE_SIZE);
            if (!delete) {
                printf("\nFile found. It occupies %u bytes.\n\n", size);
            } else {
                //Delete file
                lseek(fd, address + (i * FAT_DIR_ENTRY_SIZE), SEEK_SET);
                char buffer = 0xE5;
                write(fd, &buffer, 1);
                printf("\nThe file %s has been deleted.\n\n", filename);
            }
            found = 1;
            break;
        }
        if (isDirectory(fd, address + (i * FAT_DIR_ENTRY_SIZE) + FAT_DIR_ENTRY_ATTR_OFFSET)
            && strcmp(clean_filename, ".") != 0 && strcmp(clean_filename, "..") != 0) {
            unsigned short new_cluster;
            lseek(fd, address + (i * FAT_DIR_ENTRY_SIZE) + FAT_DIR_ENTRY_CLUSTER_OFFSET, SEEK_SET);
            read(fd, &new_cluster, FAT_DIR_ENTRY_CLUSTER_SIZE);
            unsigned int first_cluster_sec = (new_cluster-2) * fatInfo.sectors_per_cluster *
                    fatInfo.size;
            unsigned int new_address = (fatInfo.reserved_sectors * fatInfo.size) +
                    (fatInfo.fats_num * fatInfo.sectors_per_fat * fatInfo.size) +
                    (fatInfo.max_root_entries * FAT_DIR_ENTRY_SIZE) + first_cluster_sec;
            found = findFromAddress(fd, filename, fatInfo, new_address, 0);
            if (found) break;
        }
    }
    return found;
}

void findFileFat(int fd, char * filename, FatInfo fatInfo, int delete) {
    int found = 0;
    unsigned int first_root_sec_num = fatInfo.reserved_sectors +
            (fatInfo.fats_num * fatInfo.sectors_per_fat);
    found = findFromAddress(fd, filename, fatInfo, first_root_sec_num * fatInfo.size, delete);
    if (!found) printf("\nError. File not found.\n\n");
}

void deleteFileFat(int fd, char * filename, FatInfo fatInfo) {
    findFileFat(fd, filename, fatInfo, 1);
}

