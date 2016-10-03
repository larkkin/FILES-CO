#ifndef __PIC_H_
#define __PIC_H_

#include <stdint.h>

void PIC_init(uint8_t idt_index_for_slave, uint8_t idt_index_for_master);
void PIC_sendEOI(uint8_t interrupt_index);
void init_PIT(uint16_t init_value);
void init_PIT_one_shot(uint16_t init_value);

#endif // __PIC_H_
