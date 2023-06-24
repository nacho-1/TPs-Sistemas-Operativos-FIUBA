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
#define INODE_SIZE                                                             \
	ALIGNPO2(sizeof(                                                       \
	        inode_t))  // alineado a potencia de 2 para que entren justo en un bloque
#define N_INODES_PER_BLOCK (BLOCK_SIZE / INODE_SIZE)
#define N_DATA_BLOCKS_PER_INODE                                                \
	32  // arbitrario, idealmente que sea potencia de 2
#define N_INODE_BLOCKS                                                         \
	(N_BLOCKS /                                                            \
	 N_INODES_PER_BLOCK)  // al menos un inodo por cada bloque, potencias
	                      // de 2 para que la division no tenga resto
#define N_INODES (N_INODE_BLOCKS * N_INODES_PER_BLOCK)

#define N_DATA_BLOCKS (N_BLOCKS - (3 + N_INODE_BLOCKS))

// Bloques importantes
#define SUPERBLOCK 0
#define INODE_BITMAP 1
#define DATA_BITMAP 2
#define INODE_TABLE 3
#define DATA_REGION (INODE_TABLE + N_INODE_BLOCKS)


static uint8_t blocks[N_BLOCKS * BLOCK_SIZE];  // El filesystem

typedef struct {
	mode_t mode;
	uid_t uid;
	gid_t gid;
	size_t size;
	uint8_t n_blocks;
	unsigned data_blocks[N_DATA_BLOCKS_PER_INODE];
	time_t atim;
	time_t mtim;
	time_t ctim;
} inode_t;

static uint8_t *inode_table =
        &blocks[INODE_TABLE * BLOCK_SIZE];  // shortcut a la tabla de inodos

#endif  // _FISOPFS_H_