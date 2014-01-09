/*
 * Copyright 2012 (c) Barnes & Noble Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <part_efi.h>
#include <mmc.h>
#include <fastboot.h>
#include <asm/arch/sys_info.h>
#include <omap4_dsi.h>

#include <bn_boot.h>
#include "menu.h"

static const struct efi_partition_info partitions[] = {
	/* name       start_kb    size_kb */
	{ "xloader",       128,       128 }, /* xloader must start at 128 KB */
	{ "bootloader",      0,       256 },
	{ "recovery",        0,     15872 },
	{ "boot",      16*1024,   16*1024 },
	{ "rom",             0,   48*1024 },
	{ "bootdata",        0,   48*1024 },
	{ "factory",         0,  370*1024 },
	{ "system",          0,  612*1024 },
	{ "cache",           0,  426*1024 },
	{ "media",           0, 1024*1024 },
	{ "userdata",        0,         0 }, /* autosize to the end */
	{},
};

int fastboot_oem(const char *cmd)
{
	if (!strcmp(cmd, "format"))
		return efi_do_format(partitions, CFG_FASTBOOT_MMC_NO);
	return -1;
}

void board_mmc_init(void)
{
	/* nothing to do this early */
}

static struct omap_dsi_panel panel_lg = {
	.xres 		= 900,
	.yres 		= 1440,

	.dsi_data = {
		.pxl_fmt	= OMAP_PXL_FMT_RGB666_PACKED,
		.hsa		= 0,
		.hbp		= 41,
		.hfp		= 17,

		.vsa		= 18,
		.vfp 		= 14,
		.vbp		= 3,

		.window_sync	= 4,

		.regm 		= 250,
		.regn 		= 24,
		.regm_dsi 	= 5,
		.regm_dispc	= 9,
		.lp_div		= 6,
		.tl		= 567,
		.vact		= 1440,
		.line_bufs	= 2,
		.bus_width	= 1,

		.hsa_hs_int	= 72,
		.hfp_hs_int	= 114,
		.hbp_hs_int	= 150,

		.hsa_lp_int	= 130,
		.hfp_lp_int	= 223,
		.hbp_lp_int	= 59,

		.bl_lp_int	= 0x31d1,
		.bl_hs_int	= 0x7a67,

		.enter_lat	= 14,
		.exit_lat	= 14,

		.ths_prepare 	= 16,
		.ths_zero 	= 21,
		.ths_trail	= 17,
		.ths_exit	= 29,
		.tlpx		= 10,
		.tclk_trail	= 14,
		.tclk_zero	= 53,
		.tclk_prepare	= 13,
	},

	.dispc_data = {
		.hsw 		= 28,
		.hfp		= 32,
		.hbp		= 48,

		.vsw		= 18,
		.vfp		= 14,
		.vbp		= 3,

		.pcd		= 1,
		.lcd		= 1,
		.acbi		= 0,
		.acb		= 0,

		.row_inc	= 112,
	},
};

static struct omap_dsi_panel panel_auo = {
	.xres 		= 900,
	.yres 		= 1440,

	.dsi_data = {
		.pxl_fmt	= OMAP_PXL_FMT_RGB666_PACKED,
		.hsa		= 0,
		.hbp		= 43,
		.hfp		= 42,

		.vsa		= 9,
		.vfp 		= 10,
		.vbp		= 1,

		.window_sync	= 4,

		.regm 		= 260,
		.regn 		= 24,
		.regm_dsi 	= 5,
		.regm_dispc	= 9,
		.lp_div		= 7,
		.tl		= 594,
		.vact		= 1440,
		.line_bufs	= 2,
		.bus_width	= 1,

		.hsa_hs_int	= 72,
		.hfp_hs_int	= 114,
		.hbp_hs_int	= 150,

		.hsa_lp_int	= 130,
		.hfp_lp_int	= 223,
		.hbp_lp_int	= 59,

		.bl_lp_int	= 0x31d1,
		.bl_hs_int	= 0x7a67,

		.enter_lat	= 18,
		.exit_lat	= 15,

		.ths_prepare 	= 17,
		.ths_zero 	= 22,
		.ths_trail	= 17,
		.ths_exit	= 31,
		.tlpx		= 11,
		.tclk_trail	= 15,
		.tclk_zero	= 55,
		.tclk_prepare	= 14,
	},

	.dispc_data = {
		.hsw 		= 40,
		.hfp		= 76,
		.hbp		= 40,

		.vsw		= 9,
		.vfp		= 10,
		.vbp		= 1,

		.pcd		= 1,
		.lcd		= 1,
		.acbi		= 0,
		.acb		= 0,

		.row_inc	= 112,
	},
};

extern uint16_t const _binary_cyanoboot_rle_start[];
extern uint16_t const _binary_lowbatt_charge_rle_start[];

uint32_t FB = 0xb2600000;
int panel_has_enabled = 0;
int is_lg = 1;
static int bootmode = -1;

extern void panel_enable(u8, u8);

void disable_panel_backlight(void){

     if(panel_has_enabled)
     {
	     panel_enable(0, is_lg);
	     backlight_enable(0);
     }
}

extern struct img_info bootimg_info;

int board_late_init(void)
{
	// For hummingbird hard code to 18 bit panel for now
	// this works for both AUO & LG but there's some visual
	// artifacts on the AUO case
	const char *display_vendor = NULL;
	struct omap_dsi_panel *panel = &panel_lg;
	int display_painted =0; /* boot */
	enum omap_dispc_format fmt = OMAP_XRGB888_FMT;

	load_display_vendor();
	load_serial_num();
	display_vendor = getenv("display_vendor");

	if ( (display_vendor != NULL) && (strstr(display_vendor, "AUO") != NULL)) {
		printf("switching to AUO settings\n");
		panel = &panel_auo;
		is_lg = 0;
	}
	//panel_enable(0, is_lg);

	memset(FB, 0, panel->xres * panel->yres *2);
	bootimg_info.width = panel->xres;
	bootimg_info.height = panel->yres;
	bootimg_info.bg_color = 0;

	backlight_set_brightness(0);
	fmt = OMAP_RGB565_FMT;
	display_init(panel, (void *) FB, fmt);
	panel_has_enabled = 1;
	//panel_enable(1, is_lg);
	
	backlight_set_brightness(0x3f);
	
	show_image(boot);
	
	lcd_console_init();

	// Superhack (must read sdcard first to init the part table
	run_command("mmcinit 0; fatload mmc 0:1 0x80000000 stuff 4", 0);
	
	bootmode = set_boot_mode();
	
	return bootmode;
}

void show_image(ppz_images image_name)
{
	const char *display_vendor = NULL;
	struct omap_dsi_panel *panel = &panel_lg;

	display_vendor = getenv("display_vendor");

	if ( (display_vendor != NULL) && (strstr(display_vendor, "AUO") != NULL)) {
		printf("switching to AUO settings\n");
		panel = &panel_auo;
		is_lg = 0;
	}

	display_rle(_binary_cyanoboot_rle_start, (uint16_t *) FB, panel->xres, panel->yres);
}

void turn_panel_off()
{
	if(panel_has_enabled) {
		backlight_enable(0);
		lp855x_restore_i2c();
		panel_has_enabled = 0;
	}
}

void turn_panel_on()
{
	if(!panel_has_enabled) {
		backlight_enable(1);
		backlight_set_brightness(0xc8);
		panel_has_enabled = 1;
	}
}

int panel_is_enabled()
{
	return panel_has_enabled;
}
