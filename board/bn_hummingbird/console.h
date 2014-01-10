
#define ONSCREEN_BUFFER 0xb2600000

#define LCD_MONOCHROME  0
#define LCD_COLOR2      1
#define LCD_COLOR4      2
#define LCD_COLOR8      3
#define LCD_COLOR16     4

#define LCD_BPP LCD_COLOR16

# define LCD_INFO_X             (VIDEO_FONT_WIDTH)
# define LCD_INFO_Y             (VIDEO_FONT_HEIGHT)


/* Default to 8bpp if bit depth not specified */
#ifndef LCD_BPP
# define LCD_BPP                        LCD_COLOR8
#endif
#ifndef LCD_DF
# define LCD_DF                 1
#endif

/* Calculate nr. of bits per pixel  and nr. of colors */
#define NBITS(bit_code)         (1 << (bit_code))
#define NCOLORS(bit_code)       (1 << NBITS(bit_code))


#define VL_COL  900
#define VL_ROW 1440


/************************************************************************/
/* ** CONSOLE CONSTANTS                                                 */
/************************************************************************/

/*
 *  * 16bpp color definitions
 *   */
# define CONSOLE_COLOR_BLACK    0x0000
# define CONSOLE_COLOR_BLUE     0x001F
# define CONSOLE_COLOR_GREEN    0x07E0
# define CONSOLE_COLOR_RED      0xF800
# define CONSOLE_COLOR_ORANGE   0xC300  /* for clockworkish goodness */
# define CONSOLE_COLOR_GRAY     0x0432
# define CONSOLE_COLOR_YELLOW   (CONSOLE_COLOR_RED | CONSOLE_COLOR_GREEN)
# define CONSOLE_COLOR_MAGENTA  (CONSOLE_COLOR_RED | CONSOLE_COLOR_BLUE)
# define CONSOLE_COLOR_CYAN     (CONSOLE_COLOR_BLUE | CONSOLE_COLOR_GREEN)
# define CONSOLE_COLOR_WHITE    0xffff  /* Must remain last / highest   */

#define O_LANDSCAPE   0
#define O_PORTRAIT    1

extern char lcd_is_enabled;

extern int lcd_line_length;
extern int lcd_color_fg;
extern int lcd_color_bg;

/************************************************************************/
/* ** CONSOLE DEFINITIONS & FUNCTIONS                                   */

/************************************************************************/

# define CONSOLE_ROWS           (VL_ROW / VIDEO_FONT_HEIGHT)


#define CONSOLE_COLS            (VL_COL / VIDEO_FONT_WIDTH)
#define CONSOLE_ROW_SIZE        (VIDEO_FONT_HEIGHT * lcd_line_length)
#define CONSOLE_SIZE            (CONSOLE_ROW_SIZE * CONSOLE_ROWS)
#define CONSOLE_ROW_FIRST       (ONSCREEN_BUFFER)
#define CONSOLE_ROW_SECOND      (ONSCREEN_BUFFER + CONSOLE_ROW_SIZE)
#define CONSOLE_ROW_LAST        (ONSCREEN_BUFFER + CONSOLE_SIZE \
                                        - CONSOLE_ROW_SIZE)
#define CONSOLE_SIZE            (CONSOLE_ROW_SIZE * CONSOLE_ROWS)
#define CONSOLE_SCROLL_SIZE     (CONSOLE_SIZE - CONSOLE_ROW_SIZE)

#if LCD_BPP == LCD_MONOCHROME
# define COLOR_MASK(c)          ((c)      | (c) << 1 | (c) << 2 | (c) << 3 | \
                                 (c) << 4 | (c) << 5 | (c) << 6 | (c) << 7)
#elif (LCD_BPP == LCD_COLOR8) || (LCD_BPP == LCD_COLOR16)
# define COLOR_MASK(c)          (c)
#else
# error Unsupported LCD BPP.
#endif

/************************************************************************/

