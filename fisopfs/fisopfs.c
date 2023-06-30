#include "fisopfs.h"
#include "bitmap.h"

#include <assert.h>

#define END_STRING '\0'
#define DELIM_CHAR '/'

// El filesystem

// superblock
superblock_t superblock;

// inode bitmap
word_t __inode_bitmap[N_INODES / BITS_PER_WORD] = { 0 };
bitmap_t inode_bitmap = { .words = __inode_bitmap,
	                  .nwords = N_INODES / BITS_PER_WORD };

// data bitmap
word_t __data_bitmap[N_DATA_BLOCKS / BITS_PER_WORD] = { 0 };
bitmap_t data_bitmap = { .words = __data_bitmap,
	                 .nwords = N_DATA_BLOCKS / BITS_PER_WORD };

// inode_table
inode_t inodes[N_INODES];

// data blocks
static uint8_t data_blocks[N_DATA_BLOCKS * BLOCK_SIZE];

// Particionar al string src, dividiéndolo segun el delimitador
// delim y guardar el resultado en dest. Se devuelve la cantidad
// de elementos de dest.
static unsigned
strsplit(const char *src, char *dest[], char delim)
{
	unsigned copystart = 0, copysize = 0, argcount = 0;

	for (int i = 0; src[i] != END_STRING; i++) {
		if (src[i] == delim) {
			if (copysize > 0) {
				dest[argcount] = malloc(copysize + 1);  // string + \0
				memcpy(dest[argcount], src + copystart, copysize);
				dest[argcount][copysize] = '\0';
				copysize = 0;
				argcount++;
			}
			copystart = i + 1;
		} else {
			copysize++;
		}
	}
	if (copysize > 0) {
		// Si el ultimo caracter de src no es delim, agregar la ultima particion
		dest[argcount] = malloc(copysize + 1);
		memcpy(dest[argcount], src + copystart, copysize);
		dest[argcount][copysize] = '\0';
		argcount++;
	}

	return argcount;
}

static void
fill_stat(struct stat *st, inode_t *inode)
{
	st->st_mode = inode->mode;
	st->st_ino = inode->ino;
	st->st_gid = inode->gid;
	st->st_uid = inode->uid;
	inode->atim = time(NULL);
	st->st_atime = inode->atim;
	st->st_mtime = inode->mtim;
	st->st_ctime = inode->ctim;
	st->st_blocks = inode->n_blocks;
}

static inode_t *
get_inode(uint32_t ino)
{
	if (ino >= N_INODES)
		return NULL;

	return (inode_t *) &inodes[ino];
}

static uint8_t *
get_data_block(uint32_t block_no)
{
	if (block_no >= N_DATA_BLOCKS)
		return NULL;

	return &data_blocks[block_no * BLOCK_SIZE];
}

/*
 * Devuelve un puntero a la dentry numero dentry_no
 * de un directorio asociado al inodo dir_inode
 * Los numeros de dentry empiezan en 0
 * Util para iterar las dentries de un directorio.
 */
static dirent_t *
get_dirent(unsigned dentry_no, inode_t *dir_inode)
{
	if (!S_ISDIR(dir_inode->mode))
		return NULL;

	unsigned block_no = dentry_no / N_DENTRY_PER_BLOCK;
	if (block_no >= dir_inode->n_blocks)
		return NULL;

	unsigned offset = dentry_no % N_DENTRY_PER_BLOCK;
	uint8_t *block = get_data_block(dir_inode->data_blocks[block_no]);
	return (dirent_t *) &block[offset * DENTRY_SIZE];
}

/*
 * Devuelve el puntero al inodo de un dentry
 * en el directorio asociado a dir_inode.
 * Si dir_inode no corresponde a un directorio
 * o no se puede encontrar la entrada d_name
 * devuelve null.
 */
static inode_t *
find_in_dir(const char *d_name, inode_t *dir_inode)
{
	if (!S_ISDIR(dir_inode->mode))
		return NULL;

	unsigned n_dentries = dir_inode->size / DENTRY_SIZE;
	for (unsigned i = 0; i < n_dentries; i++) {
		dirent_t *dentry = get_dirent(i, dir_inode);
		if (strcmp(d_name, dentry->d_name) == 0)
			return get_inode(dentry->d_ino);
	}
	// No lo pudo encontrar
	return NULL;
}

/*
 * Devuelve el puntero al inodo correspondiente a un path.
 * Devuelve NULL si no pudo encontrarlo.
 */
