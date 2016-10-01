#ifndef __PIC_H_
#define __PIC_H_

#include <stdint.h>

void PIC_init(uint8_t idt_index_for_slave, uint8_t idt_index_for_master);
void PIC_sendEOI(uint8_t interrupt_index);

#endif // __PIC_H_
