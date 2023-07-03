#ifndef _FISOPFS_H_
#define _FISOPFS_H_

#define FUSE_USE_VERSION 30

#include <fuse.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>


#define ALIGN4(s) (((((s) -1) >> 2) << 2) + 4)
#define ALIGNPO2(s) ((s) > 0 ? (1 << (32 - __builtin_clz((s) -1))) : 1)

#define N_BLOCKS 4096    // arbitrario
#define BLOCK_SIZE 4096  // basado en el tamaño de las paginas en x86

// Macros de inodos
#define INODE_SIZE ALIGNPO2(sizeof(inode_t))  // alineado a potencia de 2 para que entren justo en un bloque
#define N_INODES_PER_BLOCK (BLOCK_SIZE / INODE_SIZE)
#define N_DATA_BLOCKS_PER_INODE 32                      // arbitrario, idealmente que sea potencia de 2
#define N_INODE_BLOCKS (N_BLOCKS / N_INODES_PER_BLOCK)  // al menos un inodo por cada bloque
#define N_INODES (N_INODE_BLOCKS * N_INODES_PER_BLOCK)

#define N_DATA_BLOCKS (N_BLOCKS - (3 + N_INODE_BLOCKS))

// Bloques importantes
#define SUPERBLOCK 0
#define INODE_BITMAP 1
#define DATA_BITMAP 2
#define INODE_TABLE 3
#define DATA_REGION (INODE_TABLE + N_INODE_BLOCKS)

#define DENTRY_SIZE ALIGNPO2(sizeof(dirent_t))
#define N_DENTRY_PER_BLOCK (BLOCK_SIZE / DENTRY_SIZE)
#define N_DENTRY_PER_DIR (N_DENTRY_PER_BLOCK * N_DATA_BLOCKS_PER_INODE)
#define ROOT_INODE_NAME "/"

#define FS_FILENAME_LEN 256
#define FS_MAX_PATH 1024
#define MAX_LEVEL 10

typedef struct {
	uint32_t ino;
	mode_t mode;
	uid_t uid;
	gid_t gid;
	size_t size;
	uint8_t n_blocks;
	uint32_t data_blocks[N_DATA_BLOCKS_PER_INODE];
	time_t atim;
	time_t mtim;
	time_t ctim;
	uint32_t parent;
} inode_t;

typedef struct {
	int magic;
	unsigned n_files;
	unsigned n_dirs;
	uint32_t root_ino;
} superblock_t;

typedef struct {
	char d_name[FS_FILENAME_LEN];
	uint32_t d_ino;  // inode number
} dirent_t;

typedef struct {
	char path[FS_FILENAME_LEN];
	char filename[FS_FILENAME_LEN];  // filename used by FUSE filler
	int d_ino;                       // inode number
} file_t;

#endif  // _FISOPFS_H_
