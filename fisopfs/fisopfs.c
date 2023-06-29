
#include "fisopfs.h"
#include "bitmap.h"

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
block_t blocks[N_DATA_BLOCKS];

file_t files[MAX_FILES];
dirent_t dirs[MAX_DIRS];


static inode_t *
get_inode(uint32_t ino)
{
	if (ino >= N_INODES)
		return NULL;

	return (inode_t *) &inodes[ino];
}

void
print_inode(inode_t *inode)
{
	printf("[debug] Inode Information\n");
	printf("--------------------------------\n");
	printf("[debug] i->mode:			%u\n", inode->mode);
	printf("[debug] i->uid;:			%u\n", inode->uid);
	printf("[debug] i->gid:				%u\n", inode->gid);
	printf("[debug] i->size:			%lu\n", inode->size);
	printf("[debug] i->n_blocks:			%u\n", inode->n_blocks);
	printf("[debug] i->atim;:			%lu\n", inode->atim);
	printf("[debug] i->mtim:			%lu\n", inode->mtim);
	printf("[debug] i->ctim:			%lu\n", inode->ctim);
}


int
get_name_index(const char *path)
{
	int len = (int) strlen(path);

	int trunc = -1;
	for (int c = len - 1; c >= 0;
	     c--) {  // Iterate through path starting from end of string
		if (path[c] == '/') {  // Look for '/'
			trunc = c;     // Save index
			break;
		}
	}

	if (len == 0 || trunc == -1)  // If path is NULL or slash not found
		return 0;             // Return 0. It may be a root dir file.

	if (trunc == 0)
		trunc = 1;

	return trunc + 1;
}

int
get_dir_index(const char *path)
{
	path++;

	for (int i = 0; i < superblock.n_dirs; i++)
		if (strcmp(path, dirs[i].path) == 0)
			return i;

	return -1;
}


int
get_file_index(const char *path)
{
	path++;

	for (int i = 0; i < superblock.n_files; i++)
		if (strcmp(path, files[i].path) == 0)
			return i;

	return -1;
}


int
is_dir(const char *path)
{
	path++;

	for (int i = 0; i < superblock.n_dirs; i++) {  // Iterate through dirs
		if (strcmp(path, dirs[i].path) ==
		    0)  // Check if path equals to dir path
			return 1;
	}
	return 0;
}


int
is_file(const char *path)
{
	path++;

	for (int i = 0; i < superblock.n_files; i++) {
		if (strcmp(path, files[i].path) == 0)
			return 1;
	}

	return 0;
}

