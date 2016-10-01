#define SLAVE 0x20 //command ports
#define MASTER 0xA0//command ports
#define INIT 0x10 // 4th bit is 1
#define IC4 0x01 // zero bit
#define ICW4_8086 0x01
#include <pic.h>
#include <ioport.h>

void PIC_init(uint8_t idt_index_for_slave, uint8_t idt_index_for_master) //function arguments//we fix two indexes in idt for master and slave
{
	uint8_t a1;
	uint8_t a2; // save the info that we had in ports

	a1 = in8(SLAVE + 1); 
	a2 = in8(MASTER + 1); //mozhno ubrat

	out8(SLAVE, INIT + IC4); //we report the 4th and 0 bit to command ports
	out8(MASTER, INIT + IC4);
	out8(SLAVE + 1, idt_index_for_slave); //we report this two indexes
	out8(MASTER + 1, idt_index_for_master);
	out8(SLAVE + 1, 0x02); // 2
	out8(MASTER + 1, 0x04); // 2nd bit, 0100 = 4 //the cascade configure

	out8(SLAVE + 1, ICW4_8086);
	out8(MASTER + 1, ICW4_8086);

	out8(SLAVE + 1, a1);
	out8(MASTER + 1, a2); // set the previous state of the ports mask!!
}

void PIC_sendEOI(uint8_t interrupt_index)
{
	if ((interrupt_index >= 32) && (interrupt_index< 40))
	{
		out8(MASTER, 0x20); //____|____||_________ we don;t forget both the two 
	}
	out8(SLAVE, 0x20); //_____|____||______
}
