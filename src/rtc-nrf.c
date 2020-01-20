#include <stdint.h>
#include <stdbool.h>

#include <nrf.h>

#include "atomic.h"


#define RTC_NVIC_PRIO           (6)
#define RTC_IRQ_n               (RTC1_IRQn)
#define RTC                     (NRF_RTC1_S)

#define COUNTER_SPAN            (1 << 24)
#define COUNTER_MAX             (COUNTER_SPAN - 1U)
#define COUNTER_HALF_SPAN       (COUNTER_SPAN / 2U)


static uint64_t ticks;


void rtc_init(void)
{
        NVIC_ClearPendingIRQ(RTC_IRQ_n);
        ticks = 0;
        RTC->TASKS_STOP = 1;
        RTC->EVTENSET = RTC_EVTENSET_OVRFLW_Msk;
        RTC->INTENSET = RTC_INTENSET_OVRFLW_Msk;
        NVIC_SetPriority(RTC_IRQ_n, RTC_NVIC_PRIO);
        NVIC_EnableIRQ(RTC_IRQ_n);

        RTC->TASKS_CLEAR = 1;
        RTC->TASKS_START = 1;
}


uint64_t rtc_millis(void)
{
        uint32_t mask;
        uint64_t ms;
        
        mask = atomic_enter();
        ms = RTC->COUNTER;
        ms += ticks;
        atomic_exit(mask);

        return ((ms * 1000) / 32768);
}


void RTC1_IRQHandler(void)
{
        uint32_t flag = RTC->EVENTS_OVRFLW;
        if (flag & 0x1) {
                RTC->EVENTS_OVRFLW = 0;
                ticks += COUNTER_SPAN;
        } 

        /*Readback event register to make sure it is written before exit*/
        RTC->EVENTS_OVRFLW;
}