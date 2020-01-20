#ifndef DRIVERS_RTC_H_INCLUDED
#define DRIVERS_RTC_H_INCLUDED


#include <stdint.h>


void rtc_init(void);
uint64_t rtc_millis(void);
#endif /*DRIVERS_RTC_H_INCLUDED*/
