#include <stdint.h>
#include <stdbool.h>

#include <nrf.h>


#define NRF_UART                (NRF_UARTE0_S)
#define SERIAL_IRQ              (UARTE0_SPIM0_SPIS0_TWIM0_TWIS0_IRQn)
#define SERIAL_IRQ_PRIORITY     (6)
#define SERIAL_BAUDRATE         (UARTE_BAUDRATE_BAUDRATE_Baud115200)
#define SERIAL_RX_PIN           (28)
#define SERIAL_TX_PIN           (29)
#define SERIAL_RX_FIFO_SIZE     (32)
#define SERIAL_TX_FIFO_SIZE     (32)


static char rx_data;
static char tx_data;
static char rx_fifo[SERIAL_RX_FIFO_SIZE];
static char tx_fifo[SERIAL_TX_FIFO_SIZE];
static uint8_t fifo_rx_head;
static uint8_t fifo_rx_tail;
static uint8_t fifo_rx_free;
static uint8_t fifo_tx_head;
static uint8_t fifo_tx_tail;
static uint8_t fifo_tx_free;


void serial_init(void)
{
    fifo_rx_head = 0;
    fifo_rx_tail = 0;
    fifo_rx_free = SERIAL_RX_FIFO_SIZE;
    fifo_tx_head = 0;
    fifo_tx_tail = 0;
    fifo_tx_free = SERIAL_TX_FIFO_SIZE;

    NVIC_SetPriority(SERIAL_IRQ, SERIAL_IRQ_PRIORITY);
    NVIC_ClearPendingIRQ(SERIAL_IRQ);

    NRF_UART->CONFIG = UARTE_CONFIG_STOP_One;
    NRF_UART->BAUDRATE = SERIAL_BAUDRATE;
    NRF_UART->PSEL.RXD = SERIAL_RX_PIN;
    NRF_UART->PSEL.TXD = SERIAL_TX_PIN;

    NRF_UART->EVENTS_TXDRDY = 0;
    NRF_UART->EVENTS_ENDRX = 0;

    NRF_UART->RXD.PTR = (uint32_t)&rx_data;
    NRF_UART->RXD.MAXCNT = 1;
    NRF_UART->TXD.PTR = (uint32_t)&tx_data;
    NRF_UART->TXD.MAXCNT = 1;

    NVIC_EnableIRQ(SERIAL_IRQ);

    NRF_UART->INTENSET = UARTE_INTEN_ENDRX_Msk | UARTE_INTEN_ENDTX_Msk;
    NRF_UART->ENABLE = UARTE_ENABLE_ENABLE_Enabled;
    NRF_UART->TASKS_STARTRX = 1;
}


void serial_deinit(void)
{
    NVIC_DisableIRQ(SERIAL_IRQ);
    NRF_UART->INTENCLR = 0xFFFFffff;

    NRF_UART->TASKS_STOPRX = 1;
    NRF_UART->TASKS_STOPTX = 1;
}


bool serial_fifo_fill(char c)
{
    bool status = false;

    /*START ATOMIC*/
    if (fifo_tx_free) {
        --fifo_tx_free;
        tx_fifo[fifo_tx_tail++] = c;

        if (fifo_tx_tail == (SERIAL_TX_FIFO_SIZE - 1)) {
            fifo_tx_tail = 0;
        }

        if (fifo_tx_free == (SERIAL_TX_FIFO_SIZE - 1)) {
            tx_data = c;
            NRF_UART->EVENTS_ENDTX = 0;
            NRF_UART->TASKS_STARTTX = 1;
        }

        status = true;
    }
    /*END ATOMIC*/

    return status;
}


bool serial_fifo_fetch(char *c)
{
    bool status = false;

    /*START ATOMIC*/
    if (fifo_rx_free != SERIAL_RX_FIFO_SIZE) {
        ++fifo_rx_free;
        *c = rx_fifo[fifo_rx_head++];

        if (fifo_rx_head == (SERIAL_RX_FIFO_SIZE - 1)) {
            fifo_rx_head = 0;
        }

        status = true;
    }
    /*END ATOMIC*/

    return status;
}


void UARTE0_SPIM0_SPIS0_TWIM0_TWIS0_IRQHandler(void)
{
    uint32_t event = NRF_UART->EVENTS_ENDRX;
    if (event) {
        /*RX*/
        NRF_UART->EVENTS_ENDRX = 0;

        if (fifo_rx_free) {
            --fifo_rx_free;
            rx_fifo[fifo_rx_tail++] = rx_data;

            if (fifo_rx_tail == (SERIAL_RX_FIFO_SIZE - 1)) {
                fifo_rx_tail = 0;
            }
        }

        NRF_UART->TASKS_STARTRX = 1;
    }

    event = NRF_UART->EVENTS_ENDTX;
    if (event) {
        /*TX*/
        NRF_UART->EVENTS_ENDTX = 0;

        if (fifo_tx_free != SERIAL_TX_FIFO_SIZE) {
            ++fifo_tx_free;
            tx_data = tx_fifo[fifo_tx_head++];
            NRF_UART->TASKS_STARTTX = 1;

            //NRF_UART->TXD = tx_fifo[fifo_tx_head++];
            if (fifo_tx_head == (SERIAL_TX_FIFO_SIZE - 1)) {
                fifo_tx_head = 0;
            }
        }
    }

    /*Read back event flags to make sure IO accesses are done.*/
    NRF_UART->EVENTS_ENDTX;
}
