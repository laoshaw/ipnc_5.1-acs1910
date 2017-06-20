/*
 * TI DaVinci DM365 EVM board support
 *
 * Copyright (C) 2011 Texas Instruments Incorporated
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation version 2.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/err.h>
#include <linux/i2c.h>
#include <linux/io.h>
#include <linux/clk.h>
#include <linux/i2c/at24.h>
#include <linux/i2c/tsc2004.h>
#include <linux/leds.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/slab.h>
#include <linux/mtd/nand.h>
#include <linux/input.h>
#include <linux/spi/spi.h>
#include <linux/spi/eeprom.h>

#include <asm/mach-types.h>
#include <asm/mach/arch.h>

#include <mach/gpio.h>
#include <mach/mux.h>
#include <mach/dm365.h>
#include <mach/common.h>
#include <mach/i2c.h>
#include <mach/serial.h>
#include <mach/mmc.h>
#include <mach/nand.h>
#include <mach/gpio.h>
#include <mach/cputype.h>
#include <mach/keyscan.h>
#include <mach/psc.h>

#include <media/tvp514x.h>
#include <media/tvp7002.h>
#include <media/davinci/videohd.h>

/* have_imager() - Check if we have support for imager interface */
static inline int have_imager(void)
{
#if defined(CONFIG_SOC_CAMERA_MT9P031) || \
	defined(CONFIG_SOC_CAMERA_MT9P031_MODULE)
	return 1;
#else
	return 0;
#endif
}


#define DM365_EVM_PHY_ID		"0:01"


static void __iomem *cpld;

#define NAND_BLOCK_SIZE		SZ_16K

static struct mtd_partition davinci_nand_partitions[] = {
	{
		/* UBL (a few copies) plus U-Boot */
		.name		= "bootloader",
		.offset		= 0,
		.size		= 32 * NAND_BLOCK_SIZE,
		.mask_flags	= 0, /* force read-only */
	}, {
		/* U-Boot environment */
		.name		= "params",
		.offset		= MTDPART_OFS_APPEND,
		.size		= 56 * NAND_BLOCK_SIZE,
		.mask_flags	= 0,
	}, {
		.name		= "kernel",
		.offset		= MTDPART_OFS_APPEND,
	    .size		= SZ_2M + SZ_1M + 48 * NAND_BLOCK_SIZE,
		.mask_flags	= 0,
	}, {
		.name		= "filesystem1",
		.offset		= MTDPART_OFS_APPEND,
		.size		= SZ_16M + SZ_8M + SZ_1M + 88 * NAND_BLOCK_SIZE,
		.mask_flags	= 0,
	}, {
		.name		= "filesystem2",
		.offset		= MTDPART_OFS_APPEND,
		.size		= SZ_1M/2,
		.mask_flags	= 0,
	}
	/* two blocks with bad block table (and mirror) at the end */
};

static struct davinci_nand_pdata davinci_nand_data = {
	.parts			= davinci_nand_partitions,
	.nr_parts		= ARRAY_SIZE(davinci_nand_partitions),
	.ecc_mode		= NAND_ECC_HW,
	.options		= NAND_USE_FLASH_BBT,
	.ecc_bits		= 4,
};

static struct resource davinci_nand_resources[] = {
	{
		.start		= DM365_ASYNC_EMIF_DATA_CE0_BASE,
		.end		= DM365_ASYNC_EMIF_DATA_CE0_BASE + SZ_32M - 1,
		.flags		= IORESOURCE_MEM,
	}, {
		.start		= DM365_ASYNC_EMIF_CONTROL_BASE,
		.end		= DM365_ASYNC_EMIF_CONTROL_BASE + SZ_4K - 1,
		.flags		= IORESOURCE_MEM,
	},
};

static struct platform_device davinci_nand_device = {
	.name			= "davinci_nand",
	.id			= 0,
	.num_resources		= ARRAY_SIZE(davinci_nand_resources),
	.resource		= davinci_nand_resources,
	.dev			= {
		.platform_data	= &davinci_nand_data,
	},
};

