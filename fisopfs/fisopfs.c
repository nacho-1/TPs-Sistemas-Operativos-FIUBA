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

char file_name[FS_FILENAME_LEN] = "fs.fisopfs";

static void
fill_stat(struct stat *st, inode_t *inode)
{
	st->st_ino = inode->ino;
	st->st_mode = inode->mode;
	st->st_uid = inode->uid;
	st->st_gid = inode->gid;
	st->st_size = inode->size;
	st->st_blocks = inode->n_blocks;
	st->st_blksize = BLOCK_SIZE;
	inode->atim = time(NULL);
	st->st_atime = inode->atim;
	st->st_mtime = inode->mtim;
	st->st_ctime = inode->ctim;
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
	assert(path != NULL);

	size_t path_len = strlen(path);
	if (path_len > FS_MAX_PATH || path_len == 0)
		return NULL;
	if (strcmp(path, ROOT_INODE_NAME) == 0)
		return curr_dir;

	// Se copia el path dado que strtok() modifica el src_string y 'path' es const.
	char filepath[FS_MAX_PATH];
	memcpy(filepath, path, path_len + 1);

	char tokens[MAX_LEVEL][FS_FILENAME_LEN];
	int path_level = 0;

	char *token = strtok(filepath, "/");

	if (!token)
		return NULL;  // Ejemplo: "/////" -> NULL

	// Los tokens devueltos por strtok terminan en \0
	memcpy(tokens[path_level++], token, strlen(token) + 1);
	// printf("	[debug] find_inode. token: %s\n", token);

	while ((token = strtok(NULL, "/"))) {
		if (path_level > MAX_LEVEL) {
			// printf("	[debug] find_inode. Max level reached.\n");
			return NULL;
		}
		memcpy(tokens[path_level++], token, strlen(token) + 1);
		// printf("	[debug] find_inode. token: %s\n", token);
	}

	for (unsigned i = 0; i < path_level; i++) {
		curr_dir = find_in_dir(tokens[i], curr_dir);
		if (curr_dir == NULL)
			break;
	}

	return curr_dir;
}

