/*
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation version 2.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <dev_i2c.h>
#include <dev_dma.h>
#include <generated/autoconf.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>           /* everything... */
#include <linux/cdev.h>
#include <linux/mm.h>
#include <linux/i2c.h>
#include <linux/kernel.h>       /* printk() */
#include <linux/slab.h>         /* kmalloc() */
#include <asm/uaccess.h>        /* copy_*_user */
#include <media/davinci/vpfe_capture.h>
extern int dm365evm_setup_video_input(enum vpfe_subdev_id);

int DRV_devInit(void)
{
  int     result;

  result = I2C_devInit();
  if(result!=0)
    return result;

  result = DMA_devInit();
  if(result!=0) {
    I2C_devExit();
    return result;
  }

  #ifdef BOARD_TI_EVM
  // select imager mux on DM365 EVM
  #ifdef YUV_MODE_INTERLACED
 dm365evm_setup_video_input(1);
  #else
 dm365evm_setup_video_input(4);
  #endif
  printk(KERN_INFO "DRV: Sensor interface selected on DM365 EVM\n");
  #endif

  printk(KERN_INFO "DRV: Module install successful\n");
  printk(KERN_INFO "DRV: Module built on %s %s \n", __DATE__, __TIME__);

  {
    volatile unsigned int *pReg;

    pReg 	= (unsigned int *)IO_ADDRESS(0x1c00284); // EDMACC.QUEPRI
    *pReg 	= (*pReg & 0xffff0fff) | (2<<12);  //change TC3 priority to highest for Audio
    printk( KERN_INFO "DRV: EDMACC.QUEPRI  = %08x\n", *pReg );

    pReg 	= (unsigned int *)IO_ADDRESS(0x1c4003c); // SYSTEM.MSTPRI0
    *pReg 	= 0x00440011;
    printk( KERN_INFO "DRV: SYSTEM.MSTPRI0 = %08x\n", *pReg );

    pReg 	= (unsigned int *)IO_ADDRESS(0x1c40040); // SYSTEM.MSTPRI1
    *pReg 	= 0x0444;
    printk( KERN_INFO "DRV: SYSTEM.MSTPRI1 = %08x\n", *pReg );

    pReg 	= (unsigned int *)IO_ADDRESS(0x1c70008); // ISP.BCR
    printk( KERN_INFO "DRV: ISP.BCR        = %08x\n", *pReg );

    pReg 	= (unsigned int *)IO_ADDRESS(0x1c40038); // SYSTEM.MISC
    *pReg 	|= 0x80; //Let the DBS be 64 bytes for TC1
    *pReg 	|= 0x10; //Make Timer2 as WDT
    printk( KERN_INFO "DRV: SYSTEM.MISC    = %08x\n", *pReg );
  }

  return result;
}

void DRV_devExit(void)
{
  I2C_devExit();
  DMA_devExit();
}

module_init(DRV_devInit);
module_exit(DRV_devExit);

MODULE_AUTHOR("Texas Instruments");
MODULE_LICENSE("GPL");
