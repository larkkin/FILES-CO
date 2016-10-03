#include <ioport.h>
#include <serport.h>
void ser_port_init()
{
	out8(0x3f8+3, 0x00); //because we want the port +1 to wait for the inf, not the coeff
	out8(0x3f8+1, 0x00);
	out8(0x3f8+3, 0x80);//setting up the seventh bit for others to wait for coeff
	out8(0x3f8+0, 0x02);
	out8(0x3f8+1, 0x00); //presenting the coeff as two bytes
	out8(0x3f8+3, 0x03); //00000011
}
static uint8_t can_write() //here we check whether the fifth bit in port5 is ok
{
//in8(0x3f8 + 5); //we grep these eight bits	
return in8(0x3f8 + 5) & 0x20; //bit-4-bit &
}
void ser_port_write_char(char something) 
{
	while (!can_write());
	out8(0x3f8,something);
	
}
void ser_port_write_string (char* something)
{
	
 	while(*something != '\0')
	{
		ser_port_write_char(*something);
		something++;
	}
}

/*
void ser_port_write_64(uint64_t something) 
{
	while (!can_write());
	out32(0x3f8, something >> 32);
	while (!can_write());
	out32(0x3f8, something & 0x00000000FFFFFFFF);
	
}
*/