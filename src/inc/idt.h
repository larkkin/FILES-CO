#ifndef __IDT_H_
#define __IDT_H_

#include <stdint.h>

void init_idt();
void clear_idt();
void add_to_idt(uint8_t interrupt_index, uint64_t address_of_interrupt_handler, uint16_t selector);


#endif // __IDT_H_