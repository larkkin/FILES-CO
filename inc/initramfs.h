#ifndef __INITRAMFS_H__
#define __INITRAMFS_H__

#include <stdint.h>


#define S_IFMT	0xF000
#define S_IFDIR	0x4000
#define S_IFREG	0x8000

#define S_ISDIR(mode)	(((mode) & S_IFMT) == S_IFDIR)
#define S_ISREG(mode)	(((mode) & S_IFMT) == S_IFREG)

#define END_OF_ARCHIVE	"TRAILER!!!"

struct cpio_header {
	char magic[6];
	char inode[8];
	char mode[8];
	char uid[8];
	char gid[8];
	char nlink[8];
	char mtime[8];
	char filesize[8];
	char major[8];
	char minor[8];
	char rmajor[8];
	char rminor[8];
	char namesize[8];
	char chksum[8];
} __attribute__((packed));

typedef struct cpio_header cpio_header_t;

typedef struct uint64_pair_type {
	uint64_t first;
	uint64_t second;
} uint64_pair;

struct mboot_info;

uint64_pair initramfs(struct mboot_info const* mbt);
void load_cpio();


#endif /*__INITRAMFS_H__*/
