
#include <common.h>
#include <omap4430sdp_lcd.h>
#include <video_font.h>
#include "console.h"

/************************* CONSOLE ********************************/


static uchar c_orient = O_PORTRAIT;
static uchar c_max_cols = VL_COL/(VIDEO_FONT_WIDTH*2);
static uchar c_max_rows = VL_ROW/(VIDEO_FONT_HEIGHT*2);
short console_col;
short console_row;
char lcd_is_enabled;
int lcd_color_fg;
int lcd_color_bg;

static uint   pixel_line_length = 900;

//static void lcd_drawchars (ushort x, ushort y, uchar *str, int count);
//static inline void lcd_puts_xy (ushort x, ushort y, uchar *s);
static inline void lcd_putc_xy (ushort x, ushort y, uchar  c);


/************************************************************************/

/*----------------------------------------------------------------------*/

static void console_scrollup (void)
{
	/* Copy up rows ignoring the first one */
	memcpy (CONSOLE_ROW_FIRST, CONSOLE_ROW_SECOND, CONSOLE_SCROLL_SIZE);

	/* Clear the last one */
	memset (CONSOLE_ROW_LAST, COLOR_MASK(lcd_color_bg), CONSOLE_ROW_SIZE);
}

/*----------------------------------------------------------------------*/

static inline void console_back (void)
{
	if (--console_col < 0) {
		console_col = c_max_cols-1 ;
		if (--console_row < 0) {
			console_row = 0;
		}
	}

	lcd_putc_xy (console_col * VIDEO_FONT_WIDTH,
		     console_row * VIDEO_FONT_HEIGHT,
		     ' ');
}

/*----------------------------------------------------------------------*/

static inline void console_newline (void)
{
	++console_row;
	console_col = 0;

	/* Check if we need to scroll the terminal */
	if (console_row >= c_max_rows) {
		/* Scroll everything up */
	//	console_scrollup () ;
	//	--console_row; 
	console_row = 0;
	}
}

/*----------------------------------------------------------------------*/

static inline void lcd_console_setpixel(ushort x, ushort y, ushort c)
{
	ushort rx = 899 - x;
	ushort ry = 1439 - y;
	//ushort rx = x;
	//ushort ry = y;
	ushort *dest = ((uint16_t *)ONSCREEN_BUFFER) + rx + (ry*pixel_line_length);
	*dest = c;
	//ushort *dest = ((uint8_t *)ONSCREEN_BUFFER) + rx * 3 + (ry*pixel_line_length*3);
	//*dest = c;
}

extern struct img_info bootimg_info;

static void lcd_drawchar(ushort x, ushort y, uchar c)
{
//    LOG_CONSOLE("lcd_drawchar: %c [%d, %d]\n", c, x, y);

    ushort row, col, rx, ry, sy, sx;
    for(row=0; row<VIDEO_FONT_HEIGHT; row++)
    {
        sy = y + row * 2;
        for(ry = sy; ry < (sy+2); ry++)
        {
            uchar bits = video_fontdata[c*VIDEO_FONT_HEIGHT+row];
            for(col=0; col<VIDEO_FONT_WIDTH; col++)
            {
                //sx = x + (col);
                //for(rx = sx; rx < (sx+1); rx++)
		rx = x + col * 2;
                {
                    lcd_console_setpixel(rx, ry,
                        (bits & 0x80)? lcd_color_fg:lcd_color_bg);
                    lcd_console_setpixel(rx+1, ry,
                        (bits & 0x80)? lcd_color_fg:lcd_color_bg);
                }
                bits <<= 1;
            }
        }
    }
}

static void lcd_drawchar1(ushort x, ushort y, uchar c)
{
//    LOG_CONSOLE("lcd_drawchar: %c [%d, %d]\n", c, x, y);

    ushort row, col, rx, ry, sy, sx;
    for(row=0; row<VIDEO_FONT_HEIGHT; row++)
    {
        sy = y + row ;
        for(ry = sy; ry < (sy+1); ry++)
        {
            uchar bits = video_fontdata[c*VIDEO_FONT_HEIGHT+row];
            for(col=0; col<VIDEO_FONT_WIDTH; col++)
            {
                //sx = x + (col);
                //for(rx = sx; rx < (sx+1); rx++)
		rx = x + col ;
                {
                    lcd_console_setpixel(rx, ry,
                        (bits & 0x80)? lcd_color_fg:lcd_color_bg);
                }
                bits <<= 1;
            }
        }
    }
}