static struct snd_platform_data dm365_evm_snd_data = {
	.asp_chan_q = EVENTQ_3,
};


static struct davinci_i2c_platform_data i2c_pdata = {
	.bus_freq	= 400	/* kHz */,
	.bus_delay	= 0	/* usec */,
};

static int dm365evm_keyscan_enable(struct device *dev)
{
	return davinci_cfg_reg(DM365_KEYSCAN);
}

static unsigned short dm365evm_keymap[] = {
	KEY_KP2,
	KEY_LEFT,
	KEY_EXIT,
	KEY_DOWN,
	KEY_ENTER,
	KEY_UP,
	KEY_KP1,
	KEY_RIGHT,
	KEY_MENU,
	KEY_RECORD,
	KEY_REWIND,
	KEY_KPMINUS,
	KEY_STOP,
	KEY_FASTFORWARD,
	KEY_KPPLUS,
	KEY_PLAYPAUSE,
	0
};

static struct davinci_ks_platform_data dm365evm_ks_data = {
	.device_enable	= dm365evm_keyscan_enable,
	.keymap		= dm365evm_keymap,
	.keymapsize	= ARRAY_SIZE(dm365evm_keymap),
	.rep		= 1,
	/* Scan period = strobe + interval */
	.strobe		= 0x5,
	.interval	= 0x2,
	.matrix_type	= DAVINCI_KEYSCAN_MATRIX_4X4,
};
#define GPIO85 85
static int cpld_mmc_get_ro(int module)
{
	int val;
	gpio_request(GPIO85,"mmc_wp");
	gpio_direction_input(GPIO85);
	val = gpio_get_value(GPIO85);
	return !!val;
}

static struct davinci_mmc_config dm365evm_mmc_config = {
	.get_ro		= cpld_mmc_get_ro,
	.wires		= 4,
	.max_freq	= 50000000,
	.caps		= MMC_CAP_MMC_HIGHSPEED | MMC_CAP_SD_HIGHSPEED,
	.version	= MMC_CTLR_VERSION_2,
};

static void dm365evm_emac_configure(void)
{
	/*
	 * EMAC pins are multiplexed with GPIO and UART
	 * Further details are available at the DM365 ARM
	 * Subsystem Users Guide(sprufg5.pdf) pages 125 - 127
	 */
	davinci_cfg_reg(DM365_EMAC_TX_EN);
	davinci_cfg_reg(DM365_EMAC_TX_CLK);
	davinci_cfg_reg(DM365_EMAC_COL);
	davinci_cfg_reg(DM365_EMAC_TXD3);
	davinci_cfg_reg(DM365_EMAC_TXD2);
	davinci_cfg_reg(DM365_EMAC_TXD1);
	davinci_cfg_reg(DM365_EMAC_TXD0);
	davinci_cfg_reg(DM365_EMAC_RXD3);
	davinci_cfg_reg(DM365_EMAC_RXD2);
	davinci_cfg_reg(DM365_EMAC_RXD1);
	davinci_cfg_reg(DM365_EMAC_RXD0);
	davinci_cfg_reg(DM365_EMAC_RX_CLK);
	davinci_cfg_reg(DM365_EMAC_RX_DV);
	davinci_cfg_reg(DM365_EMAC_RX_ER);
	davinci_cfg_reg(DM365_EMAC_CRS);
	davinci_cfg_reg(DM365_EMAC_MDIO);
	davinci_cfg_reg(DM365_EMAC_MDCLK);

	/*
	 * EMAC interrupts are multiplexed with GPIO interrupts
	 * Details are available at the DM365 ARM
	 * Subsystem Users Guide(sprufg5.pdf) pages 133 - 134
	 */
	davinci_cfg_reg(DM365_INT_EMAC_RXTHRESH);
	davinci_cfg_reg(DM365_INT_EMAC_RXPULSE);
	davinci_cfg_reg(DM365_INT_EMAC_TXPULSE);
	davinci_cfg_reg(DM365_INT_EMAC_MISCPULSE);
}