static inode_t *
find_inode(const char *path)
{
	inode_t *curr_dir = get_inode(superblock.root_ino);
	assert(curr_dir != NULL);
	assert(S_ISDIR(curr_dir->mode));

	if (strcmp(path, ROOT_INODE_NAME) == 0)
		return curr_dir;

	unsigned max_tokens = strlen(path) / 2;  // puede haber este máximo de tokens
	char **tokens = malloc(max_tokens * sizeof(char *));
	unsigned path_level = strsplit(path, tokens, DELIM_CHAR);

	for (unsigned i = 0; i < path_level; i++) {
		curr_dir = find_in_dir(tokens[i], curr_dir);
		if (curr_dir == NULL)
			break;
	}

	for (unsigned i = 0; i < path_level; i++)
		free(tokens[i]);
	free(tokens);

	return curr_dir;
}

static void
split_path(const char *path, char *parent_path, char *filename)
{
	unsigned pathlen = strlen(path);
	if (path[pathlen - 1] == DELIM_CHAR)
		pathlen -= 1;  // no estoy seguro si esto puede pasar pero por las dudas

	unsigned i = pathlen - 1;
	for (; i > 0; i--) {
		if (path[i] == DELIM_CHAR)
			break;
	}

	memcpy(parent_path, path, i);
	parent_path[i] = '\0';
	memcpy(filename, path + 1 + i, pathlen - i - 1);
	filename[pathlen - i - 1] = '\0';
}

void
print_inode(inode_t *inode)
{
	printf("\n");
	printf("	[debug] Inode Information\n");
	printf("	--------------------------------\n");
	printf("	[debug] i->ino:			%u\n", inode->ino);
	printf("	[debug] i->mode:		%u\n", inode->mode);
	printf("	[debug] i->uid;:		%u\n", inode->uid);
	printf("	[debug] i->gid:			%u\n", inode->gid);
	printf("	[debug] i->size:		%lu\n", inode->size);
	printf("	[debug] i->n_blocks:		%u\n", inode->n_blocks);
	printf("	[debug] i->atim;:		%lu\n", inode->atim);
	printf("	[debug] i->mtim:		%lu\n", inode->mtim);
	printf("	[debug] i->ctim:		%lu\n", inode->ctim);
	printf("	[debug] i->parent:		%u\n\n", inode->parent);
}

static inode_t *
init_inode(mode_t mode)
{
	int free_bit = get_free_bit(&inode_bitmap);
	if (free_bit < 0) {
		printf("	[debug] No free inode found in bitmap\n");
		return NULL;
	}
	set_bit(&inode_bitmap, free_bit);

	uint32_t ino_no = (uint32_t) free_bit;
	inode_t *inode = get_inode(ino_no);

	inode->ino = ino_no;
	inode->mode = mode;
	inode->uid = getuid();
	inode->gid = getgid();
	inode->size = 0;
	inode->n_blocks = 0;

	inode->atim = time(NULL);
	inode->mtim = inode->atim;
	inode->ctim = inode->atim;

	return inode;
}

static uint8_t *
init_data_block(uint32_t *block_no)
{
	int free_bit = get_free_bit(&data_bitmap);
	if (free_bit < 0) {
		printf("	[debug] No free block found in bitmap\n");
		return NULL;
	}
	set_bit(&data_bitmap, free_bit);
	printf("	[debug] Initializating data block #%d\n", free_bit);

	*block_no = (uint32_t) free_bit;
	return get_data_block(*block_no);
}

static int
insert_dentry(inode_t *parent, uint32_t ino, const char *d_name)
{
	if (parent->size == parent->n_blocks * BLOCK_SIZE) {
		uint32_t block_no;
		uint8_t *block = init_data_block(&block_no);
		if (block == NULL) {
			printf("	[debug] Couldn't initialize new data block\n");
			return -ENOMEM;
		}
		parent->data_blocks[parent->n_blocks] = block_no;
		parent->n_blocks++;
	}

	unsigned dentry_no = parent->size / DENTRY_SIZE;
	dirent_t *dent = get_dirent(dentry_no, parent);
	dent->d_ino = ino;
	strcpy(dent->d_name, d_name);
	parent->size += DENTRY_SIZE;
	parent->atim = time(NULL);
	parent->mtim = parent->atim;
	return 0;
}

static int
fisopfs_getattr(const char *path, struct stat *st)
{
	printf("[debug] fisopfs_getattr - path: %s\n", path);

	inode_t *inode = find_inode(path);
	if (inode == NULL) {
		printf("	[debug] Could not find inode for path %s\n", path);
		return -ENOENT;
	}

	fill_stat(st, inode);

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

	inode_t *dir = find_inode(path);
	if (dir == NULL)
		return -ENOENT;
	if (!S_ISDIR(dir->mode)) {
		printf("	[debug] %s is not a directory\n", path);
		return -ENOTDIR;
	}

	struct stat st;

	// Los directorios '.' y '..'
	fill_stat(&st, dir);
	filler(buffer, ".", &st, 0);
	fill_stat(&st, get_inode(dir->parent));
	filler(buffer, "..", &st, 0);

	unsigned n_dentries = dir->size / DENTRY_SIZE;
	printf("	[debug] Total entries: %u\n", n_dentries);
	for (unsigned i = 0; i < n_dentries; i++) {
		dirent_t *dentry = get_dirent(i, dir);
		inode_t *inode = get_inode(dentry->d_ino);
		fill_stat(&st, inode);
		filler(buffer, dentry->d_name, &st, 0);
	}

	return 0;
}

