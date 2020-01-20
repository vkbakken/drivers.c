#ifndef LTE_MODEM_H_INCLUDED
#define LTE_MODEM_H_INCLUDED


#include <stdint.h>
#include <stdbool.h>


bool lte_init(void);

bool lte_configure(void);

bool lte_connect(void);
bool lte_dicconnect(void);
#endif /*LTE_MODEM_H_INCLUDED*/
