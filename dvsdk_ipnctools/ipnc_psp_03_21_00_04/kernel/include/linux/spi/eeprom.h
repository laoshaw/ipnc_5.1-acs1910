#ifndef __LINUX_SPI_EEPROM_H
#define __LINUX_SPI_EEPROM_H

#include <linux/memory.h>

/*
 * Put one of these structures in platform_data for SPI EEPROMS handled
 * by the "at25" driver.  On SPI, most EEPROMS understand the same core
 * command set.  If you need to support EEPROMs that don't yet fit, add
 * flags to support those protocol options.  These values all come from
 * the chip datasheets.
 */
struct spi_eeprom {
	u32		byte_len;
	char		name[10];
	u16		page_size;		/* for writes */
	u16		flags;
#define	EE_ADDR1	0x0001			/*  8 bit addrs */
#define	EE_ADDR2	0x0002			/* 16 bit addrs */
#define	EE_ADDR3	0x0004			/* 24 bit addrs */
#define	EE_READONLY	0x0008			/* disallow writes */

	/* for exporting this chip's data to other kernel code */
	void (*setup)(struct memory_accessor *mem, void *context);
	void *context;
};

#define SPI_BUFFER_SIZE SMP_CACHE_BYTES
#define DAVINCI_SPI_TX_CMD_SIZE 3

struct davinci_eeprom_info {
	unsigned int eeprom_size;
	unsigned int page_size;
	unsigned int page_mask;
	unsigned long chip_sel;
	unsigned int commit_delay;
	struct spi_device *spi;
	struct davinci_spi *spi_data;

	struct mtd_partition *parts;
	unsigned int nr_parts;

	struct semaphore lock;
	char tx_buffer[SPI_BUFFER_SIZE + DAVINCI_SPI_TX_CMD_SIZE];
	char rx_buffer[SPI_BUFFER_SIZE];
};

#endif /* __LINUX_SPI_EEPROM_H */
