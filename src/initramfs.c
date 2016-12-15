#include <initramfs.h>
#include <debug.h>
#include <print.h>
#include <memory.h>
#include <alloc.h>
#include <string.h>
#include <stdint.h>
#include <file_system.h>


static int streq(char* from, char* to) { // assumes that sizes are ok
	char* current_symb = to;
	for (char* c = from; *c != '\0'; c++)
	{
		if (*current_symb != *c) {
			return 0;
		}
		current_symb++;
	}
	return 1;
}



typedef struct mboot_info
{
   	uint32_t flags;
  	uint8_t ignore0[16];
   	uint32_t mods_count;
   	uint32_t mods_addr;
} mboot_info_t;


typedef struct multiboot_mod_desc
{
	uint32_t start;
	uint32_t end;
	uint32_t smth;
	uint32_t smth2;	
} multiboot_mod_desc_t;


static uint64_t initramfs_start;
static uint64_t initramfs_end;


uint64_pair initramfs(mboot_info_t const* mbt) {
	BUG_ON(!(mbt->flags & 1000));
    multiboot_mod_desc_t* current_desc = (multiboot_mod_desc_t* ) ((uint64_t) mbt->mods_addr);

    for (uint32_t i = 0; i < mbt->mods_count; i++) {

    	uint64_pair res;
        res.first = (uint64_t) current_desc->start;
        res.second = (uint64_t) current_desc->end;
        char* str = (char*) res.first;
        printf("0x%lu 0x%lu\n", res.first, res.second);
        if ( res.second - res.first >= 7 && // large enough? magic+smth
        	 (str[0] == '0' ||
        	  str[1] == '7' ||
        	  str[2] == '0' ||
        	  str[3] == '7' ||
        	  str[4] == '0' ||
        	  str[5] == '1')) { 

        	initramfs_start = res.first;
        	initramfs_end = res.second;
            return res;
        }
        current_desc++;
    }
    BUG_ON(1);
}


uint64_t parse_str_to_hex(char* start) {
	int size = 8;
	uint64_t res = 0;
	for (int i = 0; i < size; i++) {
		char c = start[i];
		if (c <= '9' && c >= '0') {
			res += (c - '0') * (1 << (4*(size-i-1)));
			// printf("%d\n", res);
			continue;
		}
		if (c <= 'F' && c >= 'A') {
			res += (c - 'A' + 10) * (1 << (4*(size-i-1)));
			// printf("\t%d\n", res);
			continue;
		}
		if (c <= 'f' && c >= 'a') {
			res += (c - 'a' + 10) * (1 << (4*(size-i-1)));
			continue;
		}
		BUG_ON(1);
	}
	return res;
}

static int files_count;

uint64_t handle_one_cpio_object(uint64_t header_start) {
	files_count++;
	printf("cpio files count: %d, header_start: %d\n", files_count, header_start);
	header_start = (header_start + 3) / 4 * 4; // 4b-align 
	// printf("aligned header_start: %d\n", header_start);
	// printf("aligned header_start: %d, initramfs_end: %d\n", header_start, initramfs_end);
	// printf("%d\n", header_start > initramfs_end);

	if (header_start >= initramfs_end) {
		return initramfs_end;
	}

	cpio_header_t* header = (cpio_header_t*) va(header_start);
	uint64_t name_start = (uint64_t) va(header_start + sizeof(cpio_header_t));
	uint64_t file_size = parse_str_to_hex(header->filesize);
	uint64_t name_size = parse_str_to_hex(header->namesize);

	char* name = mem_alloc(name_size + 1);
	name[name_size] = '\0';
	for (uint64_t i = 0; i < name_size; i++) {
		name[i] = ((char*) name_start)[i];
	}
	if (name_size == 11 && streq(name, "TRAILER!!!")) {
		return initramfs_end;
	};

	char* path = mem_alloc(name_size + 2);
	path[0] = '/';
	path[name_size+1] = '\0';
	for (uint64_t i = 0; i < name_size; i++) {
		path[i+1] = name[i];
	}
	mem_free(name);

	uint64_t type = parse_str_to_hex(header->mode);
	if (S_ISDIR(type) && path[name_size-1] == '/') {
		path[name_size-1] = '\0';
	}

	uint64_t phys_data_start = header_start + sizeof(cpio_header_t) + name_size;
	phys_data_start = (phys_data_start + 3) / 4 * 4; // 4b-align

	uint64_t data_start = (uint64_t) va(phys_data_start);
	if (S_ISREG(type)) {
		// data_start = (data_start + 3) / 4 * 4; // 4b-align
		fs_node_t* file = open(path);
		write(file, 0, data_start, file_size);
		close(file);
		printf(path);
		printf("      -- file\n");
		mem_free(path);
		return phys_data_start + file_size;
	}
	if (S_ISDIR(type)) {
		printf(path);
		printf("      -- dir\n");
		mkdir(path);
		mem_free(path);
		return header_start + sizeof(cpio_header_t) + name_size;
	}
	BUG_ON(1);
}

void load_cpio() {
	files_count = 0;
	printf("loading cpio initrams, initramfs start: %d, initramfs end: %d\n", 
			initramfs_start, initramfs_end);
	uint64_t object = initramfs_start;
	while(object != initramfs_end) {
		object = handle_one_cpio_object(object);
	}
}

