/*
 * Original Boot Menu code by j3mm3r
 * (C) Copyright 2011 j3mm3r
 * 1.2 Enhancements/NT port by fattire & Rebellos
 * (C) Copyright 2011-2012 The CyanogenMod Project
 *
 *
 * See file CREDITS for list of more people who contributed to this
 * project.
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
#include "menu.h"
#include "console.h"

#include <common.h>
#include <twl6030.h>
#include <asm/io.h>

#define KBD_CTRL 0x4A31C028
#define KBD_ROWINPUTS 0x4A31C03C
#define KBD_COLUMNOUTPUTS 0x4A31C040
#define KBD_FULLCODE31_0 0x4A31C044

#define INDENT		16
#define MENUTOP		22

#define HOME_BUTTON		32
#define POWER_BUTTON	29

#define COL0 174
#define ROW0 178 //VOLUP
#define ROW1 2	//VOLDOWN
#define N_KEY (1 << 0)
#define POWER_KEY (1 << 1)
#define VOLUP_KEY (1 << 2)
#define VOLDOWN_KEY (1 << 3)

#define NUM_OPTS		8  //number of boot options
char *opt_list[NUM_OPTS] = { 
		" Boot from Internal Storage ",
		" Boot from SDCARD           ",
		" Boot SD SD Stock           ",
		" Boot CWM Recovery          ",
		"                            ",
		" Default Boot:              ", 
		"       Device: ", 
		"        Image: ", };

// Shared sprintf buffer for fatsave/load
static char buf[64];

extern char lcd_is_enabled;

unsigned char get_keys_pressed(unsigned char* key) {
	(*key) = 0;

	if (gpio_read(HOME_BUTTON) == 0)
		(*key) |= N_KEY;
	if (gpio_read(POWER_BUTTON) == 1)
		(*key) |= POWER_KEY;
	if (gpio_read(ROW0) == 0)
		(*key) |= VOLUP_KEY;
	if (gpio_read(ROW1) == 0)
		(*key) |= VOLDOWN_KEY;
	return (*key);
}

int check_hybrid_image(const char* file) {
    char res = 1; //0=sdcard, 1=emmc
    char vax = 10; //1=p1, 5=p5 (bootdata partition)
    lcd_is_enabled = 0;
    sprintf(buf, "mmcinit %d; ext2load mmc %d:%d 0x%08x %s 1", res, res, vax, &res, file);
    if (run_command(buf, 0)) { //no such file
        res = 0;
    } else {
        res = 1;
    }
    lcd_is_enabled = 1;
    return res;
}

int check_device_image(enum image_dev device, const char* file) {
    char res = ((device == DEV_SD) ? 0 : 1); //0=sdcard, 1=emmc
    char vax = ((device == DEV_SD) ? 1 : 5); //1=p1, 5=p5 (bootdata partition)
    lcd_is_enabled = 0;
    sprintf(buf, "mmcinit %d; fatload mmc %d:%d 0x%08x %s 1", res, res, vax, &res, file);
    if (run_command(buf, 0)) { //no such file
        res = 0;
    } else {
        res = 1;
    }
    lcd_is_enabled = 1;
    return res;
}

char read_u_boot_file(const char* file) {
	char res;
	lcd_is_enabled = 0;
	sprintf(buf, "mmcinit 0; fatload mmc 0:1 0x%08x %s 1", &res, file);
	if (run_command(buf, 0)) { //no such file
		res = 'X'; // this is going to mean no such file, or I guess the file could have 'X'...
	}
	lcd_is_enabled = 1;
	return res;
}

int write_u_boot_file(const char* file, char value) {
	lcd_is_enabled = 0;
	sprintf(buf, "mmcinit 0; fatsave mmc 0:1 0x%08x %s 1", &value, file);
	if (run_command(buf, 0)) {
		printf("Error: Cannot write /bootdata/%s.\n", file);
		value = 0;
	}
	lcd_is_enabled = 1;
	return value;
}

// -----------------------------------

void print_u_boot_dev(void) {
	if (read_u_boot_file("u-boot.device") == '1') {
		lcd_puts("Stock ROM     ");
	} else {
		lcd_puts("CM/custom ROM ");
	}
}

void print_u_boot_img(void) {
	if (read_u_boot_file("u-boot.altboot") == '1') {
		lcd_puts("Hybrid     ");
	} else {
		lcd_puts("Normal     ");
	}
}

void highlight_boot_line(int cursor, enum highlight_type highl) {
	switch (highl) {
	case HIGHLIGHT_GRAY:
		lcd_console_setcolor(CONSOLE_COLOR_GRAY, CONSOLE_COLOR_BLACK);
		break;
	case HIGHLIGHT_GREEN:
		lcd_console_setcolor(CONSOLE_COLOR_BLACK, CONSOLE_COLOR_GREEN);
		break;
	case HIGHLIGHT_CYAN:
		lcd_console_setcolor(CONSOLE_COLOR_BLACK, CONSOLE_COLOR_CYAN);
		break;
	case HIGHLIGHT_NONE:
	default:
		lcd_console_setcolor(CONSOLE_COLOR_CYAN, CONSOLE_COLOR_BLACK);
		break;
	}
	lcd_console_setpos(MENUTOP + cursor, INDENT);
	lcd_puts(opt_list[cursor]);
	if (cursor == CHANGE_BOOT_DEV) {
		print_u_boot_dev();
	}
	if (cursor == CHANGE_BOOT_IMG) {
		print_u_boot_img();
	}
}

int do_menu() {
	unsigned char key = 0;

	int valid_opt[NUM_OPTS] = { 0 };
	int x;
	int cursor = 0;
	u8 pwron = 0;

	lcd_console_setcolor(CONSOLE_COLOR_CYAN, CONSOLE_COLOR_BLACK);
	// lcd_clear (NULL, 1, 1, NULL);
	lcd_console_setpos(MENUTOP - 3, INDENT);
	lcd_puts("       Boot Menu");
	lcd_console_setpos(MENUTOP - 2, INDENT);
	lcd_puts("       ---------             ");

	gpio_write(COL0, 0);//drive COL0 LOW

	if (twl6030_hw_status(&pwron)) {
		lcd_console_setpos(MENUTOP, 1);
		lcd_puts("Error: Failed to read twl6030 hw_status\n");
	}

	// Booting from EMMC is always valid
	valid_opt[BOOT_EMMC_NORMAL] = 1;
	
	if (check_device_image(DEV_SD, "kernel") && check_device_image(DEV_SD, "ramdisk.cwm"))
		valid_opt[BOOT_SD_RECOVERY] = 1;
	if (check_device_image(DEV_SD, "kernel") && check_device_image(DEV_SD, "ramdisk"))
		valid_opt[BOOT_HYBRID] = 1;
	if (check_device_image(DEV_SD, "kernel") && check_device_image(DEV_SD, "ramdisk.stock"))
		valid_opt[BOOT_SD_ALTERNATE] = 1;

	if (read_u_boot_file("u-boot.device") != 'X') // if that file is there
		valid_opt[CHANGE_BOOT_DEV] = 1;
	if (read_u_boot_file("u-boot.altboot") != 'X') // if that file is there
		valid_opt[CHANGE_BOOT_IMG] = 1;

	for (x = 0; x < NUM_OPTS; x++) {
		lcd_console_setpos(MENUTOP + x, INDENT);
		if ((!valid_opt[CHANGE_BOOT_DEV] && !valid_opt[CHANGE_BOOT_IMG]) && (x
				== DEFAULT_BOOT_STR || x == CHANGE_BOOT_DEV || x
				== CHANGE_BOOT_IMG))
			continue;
		if (valid_opt[x])
			highlight_boot_line(x, HIGHLIGHT_NONE);
		else
			highlight_boot_line(x, HIGHLIGHT_GRAY);
	}

	lcd_console_setpos(MENUTOP + NUM_OPTS + 9, INDENT-9);
	lcd_puts("Press \"Volume down\" to move to the next item");
	lcd_console_setpos(MENUTOP + NUM_OPTS + 10, INDENT-9);
	lcd_puts("Press \"Volume up\" to to move to the previous item");
	lcd_console_setpos(MENUTOP + NUM_OPTS + 11, INDENT);
	lcd_puts("Press \"^\" to select");
	lcd_console_setpos(43, 0);
	lcd_puts1("Menu by j4mm3r, fattire, tonsofquestions, mik_os, Rebellos, HD, bokbokan, green.\n"
		 "    ** EXPERIMENTAL ** (" __DATE__ " " __TIME__ ")");

	cursor = BOOT_SD_RECOVERY;

	// highlight first option
	highlight_boot_line(cursor, HIGHLIGHT_CYAN);

	do {
		udelay(RESET_TICK);
	} while (get_keys_pressed(&key)); // wait for release

	do {
		get_keys_pressed(&key);

		lcd_console_setpos(5, 0);
#if 0 //debugging purpose
		lcd_printf(" keycode: 0x%X, 0x%X, 0x%X, 0x%X", key, readl(CM_L4PER_GPIO6_CLKCTRL),
				readl(CM_L4PER_GPIO2_CLKCTRL), readl(CM_L4PER_CLKSTCTRL));

#endif
		if (key & VOLDOWN_KEY) // button is pressed
		{
			// unhighlight current option
			highlight_boot_line(cursor, HIGHLIGHT_NONE);
			while (!valid_opt[++cursor] || cursor >= NUM_OPTS) {
				if (cursor >= NUM_OPTS)
					cursor = -1;
			}

			// highlight new option
			highlight_boot_line(cursor, HIGHLIGHT_CYAN);
			do {
				udelay(RESET_TICK);
			} while (get_keys_pressed(&key)); //wait for release

		}

		if (key & VOLUP_KEY) // button is pressed
		{
			// unhighlight current option
			highlight_boot_line(cursor, HIGHLIGHT_NONE);
			while (!valid_opt[--cursor] || cursor < 0) {
				if (cursor < 0)
					cursor = NUM_OPTS;

			}
			// highlight new option
			highlight_boot_line(cursor, HIGHLIGHT_CYAN);
			do {
				udelay(RESET_TICK);
			} while (get_keys_pressed(&key)); //wait for release

		}

		if ((key & N_KEY) && (cursor == CHANGE_BOOT_DEV)) { //selected modify device
			const char* file = "u-boot.device";
			if (read_u_boot_file(file) == '1') {
				write_u_boot_file(file, '0');
			} else {
				write_u_boot_file(file, '1');
			}
			udelay(RESET_TICK);
			highlight_boot_line(cursor, HIGHLIGHT_GREEN);
			do {
				udelay(RESET_TICK);
			} while (get_keys_pressed(&key)); //wait for release
		}

		if ((key & N_KEY) && (cursor == CHANGE_BOOT_IMG)) { //selected modify image
			const char* file = "u-boot.altboot";
			if (read_u_boot_file(file) == '1') {
				write_u_boot_file(file, '0');
			} else {
				write_u_boot_file(file, '1');
			}
			udelay(RESET_TICK);
			highlight_boot_line(cursor, HIGHLIGHT_GREEN);
			do {
				udelay(RESET_TICK);
			} while (get_keys_pressed(&key)); //wait for release
		}
		udelay(RESET_TICK);
	} while (!(key & N_KEY) || (cursor == CHANGE_BOOT_DEV) || (cursor
			== CHANGE_BOOT_IMG));

	highlight_boot_line(cursor, HIGHLIGHT_GREEN);

	lcd_console_setpos(MENUTOP + NUM_OPTS + 2, 25);
	lcd_console_setcolor(CONSOLE_COLOR_CYAN, CONSOLE_COLOR_BLACK);

	return cursor;
}
