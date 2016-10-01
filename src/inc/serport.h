#ifndef __SERPORT_H_
#define __SERPORT_H_

#include <stdint.h>

void ser_port_init();

void ser_port_write_char(char something);
void ser_port_write_string(char* something);
void ser_port_write_64(uint64_t something);

#endif // __SERPORT_H_