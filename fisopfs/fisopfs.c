
#include "fisopfs.h"
#include "bitmap.h"

// El filesystem
static uint8_t blocks[N_BLOCKS * BLOCK_SIZE];

// superblock
superblock_t superblock;

// inode bitmap
word_t __inode_bitmap[N_INODES / BITS_PER_WORD] = { 0 };
bitmap_t inode_bitmap = { .words = __inode_bitmap,
	                  .nwords = N_INODES / BITS_PER_WORD};

// data bitmap
word_t __data_bitmap[N_DATA_BLOCKS / BITS_PER_WORD] = { 0 };
bitmap_t data_bitmap = { .words = __data_bitmap,
	                 .nwords = N_DATA_BLOCKS / BITS_PER_WORD };

// inode_table
inode_t inode_table[N_INODES];

// root inode
inode_t *root;


static inode_t *
get_inode(uint32_t ino)
{
	if (ino >= N_INODES)
		return NULL;

	return (inode_t *) &inode_table[ino];
}

void
print_inode(inode_t *inode)
{
	printf("[debug] Inode Information\n");
	printf("--------------------------------\n");
	printf("[debug] i->ino:				%u\n", inode->ino);
	printf("[debug] i->mode:			%u\n", inode->mode);
	printf("[debug] i->uid;:			%u\n", inode->uid);
	printf("[debug] i->gid:				%u\n", inode->gid);
	printf("[debug] i->size:			%lu\n", inode->size);
	printf("[debug] i->n_blocks:			%u\n", inode->n_blocks);
	printf("[debug] i->atim;:			%lu\n", inode->atim);
	printf("[debug] i->mtim:			%lu\n", inode->mtim);
	printf("[debug] i->ctim:			%lu\n", inode->ctim);
}


static void *
fisopfs_init(struct fuse_conn_info *conn)
{
	printf("[debug] fisopfs init\n");
	printf("[debug] There's %u blocks of %u bytes each\n", N_BLOCKS, BLOCK_SIZE);
	printf("[debug] Inode size is %lu bytes aligned to %u\n",
	       sizeof(inode_t),
	       INODE_SIZE);
	printf("[debug] An inode block contains %u inodes \n", BLOCK_SIZE / INODE_SIZE);
	printf("[debug] There's %u inodes in %u blocks\n", N_INODES, N_INODE_BLOCKS);
	printf("[debug] There's %u data blocks\n", N_DATA_BLOCKS);
	printf("[debug] Data region start block: %u\n", DATA_REGION);



	// initialize root inode
	root = get_inode(ROOT_INODE);

	struct fuse_context *context = fuse_get_context();
	printf("[debug] DESDE INIT, context uid: %d, context gid: %d\n",
				context->uid,
				context->gid);

	root->uid = 1000;
	root->gid = 1000;
	root->ino = ROOT_INODE;
	root->mode = __S_IFDIR | 0777;
	root->ctim = time(NULL);
	root->atim = root->ctim;
	root->mtim = root->ctim;
	root->n_blocks = 0;

	// mark root inode as used in inode bitmap
	set_bit(&inode_bitmap, ROOT_INODE);

	print_inode(root);

	// initialize superblock
	superblock.magic = SUPERBLOCK_MAGIC;
	superblock.n_dirs = 1;  // One dir: root
	superblock.n_files = 0;

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
