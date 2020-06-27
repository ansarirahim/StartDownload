/*
 * SimpleGPIO_SPI.h
 *
 *  Created on: Jun 4, 2016
 *      Author: A. R. Ansari
 */

#ifndef SIMPLEGPIO_SPI_H_
#define SIMPLEGPIO_SPI_H_
#define SYSFS_GPIO_DIR "/sys/class/gpio"
#define POLL_TIMEOUT (3 * 1000) /* 3 seconds */
#define MAX_BUF 64
#define SYSFS_OMAP_MUX_DIR "/sys/kernel/debug/omap_mux/"

enum PIN_DIRECTION{
	INPUT_PIN=0,
	OUTPUT_PIN=1
};

enum PIN_VALUE{
	LOW=0,
	HIGH=1
};

/****************************************************************
 * gpio_export
 ****************************************************************/
int gpio_export_spi(unsigned int gpio);
int gpio_unexport_spi(unsigned int gpio);
int gpio_set_dir_spi(unsigned int gpio, PIN_DIRECTION out_flag);
int gpio_set_value_spi(unsigned int gpio, PIN_VALUE value);
int gpio_get_value_spi(unsigned int gpio, unsigned int *value);
int gpio_set_edge_spi(unsigned int gpio, char *edge);
int gpio_fd_open_spi(unsigned int gpio);
int gpio_fd_close_spi(int fd);
int gpio_omap_mux_setup_spi(const char *omap_pin0_name, const char *mode);





#endif /* SIMPLEGPIO_SPI_H_ */