static int
fisopfs_getattr(const char *path, struct stat *st)
{
	printf("[debug] fisopfs_getattr - path: %s\n", path);

	ino_t i;
	inode_t *inode;


	if ((strcmp(path, "/") == 0) || is_dir(path)) {
		i = get_dir_index(path);
		inode = &inodes[dirs[i].d_ino];
	} else if (is_file(path)) {
		i = get_file_index(path);
		inode = &inodes[files[i].d_ino];
		st->st_size = inode->size;
	} else {
		return -ENOENT;
	}

	st->st_mode = inode->mode;
	st->st_ino = i;
	st->st_gid = inode->gid;
	st->st_uid = inode->uid;
	inode->atim = time(NULL);
	st->st_atime = inode->atim;
	st->st_mtime = inode->mtim;
	st->st_ctime = inode->ctim;
	st->st_blocks = inode->n_blocks;

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

	dirent_t *dir = NULL;
	if (strcmp(path, "/") == 0)
		dir = &dirs[0];
	else {
		dir = &dirs[get_dir_index(path)];
	}

	if (dir == NULL) {
		return -ENOENT;
	}


	for (int j = 0; j < dir->n_files; j++) {  // Fill files

		int n_file = dir->files[j];
		if (n_file == -1)
			continue;

		file_t *file = &files[n_file];

		if (file != NULL) {
			filler(buffer, file->filename, NULL, 0);
		}


		// TODO - implement read for directories
		// for (int d = 0; d < superblock->n_dirs; d++) {  // Fill dirs
		// 	dirent_t *child = &dirs[d];
		// 	if (child != NULL && (&dirs[child->parent] == dir))
		// 		filler(buffer, child->dirname, NULL, 0);
		// }
	}

	return 0;
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


int
init_inode(mode_t mode)
{
	uint32_t ino_no = get_free_bit(&inode_bitmap);

	if (ino_no < 0) {
		printf("[debug] No free inode found in bitmap\n");
		return -1;
	}

	set_bit(&inode_bitmap, ino_no);

	inodes[ino_no].mode = mode;
	inodes[ino_no].uid = getuid();
	inodes[ino_no].gid = getgid();
	inodes[ino_no].size = 0;
	inodes[ino_no].n_blocks = 0;

	inodes[ino_no].atim = time(NULL);
	inodes[ino_no].mtim = inodes[ino_no].atim;
	inodes[ino_no].ctim = inodes[ino_no].atim;

	return ino_no;
}


int
init_file(const char *path, mode_t mode)
{
	path++;
	int ino_no = init_inode(mode);

	if (ino_no < 0) {
		return -1;
	}

	file_t new_file;  // Initialize new file

	new_file.d_ino = ino_no;
	strcpy(new_file.path, path);
	strcpy(new_file.filename, path + get_name_index(path));
	printf("[debug] init_file - Filename: %s \n", new_file.filename);

	files[superblock.n_files] = new_file;  // Save file in array

	return superblock.n_files++;
}


int
add_file(const char *filename, mode_t mode)
{
	// TODO - implement get_dir function from filename
	// dirent_t dirent *dir = get_dir(filename);
	dirent_t *dir = &dirs[0];  // harcoded to root dir

	int n_file = init_file(filename, mode);

	if (dir && (n_file >= 0))
		dir->files[dir->n_files++] = n_file;
	else {
		printf("[debug] add_file - Erro while creating file \n");
		return 0;
	}

	return 1;
}

static int
fisop_createdir(const char *path, mode_t mode)
{
	int len = (int) strlen(path);
	if (len > (MAX_NAME_LEN)) {
		errno = ENAMETOOLONG;
		return ENAMETOOLONG;
	}

	//TODO: USE GET DIR
	//dirent_t *parent = get_dir(path);
	dirent_t *parent = &dirs[0];
	inode_t *inode = &inodes[parent->d_ino];
	if (parent->level <= 2) {
		int i = init_inode(__S_IFDIR | 0775);
		if (i > -1) {
			dirent_t dir;
			dir.n_files = 0;
			strcpy(dir.path, path);
			strcpy(dir.dirname, path + get_name_index(path));
			dir.d_ino = i;
			dir.parent = parent->n_dir;
			dir.level = parent->level + 1;
			dir.n_dir = superblock.n_dirs;
			dirs[superblock.n_dirs++] = dir;
			return 0;
		}
	}
}

/** Create a file */
static int
fisopfs_create(const char *path, mode_t mode, struct fuse_file_info *info)
{
	printf("[debug] fisopfs_create - path: %s\n", path);
	printf("[debug] fisopfs_create - mode: %d\n", mode);

	if (!is_file(path) && strlen(path) < FS_FILENAME_LEN) {
		if (add_file(path, mode))
			return 0;

		else
			return -1;
	}

	printf("[debug] file %s already exists, or name is too large! \n", path);

	return -EEXIST;
}

/** Update file's times (modification, access) */
static int
fisopfs_utimens(const char *path, const struct timespec tv[2])
{
	printf("[debug] fisopfs_utimens - path: %s\n", path);

	return 0;
}


static void *
fisopfs_init(struct fuse_conn_info *conn)
{
	uint32_t root_ino;
	inode_t *root;

	printf("[debug] fisopfs init\n");
	printf("[debug] There's %u blocks of %u bytes each\n", N_BLOCKS, BLOCK_SIZE);
	printf("[debug] Inode size is %lu bytes aligned to %u\n",
	       sizeof(inode_t),
	       INODE_SIZE);
	printf("[debug] An inode block contains %u inodes \n",
	       BLOCK_SIZE / INODE_SIZE);
	printf("[debug] There's %u inodes in %u blocks\n", N_INODES, N_INODE_BLOCKS);
	printf("[debug] There's %u data blocks\n", N_DATA_BLOCKS);
	printf("[debug] Data region start block: %u\n", DATA_REGION);


	// -----------------------------------
	// initialize root inode

	root_ino = init_inode(__S_IFDIR | 0775);
	root = get_inode(root_ino);

	struct fuse_context *context = fuse_get_context();
	printf("[debug] fisopfs_init - context uid: %d, context gid: %d\n",
	       context->uid,
	       context->gid);

	root->uid = getuid();
	root->gid = getgid();
	root->mode = __S_IFDIR | 0755;
	root->ctim = time(NULL);
	root->atim = root->ctim;
	root->mtim = root->ctim;
	root->n_blocks = 0;

	print_inode(root);

	// -----------------------------------
	// initialize root dir
	dirent_t root_dir;

	root_dir.d_ino = root_ino;
	root_dir.parent = -1;
	strcpy(root_dir.path, "/");
	root_dir.n_files = 0;
	root_dir.n_dir = 0;
	root_dir.level = 1;

	dirs[0] = root_dir;


	// initialize superblock
	superblock.n_dirs = 1;  // One dir: root
	superblock.n_files = 0;

	return NULL;
}

static struct fuse_operations operations = {
	.init = fisopfs_init,
	.getattr = fisopfs_getattr,
	.readdir = fisopfs_readdir,
	.read = fisopfs_read,
	.create = fisopfs_create,
	.utimens = fisopfs_utimens,
	.mkdir = fisop_createdir,
};


int
main(int argc, char *argv[])
{
	return fuse_main(argc, argv, &operations, NULL);
}