static int
fisopfs_read(const char *path,
             char *buffer,
             size_t size,
             off_t offset,
             struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_read - path: %s, size: %lu, offset: %lu\n", path, size, offset);

	inode_t *inode = find_inode(path);
	if (inode == NULL)
		return -ENOENT;

	if (S_ISDIR(inode->mode))
		return -EISDIR;

	// TODO: chequear permisos?
	printf("	[debug] File size is %lu\n", inode->size);

	if (inode->size < offset)
		return -ERANGE;

	if (size > inode->size - offset)
		size = inode->size - offset;

	if (size == 0)
		return 0;

	unsigned blocks_2_read = size / BLOCK_SIZE;
	unsigned last_block_offset = size % BLOCK_SIZE;

	for (unsigned i = 0; i <= blocks_2_read; i++) {
		uint8_t *block = get_data_block(inode->data_blocks[i]);
		if (i < blocks_2_read)
			memcpy(buffer + i * BLOCK_SIZE, block, BLOCK_SIZE);
		else
			memcpy(buffer + i * BLOCK_SIZE, block, last_block_offset);
	}

	inode->atim = time(NULL);
	return size;
}

static int
fisopfs_create(const char *path, mode_t mode, struct fuse_file_info *info)
{
	printf("[debug] fisopfs_create - path: %s\n", path);
	if (get_free_bit(&data_bitmap) < 0) {
		printf("	[debug] No space available\n");
		return -ENOMEM;
	}
	if (find_inode(path) != NULL) {
		printf("	[debug] File already exists\n");
		return -EEXIST;
	}

	char *parent_path = malloc(strlen(path));
	char filename[FS_FILENAME_LEN];
	split_path(path, parent_path, filename);

	inode_t *parent = find_inode(parent_path);
	free(parent_path);
	if (parent == NULL) {
		printf("	[debug] Invalid path\n");
		return -ENOENT;
	}
	if (!S_ISDIR(parent->mode)) {
		printf("	[debug] Parent path is not a directory\n");
		return -ENOTDIR;
	}
	if (parent->size / DENTRY_SIZE == N_DENTRY_PER_DIR) {
		printf("	[debug] Directory is full\n");
		return -ENOMEM;
	}

	mode |= __S_IFREG;
	inode_t *inode = init_inode(mode);
	if (inode == NULL) {
		printf("	[debug] Inode table is full\n");
		return -ENOMEM;
	}

	inode->parent = parent->ino;
	insert_dentry(parent, inode->ino, filename);
	superblock.n_files++;
	info->fh = inode->ino;
	return 0;
}

static int
fisopfs_mkdir(const char *path, mode_t mode)
{
	printf("[debug] fisopfs_mkdir - path: %s\n", path);
	if (get_free_bit(&data_bitmap) < 0) {
		printf("	[debug] No space available\n");
		return -ENOMEM;
	}
	if (find_inode(path) != NULL) {
		printf("	[debug] Directory already exists\n");
		return -EEXIST;
	}

	char *parent_path = malloc(strlen(path));
	char dirname[FS_FILENAME_LEN];
	split_path(path, parent_path, dirname);

	inode_t *parent = find_inode(parent_path);
	free(parent_path);
	if (parent == NULL) {
		printf("	[debug] Invalid path\n");
		return -ENOENT;
	}
	if (!S_ISDIR(parent->mode)) {
		printf("	[debug] Parent path is not a directory\n");
		return -ENOTDIR;
	}
	if (parent->size / DENTRY_SIZE == N_DENTRY_PER_DIR) {
		printf("	[debug] Directory is full\n");
		return -ENOMEM;
	}

	mode |= __S_IFDIR;
	inode_t *inode = init_inode(mode);
	if (inode == NULL) {
		printf("	[debug] Inode table is full\n");
		return -ENOMEM;
	}

	inode->parent = parent->ino;
	insert_dentry(parent, inode->ino, dirname);
	superblock.n_dirs++;
	return 0;
}

/** Update file's times (modification, access) */
static int
fisopfs_utimens(const char *path, const struct timespec tv[2])
{
	printf("[debug] fisopfs_utimens - path: %s\n", path);

	inode_t *inode = find_inode(path);
	if (!inode)
		return -ENOENT;

	struct timespec atime = tv[0];
	struct timespec mtime = tv[1];

	inode->atim = atime.tv_sec;
	inode->mtim = mtime.tv_sec;
	inode->ctim = mtime.tv_sec;

	return 0;
}

