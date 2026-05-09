#ifndef CONFIG_H
#define CONFIG_H

#include <xc.h>

// Basic Peripheral Configuration
void input_config(void);
void uart_config(void);
void spi_config(void);

// Accelerometer Functions (BMX055)
void init_accelerometer(void);
void set_bandwidth(int bw_val);

// SPI Utility (Used by both config.c and main.c)
unsigned int spi_write(unsigned int data);

// UART Utility
void send_string(char* str);

#endif