static void
split_path(const char *path, char *parent_path, char *filename)
{
	unsigned pathlen = strlen(path);
	if (path[pathlen - 1] == DELIM_CHAR)
		pathlen -= 1;  // no estoy seguro si esto puede pasar pero por las dudas

	unsigned last_delim_pos = pathlen - 1;
	for (; last_delim_pos > 0; last_delim_pos--) {
		if (path[last_delim_pos] == DELIM_CHAR)
			break;
	}

	if (last_delim_pos == 0) {  // es el root
		memcpy(parent_path, path, 1);
		parent_path[1] = '\0';
	} else {
		memcpy(parent_path, path, last_delim_pos);
		parent_path[last_delim_pos] = '\0';
	}

	memcpy(filename, path + 1 + last_delim_pos, pathlen - last_delim_pos - 1);
	filename[pathlen - last_delim_pos - 1] = '\0';
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
write_file(inode_t *inode,
           const char *buffer,
           size_t size,
           off_t offset)
{
	printf("	[debug] Writing file...\n");

	if (inode == NULL)
		return -ENOENT;

	if (S_ISDIR(inode->mode))
		return -EISDIR;

	printf("	[debug] File size is: %lu\n", inode->size);

	if (inode->size < offset)
		return -EFBIG;

	if (size + offset > N_DATA_BLOCKS_PER_INODE * BLOCK_SIZE)
		return -EFBIG;

	if (size == 0)
		return 0;

	unsigned first_blk_no = offset / BLOCK_SIZE;
	unsigned first_blk_offset = offset % BLOCK_SIZE;
	unsigned last_blk_no = (size + offset) / BLOCK_SIZE;
	unsigned last_blk_offset = (size + offset) % BLOCK_SIZE;
	unsigned bytes_written = 0;

	for (unsigned i = first_blk_no; i <= last_blk_no; i++) {
		uint8_t *block = NULL;
		if (i < inode->n_blocks) {
			block = get_data_block(inode->data_blocks[i]);
		} else {
			uint32_t block_no;
			block = init_data_block(&block_no);
			if (block == NULL)  // TODO: no hay mas espacio en el disco. Que hacemos en este caso?
				return bytes_written;
			inode->data_blocks[inode->n_blocks] = block_no;
			inode->n_blocks++;
			printf("	[debug] Initialized data block for inode\n");
		}

		unsigned start_offset = 0;
		unsigned write_size = BLOCK_SIZE;
		if (i == first_blk_no) {
			start_offset = first_blk_offset;
			write_size = BLOCK_SIZE - first_blk_offset;
		}
		if (i == last_blk_no)
			write_size = last_blk_offset - start_offset;

		memcpy(block + start_offset, buffer + bytes_written, write_size);
		printf("	[debug] Wrote %u bytes to file\n", write_size);
		bytes_written += write_size;
		inode->size = inode->size > (offset + bytes_written) ? inode->size : (offset + bytes_written);
		printf("	[debug] File size is: %lu\n", inode->size);
		inode->atim = time(NULL);
		inode->mtim = inode->atim;
	}

	return bytes_written;
}


/*
 * Disminuye el tamaño del archivo asociado al inodo
 * al tamaño length
 */
static void
shrink_file(inode_t *inode, off_t length)
{
	if (inode == NULL || S_ISDIR(inode->mode)  // Solamente archivos tipo file
	    || inode->size <= length) {            // Solamente para encoger archivos, ver fisopfs_truncate
		printf("	[debug] Bad shrink\n");
		return;
	}

	unsigned last_blk_no = length / BLOCK_SIZE;
	unsigned last_blk_offset = length % BLOCK_SIZE;
	unsigned n_blocks = inode->n_blocks;
	for (unsigned i = last_blk_no; i < n_blocks; i++) {
		if (i == last_blk_no && last_blk_offset > 0)
			continue;
		uint32_t block_no = inode->data_blocks[i];
		clear_bit(&data_bitmap, (int) block_no);
		inode->n_blocks--;
	}
	inode->size = length;
	inode->ctim = time(NULL);
	inode->mtim = inode->ctim;
}


void
load_file_system(FILE *file)
{
	printf("	[debug] loading file system\n");
	if (fread(&superblock, sizeof(superblock), 1, file) <= 0) {
		printf("error reading loading file: %s", file_name);
	}
	if (fread(&__inode_bitmap, sizeof(__inode_bitmap), 1, file) <= 0) {
		printf("error reading loading file: %s", file_name);
	}
	if (fread(&__data_bitmap, sizeof(__data_bitmap), 1, file) <= 0) {
		printf("error reading loading file: %s", file_name);
	};
	if (fread(&inodes, sizeof(inodes), 1, file) <= 0) {
		printf("error reading loading file: %s", file_name);
	}
	if (fread(&data_blocks, sizeof(data_blocks), 1, file) <= 0) {
		printf("error reading loading file: %s", file_name);
	}
}


void
save_file_system()
{
	printf("	[debug] saving file system\n");
	FILE *file = fopen(file_name, "w+");

	fwrite(&superblock, sizeof(superblock), 1, file);
	fwrite(&__inode_bitmap, sizeof(__inode_bitmap), 1, file);
	fwrite(&__data_bitmap, sizeof(__data_bitmap), 1, file);
	fwrite(&inodes, sizeof(inodes), 1, file);
	fwrite(&data_blocks, sizeof(data_blocks), 1, file);

	fclose(file);
}


static int
fisopfs_getattr(const char *path, struct stat *st)
{
	printf("\n[debug] fisopfs_getattr - path: %s\n", path);

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
	printf("\n[debug] fisopfs_readdir - path: %s\n", path);

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
	printf("\n[debug] fisopfs_read - path: %s, size: %lu, offset: %lu\n", path, size, offset);

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

	unsigned first_blk_no = offset / BLOCK_SIZE;
	unsigned first_blk_offset = offset % BLOCK_SIZE;
	unsigned last_blk_no = (size + offset) / BLOCK_SIZE;
	unsigned last_blk_offset = (size + offset) % BLOCK_SIZE;
	unsigned bytes_read = 0;

	for (unsigned i = first_blk_no; i <= last_blk_no; i++) {
		uint8_t *block = get_data_block(inode->data_blocks[i]);
		unsigned start_offset = 0;
		unsigned read_size = BLOCK_SIZE;
		if (i == first_blk_no) {
			start_offset = first_blk_offset;
			read_size = BLOCK_SIZE - first_blk_offset;
		}
		if (i == last_blk_no)
			read_size = last_blk_offset - start_offset;

		memcpy(buffer + bytes_read, block + start_offset, read_size);
		bytes_read += read_size;
	}

	inode->atim = time(NULL);
	assert(bytes_read == size);
	return bytes_read;
}

static int
fisopfs_write(const char *path,
              const char *buffer,
              size_t size,
              off_t offset,
              struct fuse_file_info *fi)
{
	printf("\n[debug] fisopfs_write - path: %s, size: %lu, offset: %lu\n", path, size, offset);

	inode_t *inode = find_inode(path);
	if (inode == NULL)
		return -ENOENT;

	if (S_ISDIR(inode->mode))
		return -EISDIR;

	// TODO: chequear permisos
	return write_file(inode, buffer, size, offset);
}

static int
fisopfs_truncate(const char *path, off_t length)
{
	printf("\n[debug] fisopfs_truncate - path: %s, length: %lu\n", path, length);

	if (length > N_DATA_BLOCKS_PER_INODE * BLOCK_SIZE)
		return -EFBIG;

	inode_t *inode = find_inode(path);
	if (inode == NULL)
		return -ENOENT;

	if (S_ISDIR(inode->mode))
		return -EISDIR;

	// TODO: chequear permisos
	printf("	[debug] File size is: %lu\n", inode->size);
	int ret;

	if (length == inode->size) {
		ret = 0;
	} else if (length > inode->size) {
		char *buffer = malloc(length - inode->size);
		memset(buffer, 0, length - inode->size);
		ret = write_file(inode, buffer, length - inode->size, inode->size);
		free(buffer);
		if (ret >= 0)
			ret = 0;
	} else {  // length < inode->size
		shrink_file(inode, length);
		ret = 0;
	}

	return ret;
}

static int
fisopfs_create(const char *path, mode_t mode, struct fuse_file_info *info)
{
	printf("\n[debug] fisopfs_create - path: %s\n", path);
	if (strlen(path) > FS_MAX_PATH) {
		printf("	[debug] Path is too long.\n");
		return -ENAMETOOLONG;
	}
	if (get_free_bit(&data_bitmap) < 0) {
		printf("	[debug] No space available\n");
		return -ENOMEM;
	}
	if (find_inode(path) != NULL) {
		printf("	[debug] File already exists\n");
		return -EEXIST;
	}

	char parent_path[FS_MAX_PATH];
	char filename[FS_FILENAME_LEN];
	split_path(path, parent_path, filename);

	if (strlen(filename) > FS_FILENAME_LEN) {
		printf("	[debug] File name too long\n");
		return -ENAMETOOLONG;
	}

	inode_t *parent = find_inode(parent_path);
	if (parent == NULL) {
		printf("	[debug] Invalid path or exceeds max level.\n");
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
	printf("\n[debug] fisopfs_mkdir - path: %s\n", path);
	if (strlen(path) > FS_MAX_PATH) {
		printf("[debug] Path is too long.\n");
		return -ENAMETOOLONG;
	}
	if (get_free_bit(&data_bitmap) < 0) {
		printf("	[debug] No space available\n");
		return -ENOMEM;
	}
	if (find_inode(path) != NULL) {
		printf("	[debug] Directory already exists\n");
		return -EEXIST;
	}

	char parent_path[FS_MAX_PATH];
	char dirname[FS_FILENAME_LEN];
	split_path(path, parent_path, dirname);

	printf("	[debug] parent_path: %s\n", parent_path);
	printf("	[debug] dirname: %s\n", dirname);
	if (strlen(dirname) > FS_FILENAME_LEN) {
		printf("	[debug] File name too long\n");
		return -ENAMETOOLONG;
	}

	inode_t *parent = find_inode(parent_path);
	if (parent == NULL) {
		printf("	[debug] Invalid path or exceeds max level\n");
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
		return -EDQUOT;
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
	printf("\n[debug] fisopfs_utimens - path: %s\n", path);

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
	char name[FS_FILENAME_LEN];
	char *fgets_status;
	;
	printf("Enter a name for file system, must finish with .fisopfs and exist\n");
	fgets_status = fgets(name, FS_FILENAME_LEN, stdin);
	if (fgets_status != NULL && (strstr(name, ".fisopfs") != NULL)) {
		strcpy(file_name, name);
	}

	printf("\n[debug] fisopfs_init\n");
	printf("	[debug] There's %u blocks of %u bytes each\n", N_BLOCKS, BLOCK_SIZE);
	printf("	[debug] Inode size is %lu bytes aligned to %u\n", sizeof(inode_t), INODE_SIZE);
	printf("	[debug] An inode block contains %u inodes \n", BLOCK_SIZE / INODE_SIZE);
	printf("	[debug] There's %u inodes in %u blocks\n", N_INODES, N_INODE_BLOCKS);
	printf("	[debug] There's %u data blocks\n", N_DATA_BLOCKS);
	printf("	[debug] Data region start block: %u\n\n", DATA_REGION);
	printf("	[debug] File system name: %s\n\n", file_name);

	struct fuse_context *context = fuse_get_context();
	printf("	[debug] context uid: %d, context gid: %d\n", context->uid, context->gid);


	printf("	[debug] Looking for saved file system data in file: %s\n", file_name);
	FILE *file = fopen(file_name, "r");

	if (file != NULL) {
		printf("	[debug] File system data found: %s\n", file_name);
		load_file_system(file);
		fclose(file);
	} else {
		printf("	[debug] File system data not found: %s\n", file_name);
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
	}
	printf("	[debug] Filesystem summary:\n");
	printf("	[debug] There's %u directories\n", superblock.n_dirs);
	printf("	[debug] There's %u files\n", superblock.n_files);
	printf("	[debug] Directory entry size (aligned): %u\n", DENTRY_SIZE);

	return NULL;
}

static int
unlink_inode(const char *path, inode_t *inode)
{
	if (inode == NULL)
		return -ENOENT;

	if (S_ISDIR(inode->mode) && inode->size > 0)
		return -ENOTEMPTY;

	// Delete directory entry from parent dir
	char parent_path[FS_MAX_PATH];
	char filename[FS_FILENAME_LEN];
	split_path(path, parent_path, filename);

	inode_t *parent = find_inode(parent_path);

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

	// Free las parent data block if there is no content
	if (((n_dentries - 1) % N_DENTRY_PER_BLOCK) == 0) {
		printf("	[debug] Data block %d from parent will be freed\n", parent->data_blocks[parent->n_blocks - 1]);
		clear_bit(&data_bitmap, parent->data_blocks[parent->n_blocks - 1]);
		parent->n_blocks--;
	}

	if (!S_ISDIR(inode->mode))
		shrink_file(inode, 0);

	clear_bit(&inode_bitmap, inode->ino);

	return 0;
}


static int
fisopfs_unlink(const char *path)
{
	printf("\n[debug] fisopfs_unlink - path: %s\n", path);

	inode_t *inode = find_inode(path);
	if (!inode)
		return -ENOENT;

	if (S_ISDIR(inode->mode))
		return -EISDIR;

	return unlink_inode(path, inode);
}


static int
fisopfs_rmdir(const char *path)
{
	printf("\n[debug] fisopfs_rmdir - path: %s\n", path);

	inode_t *dir = find_inode(path);

	if (!dir)
		return -ENOENT;

	if (!S_ISDIR(dir->mode))
		return -ENOTDIR;

	if (dir->ino == superblock.root_ino)
		// forbid removal of the root inode
		return -EPERM;

	if (dir->size > 0)
		// only delete a directory if it's empty
		return -ENOTEMPTY;

	return unlink_inode(path, dir);
}


void
fisopfs_destroy(void *a)
{
	printf("\n[debug] fisopfs_destroy\n");
	save_file_system();
}


static int
fisopfs_flush(const char *path, struct fuse_file_info *fi)
{
	printf("\n[debug] fisopfs_flush - path: %s\n", path);
	save_file_system();

	return 0;
}

static struct fuse_operations operations = {
	.init = fisopfs_init,
	.getattr = fisopfs_getattr,
	.readdir = fisopfs_readdir,
	.read = fisopfs_read,
	.write = fisopfs_write,
	.truncate = fisopfs_truncate,
	.create = fisopfs_create,
	.mkdir = fisopfs_mkdir,
	.utimens = fisopfs_utimens,
	.unlink = fisopfs_unlink,
	.rmdir = fisopfs_rmdir,
	.destroy = fisopfs_destroy,
	.flush = fisopfs_flush
};


int
main(int argc, char *argv[])
{
	return fuse_main(argc, argv, &operations, NULL);
}
