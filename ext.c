//
// Created by arnau on 09/06/2022.
//

#include "ext.h"

int checkExt(int fd) {
    int type;
    lseek(fd, EXT_SUPERBLOCK_OFFSET + EXT_MAGIC_OFFSET, SEEK_SET);
    read(fd, &type, 2);
    if (type == EXT_MAGIC) {
        return 1;
    }
    return 0;
}

ExtInfo readExtFile(int fd) {
    ExtInfo ext;

    //Read inode
    lseek(fd, EXT_SUPERBLOCK_OFFSET + EXT_INO_SIZE_OFFSET, SEEK_SET);
    read(fd, &(ext.inode.size), EXT_INO_SIZE_SIZE);
    lseek(fd, EXT_SUPERBLOCK_OFFSET + EXT_INO_NUMINODES_OFFSET, SEEK_SET);
    read(fd, &(ext.inode.num_inodes), EXT_INO_NUMINODES_SIZE);
    lseek(fd, EXT_SUPERBLOCK_OFFSET + EXT_INO_FIRSTINODE_OFFSET, SEEK_SET);
    read(fd, &(ext.inode.first_inode), EXT_INO_FIRSTINODE_SIZE);
    lseek(fd, EXT_SUPERBLOCK_OFFSET + EXT_INO_INODESGROUP_OFFSET, SEEK_SET);
    read(fd, &(ext.inode.inodes_group), EXT_INO_INODESGROUP_SIZE);
    lseek(fd, EXT_SUPERBLOCK_OFFSET + EXT_INO_FREEINODES_OFFSET, SEEK_SET);
    read(fd, &(ext.inode.free_inodes), EXT_INO_FREEINODES_SIZE);

    //Read block
    lseek(fd, EXT_SUPERBLOCK_OFFSET + EXT_BLO_SIZE_OFFSET, SEEK_SET);
    read(fd, &(ext.block.size), EXT_BLO_SIZE_SIZE);
    ext.block.size = 1024 << ext.block.size;
    lseek(fd, EXT_SUPERBLOCK_OFFSET + EXT_BLO_RSVBLOCKS_OFFSET, SEEK_SET);
    read(fd, &(ext.block.reserved_blocks), EXT_BLO_RSVBLOCKS_SIZE);
    lseek(fd, EXT_SUPERBLOCK_OFFSET + EXT_BLO_FREEBLOCKS_OFFSET, SEEK_SET);
    read(fd, &(ext.block.free_blocks), EXT_BLO_FREEBLOCKS_SIZE);
    lseek(fd, EXT_SUPERBLOCK_OFFSET + EXT_BLO_TOTALBLOCKS_OFFSET, SEEK_SET);
    read(fd, &(ext.block.total_blocks), EXT_BLO_TOTALBLOCKS_SIZE);
    lseek(fd, EXT_SUPERBLOCK_OFFSET + EXT_BLO_FIRSTBLOCK_OFFSET, SEEK_SET);
    read(fd, &(ext.block.first_block), EXT_BLO_FIRSTBLOCK_SIZE);
    lseek(fd, EXT_SUPERBLOCK_OFFSET + EXT_BLO_GROUPBLOCKS_OFFSET, SEEK_SET);
    read(fd, &(ext.block.group_blocks), EXT_BLO_GROUPBLOCKS_SIZE);

    //Read volume
    lseek(fd, EXT_SUPERBLOCK_OFFSET + EXT_VOL_NAME_OFFSET, SEEK_SET);
    read(fd, &(ext.volume.name), EXT_VOL_NAME_SIZE);
    lseek(fd, EXT_SUPERBLOCK_OFFSET + EXT_VOL_LASTCHECK_OFFSET, SEEK_SET);
    read(fd, &(ext.volume.last_check), EXT_VOL_LASTCHECK_SIZE);
    lseek(fd, EXT_SUPERBLOCK_OFFSET + EXT_VOL_LASTMOUNT_OFFSET, SEEK_SET);
    read(fd, &(ext.volume.last_mount), EXT_VOL_LASTMOUNT_SIZE);
    lseek(fd, EXT_SUPERBLOCK_OFFSET + EXT_VOL_LASTWRITE_OFFSET, SEEK_SET);
    read(fd, &(ext.volume.last_write), EXT_VOL_LASTWRITE_SIZE);

    return ext;
}

char * getDateTime(time_t time) {
    char * buff = malloc (25);
    strftime(buff, 25, "%a %b %d %X %Y", localtime(&time));
    buff[24] = '\0';
    return buff;
}

