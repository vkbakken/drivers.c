#ifndef SERIAL_H_INCLUDED
#define SERIAL_H_INCLUDED


#include <stdint.h>
#include <stdbool.h>


void serial_init(void);
void serial_deinit(void);
bool serial_fifo_fill(char c);
bool serial_fifo_fetch(char *c);
#endif /*SERIAL_H_INCLUDED*/
