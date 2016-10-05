//#define DEBUG
static void qemu_gdb_hang(void)
{
#ifdef DEBUG
	static volatile int wait = 1;

	while (wait);
#endif
}

#include <desc.h>
#include <ints.h>
#include <serport.h>
#include <idt.h>
#include <stdint.h>
#include <pic.h>

struct frame 
{
	uint64_t r15;
	uint64_t r14;
	uint64_t r13;
	uint64_t r12;
	uint64_t r11;
	uint64_t r10;
	uint64_t r9;
	uint64_t r8;
	uint64_t rsi;
	uint64_t rdi;
	uint64_t rbp;
	uint64_t rdx;
	uint64_t rcx;
	uint64_t rbx;
	uint64_t rax;
	uint64_t intno;
	uint64_t error;
	//uint64_t rsp;
}__attribute__((packed));

char* nums[50] = {"0","1","2","3","4","5","6",
				  "7","8","9","10","11","12", "13",
				  "14","15","16","17","18","19","20",
				  "21","22","23","24","25","26","27",
				  "28","29","30","31","32","33","34",
				  "35","36","37","38","39","40","41",
				  "42","43","44","45","46","47","48","49"};



void c_handler(struct frame* fr)
{
	ser_port_write_string("got interrupt ");
	ser_port_write_string(nums[fr->intno]);
	ser_port_write_string(" \n");
	if (fr->intno < 48 && fr->intno >= 32) 
	{
		PIC_sendEOI(fr->intno);
	}
}


void pit_handler(struct frame* fr)
{
	static uint64_t counter = 1;	
	counter++;
	if (counter % 10 == 0)
	{
		ser_port_write_string("beep\n");
	}
	if (fr->intno < 48) 
	{
		PIC_sendEOI(fr->intno);
	}
}


void main(void)
{
	qemu_gdb_hang();
    ser_port_init();
    ser_port_write_string("hi!\n");

    //clear_idt();
    init_idt();
 	
 	ser_port_write_string("idt initialized\n");
 	__asm__ volatile ("int $1");
 	ser_port_write_string("interrupt handled\n"); // lie :(
	__asm__ volatile ("int $16"); 
	ser_port_write_string("2nd interrupt handled\n");

	PIC_init(32, 40);
	init_PIT(10);

	while (1);
}