#define VENC_STD_ALL	(V4L2_STD_NTSC | V4L2_STD_PAL)

/* venc standards timings */
static struct vpbe_enc_mode_info vbpe_enc_std_timings[] = {
	{
		.name		= "ntsc",
		.timings_type	= VPBE_ENC_STD,
		.timings	= {V4L2_STD_525_60},
		.interlaced	= 1,
		//.xres		= 640,
		.xres		= 720,
		.yres		= 480,
		.aspect		= {11, 10},
		.fps		= {30000, 1001},
		.left_margin	= 0x79,
		.right_margin	= 0,
		.upper_margin	= 0x10,
		.lower_margin	= 0,
		.hsync_len	= 0,
		.vsync_len	= 0,
		.flags		= 0,
	},
	{
		.name		= "pal",
		.timings_type	= VPBE_ENC_STD,
		.timings	= {V4L2_STD_625_50},
		.interlaced	= 1,
		.xres		= 720,
		.yres		= 576,
		.aspect		= {54, 59},
		.fps		= {25, 1},
		.left_margin	= 0x7E,
		.right_margin	= 0,
		.upper_margin	= 0x16,
		.lower_margin	= 0,
		.hsync_len	= 0,
		.vsync_len	= 0,
		.flags		= 0,
	},
};

/* venc dv preset timings */
static struct vpbe_enc_mode_info vbpe_enc_preset_timings[] = {
	{
		.name		= "480p59_94",
		.timings_type	= VPBE_ENC_DV_PRESET,
		.timings	= {V4L2_DV_480P59_94},
		.interlaced	= 0,
		.xres		= 720,
		.yres		= 480,
		.aspect		= {1, 1},
		.fps		= {5994, 100},
		.left_margin	= 0x8F,
		.right_margin	= 0,
		.upper_margin	= 0x2D,
		.lower_margin	= 0,
		.hsync_len	= 0,
		.vsync_len	= 0,
		.flags		= 0,
	},
	{
		.name		= "576p50",
		.timings_type	= VPBE_ENC_DV_PRESET,
		.timings	= {V4L2_DV_576P50},
		.interlaced	= 0,
		.xres		= 720,
		.yres		= 576,
		.aspect		= {1, 1},
		.fps		= {50, 1},
		.left_margin	= 0x8C,
		.right_margin	= 0,
		.upper_margin	= 0x36,
		.lower_margin	= 0,
		.hsync_len	= 0,
		.vsync_len	= 0,
		.flags		= 0,
	},
	{
		.name		= "720p60",
		.timings_type	= VPBE_ENC_DV_PRESET,
		.timings	= {V4L2_DV_720P60},
		.interlaced	= 0,
		.xres		= 1280,
		.yres		= 720,
		.aspect		= {1, 1},
		.fps		= {60, 1},
		.left_margin	= 0x117,
		.right_margin	= 70,
		.upper_margin	= 38,
		.lower_margin	= 3,
		.hsync_len	= 80,
		.vsync_len	= 5,
		.flags		= 0,
	},
	{
		.name		= "1080i30",
		.timings_type	= VPBE_ENC_DV_PRESET,
		.timings	= {V4L2_DV_1080I30},
		.interlaced	= 1,
		.xres		= 1920,
		.yres		= 1080,
		.aspect		= {1, 1},
		.fps		= {30, 1},
		.left_margin	= 0xc9,
		.right_margin	= 80,
		.upper_margin	= 30,
		.lower_margin	= 3,
		.hsync_len	= 88,
		.vsync_len	= 5,
		.flags		= 0,
	},
};