void printExtInfo(ExtInfo extInfo) {
    printf("\n------ Filesystem Information ------\n\n");
    printf("Filesystem: EXT2\n\n");
    printf("INFO INODE\n");
    printf("Size: %hu\n", extInfo.inode.size);
    printf("Num inodes: %d\n", extInfo.inode.num_inodes);
    printf("First inode: %d\n", extInfo.inode.first_inode);
    printf("Inodes group: %d\n", extInfo.inode.inodes_group);
    printf("Free inodes: %d\n\n", extInfo.inode.free_inodes);
    printf("INFO BLOCK\n");
    printf("Size Block: %d\n", extInfo.block.size);
    printf("Reserved blocks: %d\n", extInfo.block.reserved_blocks);
    printf("Free blocks: %d\n", extInfo.block.free_blocks);
    printf("Total blocks: %d\n", extInfo.block.total_blocks);
    printf("First block: %d\n", extInfo.block.first_block);
    printf("Group blocks: %d\n\n", extInfo.block.group_blocks);
    printf("INFO VOLUME\n");
    printf("Volume Name: %s\n", extInfo.volume.name);
    char * aux = getDateTime(extInfo.volume.last_check);
    printf("Last Checked: %s\n", aux);
    free(aux);
    aux = getDateTime(extInfo.volume.last_mount);
    printf("Last Mounted: %s\n", aux);
    free(aux);
    aux = getDateTime(extInfo.volume.last_write);
    printf("Last Written: %s\n\n", aux);
    free(aux);
}

InodeTable getInodeTable(int fd, ExtInfo extInfo, int num_inode) {
    InodeTable inode_table;

    //Find block id of inode table in group descriptor
    unsigned int inode_block;
    lseek(fd, EXT_SUPERBLOCK_OFFSET + EXT_SUPERBLOCK_SIZE + EXT_GDT_INODETABLE_OFFSET, SEEK_SET);
    read(fd, &inode_block, EXT_GDT_INODETABLE_SIZE);

    //Find inode position
    unsigned int block_group = (num_inode - 1) / extInfo.inode.inodes_group;
    unsigned int inode_index = (num_inode - 1) % extInfo.inode.inodes_group;
    unsigned int group_offset = block_group * extInfo.block.group_blocks * extInfo.block.size;
    unsigned int inode_pos = inode_block * extInfo.block.size + inode_index * extInfo.inode.size + group_offset;

    lseek(fd, inode_pos, SEEK_SET);
    read(fd, &inode_table, sizeof(inode_table));
    return inode_table;
}

int readDirEntry(int fd, DirEntry * dirEntry, int block_pos, int * size, int max_size) {
    unsigned int inode;
    lseek(fd, block_pos + *size, SEEK_SET);
    read(fd, &inode, 4);
    if (inode == 0) return -1; //If inode is 0 it is not used
    unsigned short rec_len;
    read(fd, &rec_len, 2);
    if (rec_len + *size > max_size) return -1;
    char name_len;
    read(fd, &name_len, 1);
    lseek(fd, block_pos + *size, SEEK_SET);
    read(fd, dirEntry, sizeof(DirEntry) - EXT_FILE_NAME_LEN + name_len);
    *size += rec_len;
    return 1;
}

int recursiveSearch(int fd, char * filename, int inode, ExtInfo extInfo, int delete) {
    DirEntry dirEntry;
    int size = 0;

    InodeTable inode_table = getInodeTable(fd, extInfo, inode);

    for (int i = 0; i < EXT_NUM_BLOCKS && size < inode_table.size; i++) {
        unsigned int first_dir_pos = inode_table.blocks[i] * extInfo.block.size;

        for (int j = 0; j < EXT_NUM_DIR_ENTRIES; j++) {
            if (readDirEntry(fd, &dirEntry, first_dir_pos, &size, inode_table.size) == -1) break;
            if (strcmp(filename, dirEntry.name) == 0) {
                inode_table = getInodeTable(fd, extInfo, dirEntry.inode);
                if (delete) {
                    unsigned int aux = 0;
                    lseek(fd, first_dir_pos, SEEK_SET);
                    write(fd, &aux, 4);
                }
                return inode_table.size;
            }
            if (dirEntry.file_type == EXT_DIR_FILETYPE && strcmp(dirEntry.name, ".") != 0 &&
                strcmp(dirEntry.name, "..") != 0) {
                int new_size = recursiveSearch(fd, filename, dirEntry.inode, extInfo, delete);
                if (new_size != 0) return new_size;
            }
        }
    }
    return 0;
}

void findFileExt(int fd, char * filename, ExtInfo extInfo) {
    int size = recursiveSearch(fd, filename, EXT_ROOT_INODE, extInfo, 0);
    if (size <= 0) {
        printf("\nError. File not found.\n\n");
    } else {
        printf("\nFile found. It occupies %d bytes.\n\n", size);
    }
}

void deleteFileExt(int fd, char * filename, ExtInfo extInfo) {
    int size = recursiveSearch(fd, filename, EXT_ROOT_INODE, extInfo, 1);
    if (size <= 0) {
        printf("\nError. File not found.\n\n");
    } else {
        printf("\nThe file %s has been deleted.\n\n", filename);
    }
}