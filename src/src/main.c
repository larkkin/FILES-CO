#include <serial.h>
#include <ints.h>
#include <time.h>
#include <print.h>
#include <multiboot.h>

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


	while (1);
}
