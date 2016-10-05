#define SLAVE 0xA0 //command ports
#define MASTER 0x20//command ports
#define INIT 0x10 // 4th bit is 1
#define IC4 0x01 // zero bit
#define ICW4_8086 0x01
#include <pic.h>
#include <ioport.h>
#include <ints.h>

void PIC_init(uint8_t idt_index_for_slave, uint8_t idt_index_for_master) //function arguments//we fix two indexes in idt for master and slave
{
	out8(SLAVE, INIT + IC4); //we report the 4th and 0 bit to command ports
	out8(MASTER, INIT + IC4);
	out8(SLAVE + 1, idt_index_for_slave); //we report this two indexes
	out8(MASTER + 1, idt_index_for_master);
	out8(SLAVE + 1, 0x02); // 2
	out8(MASTER + 1, 0x04); // 2nd bit, 0100 = 4 //the cascade configure

	out8(SLAVE + 1, ICW4_8086);
	out8(MASTER + 1, ICW4_8086);
	
	out8(SLAVE+1, 0xFF);
	out8(MASTER+1, 0xFF); // mask all the interrupts 
	enable_ints();
}

void PIC_sendEOI(uint8_t interrupt_index)
{
	if (interrupt_index < 40)
	{
		out8(SLAVE, 0x20); //____|____||_________ we don;t forget both the two		 
	}
	out8(MASTER, 0x20); //_____|____||______
}

#define PIT_CMD 0x43
#define PIT_DATA0 0x40

void init_PIT(uint16_t init_value)
{
	uint16_t divisor = 1193180 / init_value;
	out8(PIT_CMD, 0x34); // 00110100  -- 00 11 010 0  
	//00 for zero channel, 11 for 2 bytes of init_value to write, 010 for 2nd working mode, 0 for BCD
	out8(PIT_DATA0, divisor & 0xFF); // 1st byte
	out8(PIT_DATA0, divisor >> 8); // 2nd byte
	out8(MASTER+1, 0xFE); // 11111110
}
