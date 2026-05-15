#ifndef XC_HEADER_TEMPLATE_H
#define	XC_HEADER_TEMPLATE_H

#include <xc.h> // include processor files - each processor file is guarded.  

void input_config(void);
void uart_config(void);
void spi_config(void);
void adc_config(int sampling, int conversion, int n_tad, int tad_length, int scan, int scan_mode, int n_channels);

#endif	/* XC_HEADER_TEMPLATE_H */

