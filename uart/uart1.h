#include "../kernel/gpio.h"

/* Auxilary mini UART (UART1) registers */
#define AUX_ENABLE      (* (volatile unsigned int*)(MMIO_BASE+0x00215004))
#define AUX_MU_IO       (* (volatile unsigned int*)(MMIO_BASE+0x00215040))
#define AUX_MU_IER      (* (volatile unsigned int*)(MMIO_BASE+0x00215044))
#define AUX_MU_IIR      (* (volatile unsigned int*)(MMIO_BASE+0x00215048))
#define AUX_MU_LCR      (* (volatile unsigned int*)(MMIO_BASE+0x0021504C))
#define AUX_MU_MCR      (* (volatile unsigned int*)(MMIO_BASE+0x00215050))
#define AUX_MU_LSR      (* (volatile unsigned int*)(MMIO_BASE+0x00215054))
#define AUX_MU_MSR      (* (volatile unsigned int*)(MMIO_BASE+0x00215058))
#define AUX_MU_SCRATCH  (* (volatile unsigned int*)(MMIO_BASE+0x0021505C))
#define AUX_MU_CNTL     (* (volatile unsigned int*)(MMIO_BASE+0x00215060))
#define AUX_MU_STAT     (* (volatile unsigned int*)(MMIO_BASE+0x00215064))
#define AUX_MU_BAUD     (* (volatile unsigned int*)(MMIO_BASE+0x00215068))
#define AUX_BASE        0x3F215000    // Base for mini UART and SPI1
#define AUX_MU_BASE     (AUX_BASE + 0x40)  // Mini UART registers start at +0x40
#define AUX_MU_LSR_REG  (AUX_MU_BASE + 0x14)  // Line Status Register

/* Function prototypes */
void uart_init(int handshake);
void set_handshake(int enable);
void uart_sendc(char c);
char uart_getc();
void uart_puts( const char *s);
void uart_hex(unsigned int num);
void uart_hex_byte(unsigned char b);
void uart_dec(int num);
unsigned char uart_read();
int uart_char_available();