void lcd_putc (const char c)
{
	switch (c) {
	case '\r':	console_col = 0;
			return;

	case '\n':	console_newline();
			return;

	case '\t':	/* Tab (8 chars alignment) */
			console_col +=  8;
			console_col &= ~7;

			if (console_col >= c_max_cols) {
				console_newline();
			}
			return;

	case '\b':	console_back();
			return;

	default:	lcd_drawchar(console_col*VIDEO_FONT_WIDTH*2,
				     console_row * VIDEO_FONT_HEIGHT*2,
				     c);
			if (++console_col >= c_max_cols) {
				console_newline();
			}
			return;
	}
	/* NOTREACHED */
}

void lcd_putc1 (const char c)
{
	switch (c) {
	case '\r':	console_col = 0;
			return;

	case '\n':	console_newline();
			return;

	case '\t':	/* Tab (8 chars alignment) */
			console_col +=  8;
			console_col &= ~7;

			if (console_col >= c_max_cols) {
				console_newline();
			}
			return;

	case '\b':	console_back();
			return;

	default:	lcd_drawchar1(console_col*VIDEO_FONT_WIDTH,
				     console_row * VIDEO_FONT_HEIGHT*2,
				     c);
			if (++console_col >= c_max_cols*2) {
				console_newline();
			}
			return;
	}
	/* NOTREACHED */
}

/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/

void lcd_puts1 (const char *s)
{
	while (*s) {
		lcd_putc1 (*s++);
	}
}

void lcd_puts (const char *s)
{
	while (*s) {
		lcd_putc (*s++);
	}
}


void lcd_printf(const char *fmt, ...)
{
        va_list args;
        char buf[CFG_PBSIZE];

        va_start(args, fmt);
        vsprintf(buf, fmt, args);
        va_end(args);

        lcd_puts(buf);

}


/*----------------------------------------------------------------------*/

void lcd_console_setpos(short row, short col)
{
	console_row = (row>0)? ((row > c_max_rows)? c_max_rows:row):0;
	console_col = (col>0)? ((col > c_max_cols)? c_max_cols:col):0;
}


/*----------------------------------------------------------------------*/

void lcd_console_setcolor(int fg, int bg)
{
  lcd_color_fg = fg;
  lcd_color_bg = bg;
}


/************************************************************************/
/* ** Low-Level Graphics Routines					*/
/************************************************************************/


static void lcd_drawchars (ushort x, ushort y, char *str, int count)
{
	ushort row;

	for (row=0;  row < VIDEO_FONT_HEIGHT;  ++row)  {
		int i, ry, sy = y + row * 2;
		for (ry = sy; ry < sy + 2; ry++) {
			uchar *s = str;
			for (i=0; i<count; ++i) {
				uchar c, bits;

				c = *s++;
				bits = video_fontdata[c * VIDEO_FONT_HEIGHT + row];

				for (c=0; c<8; ++c) {
					int rx = x + (i * 8 + c) * 2;
                    			lcd_console_setpixel(rx, ry,
                        			(bits & 0x80)? lcd_color_fg:lcd_color_bg);
                    			lcd_console_setpixel(rx + 1, ry,
                        			(bits & 0x80)? lcd_color_fg:lcd_color_bg);
					bits <<= 1;
				}
			}
		}
	}
}

void lcd_test() {
	int i;
	uint16_t *fb = ONSCREEN_BUFFER;
	for (i = 0; i < 900; i++)
		lcd_console_setpixel(i, 100, 0xFFFF);
	lcd_console_setcolor(0xFFFF, 0xFF);
	lcd_drawchar(100,100, 'A');
	lcd_drawchars(100,200, "I love nook HD", 14);
	lcd_console_setpos(5,5);
	lcd_puts("heeel\n");
	lcd_puts("hel\n");
}


/*----------------------------------------------------------------------*/

static inline void lcd_puts_xy (ushort x, ushort y, uchar *s)
{
#if defined(CONFIG_LCD_LOGO) && !defined(CONFIG_LCD_INFO_BELOW_LOGO) \
  && !defined(CONFIG_ACCLAIM)
	lcd_drawchars (x, y+BMP_LOGO_HEIGHT_B, s, strlen ((char *)s));
#else
	lcd_drawchars (x, y, s, strlen ((char *)s));
#endif
}

/*----------------------------------------------------------------------*/


static inline void lcd_putc_xy (ushort x, ushort y, uchar c)
{ 
}

void lcd_console_init()
{


        c_max_cols = VL_COL/(VIDEO_FONT_WIDTH*2);
        c_max_rows = VL_ROW/(VIDEO_FONT_HEIGHT*2);

//	lcd_clear_screen();

	console_col = 0;

	console_row = 1;	/* leave 1 blank line below logo */

	lcd_console_setpos(0, 0);
	return 0;
}

