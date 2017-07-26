#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/ioport.h>
#include <linux/miscdevice.h> 
#include <linux/sched.h> 
#include <linux/delay.h> 
#include <linux/poll.h> 
#include <linux/spinlock.h> 
#include <linux/delay.h> 
#include <linux/wait.h> 
#include <linux/delay.h>
#include <asm/io.h> 
#include <mach/hardware.h>  

#include "DM368ADC.h"


/* ------------------------------------------------------------------------ *
 *  ADC Definitions                                                         *
 * ------------------------------------------------------------------------ */

//#define ADC_BASE                        0x01C23C00

#define ADC_BASE                        base_addr
#define PWM0_BASE												pwm0_base_addr

#define PWM0_PID												*(volatile Uint32 *)(PWM0_BASE + 0x00)

#define ADC_ADCTL        		*( volatile Uint32* )( ADC_BASE + 0x00 ) // Control register
#define ADC_CMPTGT        		*( volatile Uint32* )( ADC_BASE + 0x04 ) // Comparator target channel
#define ADC_CMPLDAT        		*( volatile Uint32* )( ADC_BASE + 0x08 ) // Comparison A/D Lower data(7:0)
#define ADC_CMPUDAT        		*( volatile Uint32* )( ADC_BASE + 0x0C ) // Comparison A/D Upper data(7:0)
#define ADC_SETDIV        		*( volatile Uint32* )( ADC_BASE + 0x10 ) // SETUP divide value for start A/D conversion
#define ADC_CHSEL        		*( volatile Uint32* )( ADC_BASE + 0x14 ) // Analog Input channel select
#define ADC_AD0DAT        		*( volatile Uint32* )( ADC_BASE + 0x18 ) // A/D conversion data 0
#define ADC_AD1DAT        		*( volatile Uint32* )( ADC_BASE + 0x1c ) // A/D conversion data 1
#define ADC_AD2DAT        		*( volatile Uint32* )( ADC_BASE + 0x20 ) // A/D conversion data 2
#define ADC_AD3DAT        		*( volatile Uint32* )( ADC_BASE + 0x24 ) // A/D conversion data 3
#define ADC_AD4DAT        		*( volatile Uint32* )( ADC_BASE + 0x28 ) // A/D conversion data 4
#define ADC_AD5DAT        		*( volatile Uint32* )( ADC_BASE + 0x2c ) // A/D conversion data 5
#define ADC_EMUCTRL        		*( volatile Uint32* )( ADC_BASE + 0x30 ) // emulator control

static void __iomem * base_addr;  
static void __iomem * pwm0_base_addr;

#define DEVICE_NAME "adc_device"

/* ------------------------------------------------------------------------ *
 *                                                                          *
 *  _ADC_init( )                                                            *
 *                                                                          *
 *      Initialize ADC                                                      *
 *                                                                          *
 * ------------------------------------------------------------------------ */
int EVMDM365_ADC_init( )
{
    ADC_CMPTGT  = 0x00;                   // comparator not used  *** Assuming comparator is not required***
    ADC_CMPLDAT = 0x00;                   // Comparator lower data is not used
    ADC_CMPUDAT = 0x00;                   // Comparator upper data is not used
    ADC_SETDIV  = 0xFFFF;                 // Default value for SETDIV
                                          // SETUP clock period = Peripheral CLK period * (SET_DIV[15:0] + 1)
                                          // Conversion clock period = Peripheral CLK period * (SET_DIV[5:0] + 1)
    ADC_CHSEL   = 0x3F;                   // All six channels are selected
    ADC_EMUCTRL = 0x03;                   // EMUCTRL default values used
    ADC_ADCTL   = 0x01;                   // 6: cleared the comparator interrupt flag, 5: Comparator interrupt disabled, 
                                          // 4: Comparator mode ADC input data < CMPLDAT, 3:Scan interrupt flag cleared, 
                                          // 2: Scan interrupt disabled, 1: Scan mode is One-Shot, 0: Start ADC
    return 0;
}