static void *
fisopfs_init(struct fuse_conn_info *conn)
{
	printf("[debug] fisopfs_init\n");
	printf("	[debug] There's %u blocks of %u bytes each\n", N_BLOCKS, BLOCK_SIZE);
	printf("	[debug] Inode size is %lu bytes aligned to %u\n", sizeof(inode_t), INODE_SIZE);
	printf("	[debug] An inode block contains %u inodes \n", BLOCK_SIZE / INODE_SIZE);
	printf("	[debug] There's %u inodes in %u blocks\n", N_INODES, N_INODE_BLOCKS);
	printf("	[debug] There's %u data blocks\n", N_DATA_BLOCKS);
	printf("	[debug] Data region start block: %u\n\n", DATA_REGION);

	struct fuse_context *context = fuse_get_context();
	printf("	[debug] context uid: %d, context gid: %d\n", context->uid, context->gid);

	// -----------------------------------
	// initialize root inode
	printf("	[debug] Initializing root inode\n");
	inode_t *root;
	root = init_inode(__S_IFDIR | 0775);
	print_inode(root);

	// initialize superblock
	superblock.n_dirs = 1;  // One dir: root
	superblock.n_files = 0;
	superblock.root_ino = root->ino;

	printf("	[debug] Filesystem summary:\n");
	printf("	[debug] There's %u directories\n", superblock.n_dirs);
	printf("	[debug] There's %u files\n", superblock.n_files);
	printf("	[debug] Directory entry size (aligned): %u\n", DENTRY_SIZE);

	return NULL;
}

int
unlink_inode(const char *path, inode_t *inode)
{
	// Delete directory entry from parent dir

	char *parent_path = malloc(strlen(path));
	char filename[FS_FILENAME_LEN];
	split_path(path, parent_path, filename);

	inode_t *parent = find_inode(parent_path);
	free(parent_path);


	unsigned n_dentries = parent->size / DENTRY_SIZE;

	dirent_t *curr_entry;
	dirent_t *next_entry;

	int index = -1;

	for (int i = 0; i < parent->n_blocks; i++) {
		uint32_t nentries = (i + 1) * N_DENTRY_PER_BLOCK <= n_dentries
		                            ? N_DENTRY_PER_BLOCK
		                            : n_dentries % N_DENTRY_PER_BLOCK;


		for (int j = 0; j < nentries; j++) {
			curr_entry = get_dirent(j + i * N_DENTRY_PER_BLOCK, parent);
			if (inode->ino == curr_entry->d_ino &&
			    strcmp(curr_entry->d_name, filename) == 0) {
				printf("	[debug] Found entry at index %d\n", j);
				index = j;
			}

			if (index >= 0) {
				if (!((j == nentries - 1) &&
				      (i == parent->n_blocks - 1))) {
					printf("	[debug] assigning entry: %d = entry: %d \n",
					       j + i * N_DENTRY_PER_BLOCK,
					       j + 1 + i * N_DENTRY_PER_BLOCK);
					next_entry = get_dirent(j + 1 + i * N_DENTRY_PER_BLOCK, parent);
					strcpy(curr_entry->d_name, next_entry->d_name);
					curr_entry->d_ino = next_entry->d_ino;
				}
			}
		}
	}

	if (index < 0) {
		printf("	[debug] File not found for deleting: %s\n", filename);
		return -ENOENT;
	}

	printf("	[debug] Deleting entry: %s\n", filename);

	parent->size -= DENTRY_SIZE;
	print_inode(parent);

	if (((n_dentries - 1) % N_DENTRY_PER_BLOCK) == 0) {
		printf("	[debug] Data block %d will be freed\n", parent->data_blocks[parent->n_blocks - 1]);
		clear_bit(&data_bitmap, parent->data_blocks[parent->n_blocks - 1]);
		parent->n_blocks--;
	}


	clear_bit(&inode_bitmap, inode->ino);

	return 0;
}


static int
fisopfs_unlink(const char *path)
{
	printf("[debug] fisopfs_unlink - path: %s\n", path);

	inode_t *inode = find_inode(path);
	if (!inode)
		return -ENOENT;


	return unlink_inode(path, inode);
}

static struct fuse_operations operations = {
	.init = fisopfs_init,
	.getattr = fisopfs_getattr,
	.readdir = fisopfs_readdir,
	.read = fisopfs_read,
	.create = fisopfs_create,
	.mkdir = fisopfs_mkdir,
	.utimens = fisopfs_utimens,
	.unlink = fisopfs_unlink,
};


int
main(int argc, char *argv[])
{
	return fuse_main(argc, argv, &operations, NULL);
}
