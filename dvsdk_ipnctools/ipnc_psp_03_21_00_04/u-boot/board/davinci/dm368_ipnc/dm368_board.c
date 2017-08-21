/*
 *
 * Copyright (C) 2008 Texas Instruments.
 *
 * ----------------------------------------------------------------------------
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 * ----------------------------------------------------------------------------
 *
 */
#include <common.h>
#include <i2c.h>
#include <asm/arch/hardware.h>
#include <asm/arch/i2c_defs.h>

#define PLL1_PLLM    *(volatile unsigned int *)0x01c40910
#define PLL1_PREDIV  *(volatile unsigned int *)0x01c40914
#define PLL1_DIV2    *(volatile unsigned char *)0x01c4091C
#define PLL1_POSTDIV *(volatile unsigned int *)0x01C40928
#define PLL1_DIV7    *(volatile unsigned char *)0x01C4096C

#define PLL2_PLLM    *(volatile unsigned int *)0x01c40D10
#define PLL2_PREDIV  *(volatile unsigned int *)0x01C40D14
#define PLL2_DIV2    *(volatile unsigned char *)0x01c40D1C
#define PLL2_DIV3    *(volatile unsigned char *)0x01C40D20
#define PLL2_POSTDIV *(volatile unsigned int *)0x01C40D28
#define SYS_PERI_CLKCTL	*(volatile unsigned int *)0x01C40048
#define PLL1_CKEN	*(volatile unsigned int *)0x01C40948
#define PINMUX4	*(volatile unsigned int *)0x01C40010
#define PINMUX1 *(volatile unsigned int *)0x01C40004
extern int timer_init(void);
extern int eth_hw_init(void);

void EnableClock0Output()
{
	volatile unsigned int i;
	/* Use GPIO37 to ouput 24MHz for CMOS and AIC26 */
	/* Disable output CLKOUT0 */
	SYS_PERI_CLKCTL |= 1;
	/* Short Delay */
	for(i = 0; i < 1000; i++);
	/* Enable output CLKOUT0 */
	SYS_PERI_CLKCTL &= ~1;
	/* PLL1 enable output OBSCLK for CLKOUT0 */
	//PLL1_CKEN |= 0x2;
	/* Set GPIO37 function to CLKOUT0 */
	//PINMUX4 |= 0x00300000;
	
	PINMUX1 |= 0x00000800;
}

/*******************************************
 Routine: board_init
 Description:  Board Initialization routine
*******************************************/
int board_init (void)
{
	DECLARE_GLOBAL_DATA_PTR;
	
	/* arch number of DaVinci DM365 */
	gd->bd->bi_arch_number = 1939;

	/* adress of boot parameters */
	gd->bd->bi_boot_params = LINUX_BOOT_PARAM_ADDR;
        
	/* Set the Bus Priority Register to appropriate value */
	REG(VBPR) = 0x20;
	
	timer_init();
	EnableClock0Output();
	return 0;
}


/******************************
 Routine: misc_init_r
 Description:  Misc. init
******************************/
int misc_init_r (void)
{
	 unsigned int arm_clk,ddr_clk,pll1_clk,pll2_clk;
	
	REG(0x01c4000c) |= 0x15AFFFF;
	pll1_clk = ((PLL1_PLLM & 0x1ff) *2 *24) / ((PLL1_PREDIV & 0x1F) + 1)/((PLL1_POSTDIV & 0x1F) + 1);
	pll2_clk = ((PLL2_PLLM & 0x1ff) *2 *24) / ((PLL2_PREDIV & 0x1F) + 1)/((PLL2_POSTDIV & 0x1F) + 1);
    if (SYS_PERI_CLKCTL & (1<<29)){
	    arm_clk = pll2_clk/((PLL2_DIV2 & 0x1f)+1);	
	}else{
		arm_clk = pll1_clk/((PLL1_DIV2 & 0x1f)+1);	
	}
	if (SYS_PERI_CLKCTL & (1<<27)){
		ddr_clk = pll2_clk / ((PLL2_DIV3 & 0x1F) + 1); 
	}else{
	    ddr_clk = pll1_clk / ((PLL1_DIV7 & 0x1F) + 1);
	}

    printf ("ARM Clock :- %dMHz\n",arm_clk  );
    printf ("DDR Clock :- %dMHz\n", (ddr_clk/2));
	
	if (!eth_hw_init())
		printf("ethernet init failed!\n");

	return (0);
}

/******************************
 Routine: dram_init
 Description:  Memory Info
******************************/
int dram_init (void)
{
        DECLARE_GLOBAL_DATA_PTR;

	      gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	      gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;

        return 0;
}