/* ------------------------------------------------------------------------ *
 *                                                                          *
 *  _ADC_readChannel( )                                                     *
 *                                                                          *
 *      Get the ADC value from a specific channel                           *
 *                                                                          *
 * ------------------------------------------------------------------------ */
int EVMDM365_ADC_readChannel(int c)
{
    int data;
    if((c>5) | (c<0))
    return(-1);                       
    ADC_CHSEL  = (0x01 << c);
    ADC_ADCTL   = 0x01; 
    while((ADC_ADCTL & 0x80) != 0)msleep(2);             // Check the busy bit

        switch (c)                      // Select  channel and get the data from it. 
        {
            case 0: 
                data = ADC_AD0DAT;
                break ;
            case 1:
                data = ADC_AD1DAT;
                break ;
            case 2: 
                data = ADC_AD2DAT;
                break ;
            case 3: 
                data = ADC_AD3DAT;
                break ;
            case 4:
                data = ADC_AD4DAT;
                break ;
            case 5: 
                data = ADC_AD5DAT;
                break ;
        }
	
    return(data);
}

static ssize_t adc_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	int data[6];
	int chnum;
	unsigned int retVal;
	for(chnum=0;chnum<count/sizeof(int);chnum++)
	{
		data[chnum]=EVMDM365_ADC_readChannel(chnum);
	}
	if(copy_to_user(buf,data,count))
			return -EFAULT;
	retVal = PWM0_PID;
	printk("PWM0_PID: 0x%08X\n", retVal);
	return count;
}

static ssize_t adc_write(struct file * file, const char __user * buf, size_t count, loff_t * off)
{
	unsigned long tmp;
	struct ADC_DEV adcdev;
	
	copy_from_user(&adcdev,(struct ADC_DEV *)buf,sizeof(struct ADC_DEV));
	
	return 0;
}

static int adc_open(struct inode * inode, struct file * filp)
{
	int channel_num;
	static void __iomem * pwm0_addr;
	
 	EVMDM365_ADC_init( );
        // Clearing ADC data
	for (channel_num=0;channel_num<6;channel_num++)
  {
        	EVMDM365_ADC_readChannel(channel_num);
  }

  printk("=======open: PWM0_PID = 0x%08X\n", PWM0_PID);
	return 0;
}

static int adc_release(struct inode * inode, struct file * filp)
{
	return 0;
}

static struct file_operations adc_fops = {
	 
	.owner = THIS_MODULE,
	.write = adc_write, 
	.read = adc_read,
	.open  = adc_open,	
	.release=adc_release,
}; 

static struct miscdevice misc = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = DEVICE_NAME,
    .fops = &adc_fops,
};

static int __init adc_init(void)
{
	int ret;  
       printk("DM365_ADC init NOW...\n");
       base_addr =ioremap(0x01C23C00,0xff);//地址映射    
       pwm0_base_addr = ioremap(0x01C22000, 0x1000); 
       
     if(base_addr == NULL)    
     {    
       printk(KERN_ERR"failed to remap\n");    
       return -ENOMEM;    
     }   
    printk("ioremap sucessed!\n"); 

    ret = misc_register(&misc);
    if (ret < 0) 
    {
        printk(DEVICE_NAME " can't register\n");
      
        return ret;
    }
    printk("DM365_ADC driver initial ok\n");

   printk("=======init: PWM0_PID = 0x%08X\n", PWM0_PID);

    
	return 0;
}

static void __exit adc_exit(void)
{
	misc_deregister(&misc);
	iounmap(base_addr);//取消映射    
       printk("DM365_adc driver removed\n"); 
}

module_init(adc_init);
module_exit(adc_exit);

MODULE_ALIAS("adc"); 
MODULE_DESCRIPTION("ADC  Driver For DM365");
MODULE_AUTHOR("WangJian");
MODULE_LICENSE("GPL");