/* custom timings */
static struct vpbe_enc_mode_info vbpe_enc_custom_timings[] = {
	{
		.name		= "272p60",
		.timings_type	= VPBE_ENC_CUSTOM_TIMINGS,
		.timings	= {CUSTOM_TIMING_480_272},
		.interlaced	= 0,
		.xres		= 480,
		.yres		= 272,
		.aspect		= {1, 1},
		.fps		= {60, 1},
		.left_margin	= 43,
		.right_margin	= 43,
		.upper_margin	= 12,
		.lower_margin	= 12,
		.hsync_len	= 42,
		.vsync_len	= 10,
		.flags		= 0
	},
};

/*
 * The outputs available from VPBE + ecnoders. Keep the
 * the order same as that of encoders. First those from venc followed by that
 * from encoders. Index in the outpuvpbe-t refers to index on a particular
 * encoder.Driver uses this index to pass it to encoder when it supports more
 * than one output. Application uses index of the array to set an output.
 */
static struct vpbe_output dm365_vpbe_outputs[] = {
	{
		.output		= {
			.index		= 0,
			.name		= "Composite",
			.type		= V4L2_OUTPUT_TYPE_ANALOG,
			.std		= VENC_STD_ALL,
			.capabilities	= V4L2_OUT_CAP_STD,
		},
		.subdev_name	= VPBE_VENC_SUBDEV_NAME,
		.default_mode	= "ntsc",
		.num_modes	= ARRAY_SIZE(vbpe_enc_std_timings),
		.modes		= vbpe_enc_std_timings,
		.if_params	= V4L2_MBUS_FMT_FIXED,
	},
	{
		.output		= {
			.index		= 1,
			.name		= "Component",
			.type		= V4L2_OUTPUT_TYPE_ANALOG,
			.capabilities	= V4L2_OUT_CAP_PRESETS,
		},
		.subdev_name	= VPBE_VENC_SUBDEV_NAME,
		.default_mode	= "480p59_94",
		.num_modes	= ARRAY_SIZE(vbpe_enc_preset_timings),
		.modes		= vbpe_enc_preset_timings,
		.if_params	= V4L2_MBUS_FMT_FIXED,
	},
	{
		.output         = {
			.index          = 2,
			.name           = "Lcdout",
			.type           = V4L2_OUTPUT_TYPE_MODULATOR,
			.capabilities   = V4L2_OUT_CAP_CUSTOM_TIMINGS,
		},
		.subdev_name    = VPBE_VENC_SUBDEV_NAME,
		.default_mode   = "272p60",
		.num_modes      = ARRAY_SIZE(vbpe_enc_custom_timings),
		.modes          = vbpe_enc_custom_timings,
		.if_params      = V4L2_MBUS_FMT_RGB565_2X8_BE,
	},
};

static struct vpbe_display_config vpbe_display_cfg = {
	.module_name	= "dm365-vpbe-display",
	.i2c_adapter_id	= 1,
	.osd		= {
		.module_name	= VPBE_OSD_SUBDEV_NAME,
	},
	.venc		= {
		.module_name	= VPBE_VENC_SUBDEV_NAME,
	},
	.num_outputs	= ARRAY_SIZE(dm365_vpbe_outputs),
	.outputs	= dm365_vpbe_outputs,
};

static void dm365evm_usb_configure(void)
{
	davinci_cfg_reg(DM365_GPIO33);
	gpio_request(33, "usb");
	gpio_direction_output(33, 1);
	davinci_setup_usb(500, 8);
}

static void __init evm_init_i2c(void)
{
	davinci_init_i2c(&i2c_pdata);
}

static struct platform_device *dm365_evm_nand_devices[] __initdata = {
	&davinci_nand_device,
};

static inline int have_leds(void)
{
#ifdef CONFIG_LEDS_CLASS
	return 1;
#else
	return 0;
#endif
}



