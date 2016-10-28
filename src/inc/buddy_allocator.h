#ifndef __BUDDY_H__
#define __BUDDY_H__

#include <stdint.h>
#include <stdbool.h>
#include <multiboot.h>

#define PAGE_SIZE 4096

typedef struct page_type {
	char arr[1<<12];
} page_t;

typedef struct allocate_return_type {
	bool success;
	page_t* page_ptr;	
} allocate_return_t;

allocate_return_t b_allocate(uint32_t size);
bool b_free(page_t* page);
void init_allocator(multiboot_info_t* mbt);


#endif /*__BUDDY_H__*/