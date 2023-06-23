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
#define ALIGNPO2(s) ((s) > 0 ? (1 << (32 - __builtin_clz((s) - 1))) : 1)

#define N_BLOCKS 4096 // arbitrario
#define BLOCK_SIZE 4096 // basado en el tamaño de las paginas en x86

// Macros de inodos
#define INODE_SIZE ALIGNPO2(sizeof(inode_t)) // alineado a potencia de 2 para que entren justo en un bloque
#define N_INODES_PER_BLOCK (BLOCK_SIZE / INODE_SIZE)
#define N_DATA_BLOCKS_PER_INODE 32 // arbitrario, idealmente que sea potencia de 2
#define N_INODE_BLOCKS (N_BLOCKS / N_INODES_PER_BLOCK) // al menos un inodo por cada bloque, potencias de 2 para que la division no tenga resto
#define N_INODES (N_INODE_BLOCKS * N_INODES_PER_BLOCK)

#define N_DATA_BLOCKS (N_BLOCKS - (3 + N_INODE_BLOCKS))

// Bloques importantes
#define SUPERBLOCK 0
#define INODE_BITMAP 1
#define DATA_BITMAP 2
#define INODE_TABLE 3
#define DATA_REGION (INODE_TABLE + N_INODE_BLOCKS)


static uint8_t blocks[N_BLOCKS * BLOCK_SIZE]; // El filesystem

static struct inode {
	mode_t mode;
	uid_t uid;
	gid_t gid;
	size_t size;
	uint8_t n_blocks;
	unsigned data_blocks[N_DATA_BLOCKS_PER_INODE];
	time_t atim;
	time_t mtim;
	time_t ctim;
};
typedef struct inode inode_t;
static uint8_t *inode_table = &blocks[INODE_TABLE * BLOCK_SIZE]; // shortcut a la tabla de inodos

static void *
fisopfs_init(struct fuse_conn_info *conn)
{
	printf("[debug] fisopfs init\n");
	printf("[debug] There's %u blocks of %u bytes each\n", N_BLOCKS, BLOCK_SIZE);
	printf("[debug] Inode size is %lu bytes aligned to %u\n", sizeof(inode_t), INODE_SIZE);
	printf("[debug] There's %u inodes in %u blocks\n", N_INODES, N_INODE_BLOCKS);
	printf("[debug] There's %u data blocks\n", N_DATA_BLOCKS);
	printf("[debug] Data region start block: %u\n", DATA_REGION);

	return NULL;
}

static int
fisopfs_getattr(const char *path, struct stat *st)
{
	printf("[debug] fisopfs_getattr - path: %s\n", path);

	if (strcmp(path, "/") == 0) {
		st->st_uid = 1717;
		st->st_mode = __S_IFDIR | 0755;
		st->st_nlink = 2;
	} else if (strcmp(path, "/fisop") == 0) {
		st->st_uid = 1818;
		st->st_mode = __S_IFREG | 0644;
		st->st_size = 2048;
		st->st_nlink = 1;
	} else {
		return -ENOENT;
	}

	return 0;
}

static int
fisopfs_readdir(const char *path,
                void *buffer,
                fuse_fill_dir_t filler,
                off_t offset,
                struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_readdir - path: %s\n", path);

	// Los directorios '.' y '..'
	filler(buffer, ".", NULL, 0);
	filler(buffer, "..", NULL, 0);

	// Si nos preguntan por el directorio raiz, solo tenemos un archivo
	if (strcmp(path, "/") == 0) {
		filler(buffer, "fisop", NULL, 0);
		return 0;
	}

	return -ENOENT;
}

#define MAX_CONTENIDO 100
static char fisop_file_contenidos[MAX_CONTENIDO] = "hola fisopfs!\n";

static int
fisopfs_read(const char *path,
             char *buffer,
             size_t size,
             off_t offset,
             struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_read - path: %s, offset: %lu, size: %lu\n",
	       path,
	       offset,
	       size);

	// Solo tenemos un archivo hardcodeado!
	if (strcmp(path, "/fisop") != 0)
		return -ENOENT;

	if (offset + size > strlen(fisop_file_contenidos))
		size = strlen(fisop_file_contenidos) - offset;

	size = size > 0 ? size : 0;

	strncpy(buffer, fisop_file_contenidos + offset, size);

	return size;
}

static struct fuse_operations operations = {
	.init = fisopfs_init,
	.getattr = fisopfs_getattr,
	.readdir = fisopfs_readdir,
	.read = fisopfs_read,
};

int
main(int argc, char *argv[])
{
	return fuse_main(argc, argv, &operations, NULL);
}