static void __init evm_init_emif(void)
{
	struct clk *aemif_clk;
	struct davinci_soc_info *soc_info = &davinci_soc_info;

	/* Make sure we can configure the CPLD through CS1.  Then
	 * leave it on for later access to MMC and LED registers.
	 */
	aemif_clk = clk_get(NULL, "aemif");
	if (IS_ERR(aemif_clk))
		return;
	clk_enable(aemif_clk);

	if (request_mem_region(DM365_ASYNC_EMIF_DATA_CE1_BASE, SECTION_SIZE,
			"cpld") == NULL)
		goto fail;
	cpld = ioremap(DM365_ASYNC_EMIF_DATA_CE1_BASE, SECTION_SIZE);
	if (!cpld) {
		release_mem_region(DM365_ASYNC_EMIF_DATA_CE1_BASE,
				SECTION_SIZE);
fail:
		pr_err("ERROR: can't map CPLD\n");
		clk_disable(aemif_clk);
		return;
	}
		platform_add_devices(dm365_evm_nand_devices,
				ARRAY_SIZE(dm365_evm_nand_devices));
	/* ... and ENET ... */
	dm365evm_emac_configure();
	soc_info->emac_pdata->phy_id = DM365_EVM_PHY_ID;


	/* REVISIT export switches: NTSC/PAL (SW5.6), EXTRA1 (SW5.2), etc */
}


static struct davinci_uart_config uart_config __initdata = {
	.enabled_uarts = (7 << 0),
	//.enabled_uarts = (1 << 1),
};

static void __init dm365_evm_map_io(void)
{
	dm365_set_vpbe_display_config(&vpbe_display_cfg);
	dm365_init();
}


static void setup_sensor(void)
{
        unsigned int temp1 = 0;

        void __iomem *pll1_cken = (void __iomem *) IO_ADDRESS(0x01C40948);
        void __iomem *pinmux1 = (void __iomem *) IO_ADDRESS(0x01C40004);


        temp1 = __raw_readl(pinmux1);  //select gio91
        temp1 = temp1 & ~(3<<2);
        temp1 = temp1 | (3<<2);
        __raw_writel(temp1, pinmux1);

        /* Configure GPIO91 as an output */
	gpio_request(91, "camera_reset");
        gpio_direction_output(91, 0);
        gpio_set_value(91, 1);

	davinci_cfg_reg(DM365_CLKOUT0);
        temp1 = __raw_readl(pll1_cken);
        temp1 = temp1 | 0x2;
        __raw_writel(temp1, pll1_cken);
}
void enable_lcd(void)
{
	/*dummy function to resolve compilation error. lcd is not connected on dm365 ipnc*/
}
EXPORT_SYMBOL(enable_lcd);
void enable_hd_clk(void)
{
	/* dummy function to resolve compilation issue*/
}
EXPORT_SYMBOL(enable_hd_clk);

static __init void dm365_evm_init(void)
{
	evm_init_i2c();
        setup_sensor();
	davinci_serial_init(&uart_config);
	dm365evm_emac_configure();

	davinci_setup_mmc(0, &dm365evm_mmc_config);

	/* maybe setup mmc1/etc ... _after_ mmc0 */
	evm_init_emif();

#ifdef CONFIG_SND_DM365_AIC3X_CODEC
	dm365_init_asp(&dm365_evm_snd_data);
#elif defined(CONFIG_SND_DM365_VOICE_CODEC)
	dm365_init_vc(&dm365_evm_snd_data);
#endif
	dm365_init_rtc();
	dm365_init_ks(&dm365evm_ks_data);
	davinci_psc_config(DAVINCI_GPSC_ARMDOMAIN, 0, DM365_LPSC_VPSSMSTR, PSC_STATE_SYNCRST, 1);
	davinci_psc_config(DAVINCI_GPSC_ARMDOMAIN, 0, DM365_LPSC_VPSSMSTR, PSC_STATE_ENABLE, 1);
	dm365evm_usb_configure();

}

MACHINE_START(DAVINCI_DM365_EVM, "DaVinci DM36x IPNC")
	.boot_params	= (0x80000100),
	.map_io		= dm365_evm_map_io,
	.init_irq	= davinci_irq_init,
	.timer		= &davinci_timer,
	.init_machine	= dm365_evm_init,
MACHINE_END

