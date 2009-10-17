#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include "mmap.h"

int map_file(mf_t *mf, char *file, int ofs, int size)
{
	struct stat attr;
	int fd;

	if ((fd = open(file, O_RDONLY)) < 0)
		return fd;

	if (fstat(fd, &attr)) {
		close(fd);
		return -1;
	}

	if (ofs < 1)
		ofs = 0;

	if ((size < 1) || (size > (attr.st_size - ofs)))
		size = attr.st_size - ofs;

	if ((mf->mem =
	     mmap(NULL, size, PROT_READ, MAP_SHARED, fd, ofs)) == MAP_FAILED) {
		close(fd);
		return -1;
	}
	
	//readahead(fd, ofs, size);

	close(fd);

	mf->real_len = mf->len = attr.st_size;
	mf->ptr = mf->mem;

	return 0;
}

void unmap_file(mf_t *mf)
{
	munmap(mf->mem, mf->real_len);
	mf->mem = mf->ptr = NULL;
	mf->real_len = mf->len = 0;
}
