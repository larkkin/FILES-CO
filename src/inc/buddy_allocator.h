#ifndef __BUDDY_H__
#define __BUDDY_H__

#include <stdint.h>
#include <stdbool.h>

#define PAGE_SIZE 4096

typedef struct page_type {
	char arr[1<<12];
} page_t;

page_t* b_allocate(uint32_t size);
bool b_free(page_t* page);
void init_allocator(uint64_t start, uint64_t end);


#endif /*__BUDDY_H__*/