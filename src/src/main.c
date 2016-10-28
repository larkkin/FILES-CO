#include <serial.h>
#include <ints.h>
#include <time.h>
#include <print.h>
#include <multiboot.h>
#include <buddy_allocator.h>

static void qemu_gdb_hang(void)
{
#ifdef DEBUG
	static volatile int wait = 1;

	while (wait);
#endif
}

extern char text_phys_begin[];
extern char bss_phys_end[];



void print_free_memory(multiboot_info_t* mbt)
{
    uint64_t os_start = (uint64_t) text_phys_begin;
    uint64_t os_end = (uint64_t) bss_phys_end;
    
    
    printf("free memory segments:\n");
    for (multiboot_memory_map_t* mmap = (multiboot_memory_map_t*)(uint64_t) mbt->mmap_addr;
         mmap < (multiboot_memory_map_t*) (uint64_t) (mbt->mmap_addr + mbt->mmap_length);
         mmap = (multiboot_memory_map_t*) (uint64_t) ( (uint64_t) mmap + mmap->size + sizeof(mmap->size) ))  
    {
        if (mmap->type != 1)
        {
            continue;
        }
        if (mmap->addr >= os_end || mmap->addr + mmap->len <= os_start)
        {  
            // segment_start....segment_end.......os_start....os_end  or 
            // os_start....os_end.......segment_start....segment_end
            printf("from: %lx to: %lx\n", mmap->addr, mmap->addr + mmap->len);
            continue; 
        }
        if (mmap->addr < os_start)
        {
            // segment_start....os_start
            printf("from: %lx to: %lx\n", mmap->addr, os_start);
        }
        if (mmap->addr + mmap->len > os_end)
        {
            // os_end....segment_end
            printf("from: %lx to: %lx\n", os_end, mmap->addr + mmap->len);
        }
    }
}

typedef struct longest_free_segment_type {
    uint64_t start;
    uint64_t end;
} longest_free_segment_t;

longest_free_segment_t find_longest_free_segment(multiboot_info_t* mbt)
{
    uint64_t os_start = (uint64_t) text_phys_begin;
    uint64_t os_end = (uint64_t) bss_phys_end;
    
    uint64_t res_start = 0;
    uint64_t res_end = 0;
    uint64_t max_length = 0;

    for (multiboot_memory_map_t* mmap = (multiboot_memory_map_t*)(uint64_t) mbt->mmap_addr;
         mmap < (multiboot_memory_map_t*) (uint64_t) (mbt->mmap_addr + mbt->mmap_length);
         mmap = (multiboot_memory_map_t*) (uint64_t) ( (uint64_t) mmap + mmap->size + sizeof(mmap->size) ))  
    {
        if (mmap->type != 1)
        {
            continue;
        }
        if (mmap->addr >= os_end || mmap->addr + mmap->len <= os_start)
        {  
            // segment_start....segment_end.......os_start....os_end  or 
            // os_start....os_end.......segment_start....segment_end
            // printf("from: %lx to: %lx\n", mmap->addr, mmap->addr + mmap->len);
            if (mmap->addr + mmap->len - mmap->addr > max_length) {
                res_start = mmap->addr;
                res_end = mmap->addr + mmap->len;
                max_length = res_end - res_start;
            }
            continue; 
        }
        if (mmap->addr < os_start)
        {
            // segment_start....os_start
            if (os_start - mmap->addr > max_length) {
                res_start = mmap->addr;
                res_end = os_start;
                max_length = res_end - res_start;
            }
        }
        if (mmap->addr + mmap->len > os_end)
        {
            // os_end....segment_end
            if (mmap->addr + mmap->len - os_end > max_length) {
                res_start = os_end;
                res_end = mmap->addr + mmap->len;
                max_length = res_end - res_start;
            }
        }
    }
    longest_free_segment_t res;
    res.start = res_start;
    res.end = res_end;
    return res;
}

void main(uint32_t mbt_num)
{
    qemu_gdb_hang();

	serial_setup();
	ints_setup();
	//time_setup();
	enable_ints();
	printf("hi!\n");
	
    // multiboot_info_t* mbt =
    //         (multiboot_info_t* )(uint64_t)mbt_info;
    multiboot_info_t* mbt = (multiboot_info_t*) (uint64_t) mbt_num;
    
    if (mbt->flags & (1 << 6))
    {
        printf("valid memory map\n");
    }
    else
    {
        return;
    }
    
    int entry_num = 0;
    multiboot_memory_map_t* mmap = (multiboot_memory_map_t*)(uint64_t) mbt->mmap_addr;
    // printf("kernel: from %d to %d\n", text_phys_begin, data_phys_end);
    printf("kernel: from %lx to %lx\n", text_phys_begin, bss_phys_end);
    while(mmap < (multiboot_memory_map_t*) (uint64_t) (mbt->mmap_addr + mbt->mmap_length)) {
        entry_num++;
        printf("memory map entry %d:\n", entry_num);
        printf("size: %lx, base_addr: %lx, length: %lx, type: %lx\n", 
            mmap->size, mmap->addr, mmap->len, mmap->type);
        mmap = (multiboot_memory_map_t*) (uint64_t) ( (uint64_t) mmap + mmap->size + sizeof(mmap->size) );
    }
    printf("\nkernel: from %lx to %lx\n\n", text_phys_begin, bss_phys_end);

    print_free_memory(mbt);

    // longest_free_segment_t longest_free_segment = find_longest_free_segment(mbt);
    init_allocator(mbt);
    int* arr = (int*) b_allocate(1600).page_ptr;
    int* arr2 = (int*) b_allocate(3).page_ptr;
    arr[1024] = 2;
    printf("%d\n", arr[1024]);
    printf("allocated to %lx\n", (uint64_t) arr);
    if (b_free((page_t*) arr)) {
        printf("yes\n");
    }
    if (b_free((page_t*) arr2)) {
        printf("yes\n");
    }
    printf("%d\n", arr[1024]);

	while (1);
}
