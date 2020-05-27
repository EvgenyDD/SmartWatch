/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef SSD1306_H
#define SSD1306_H

/* Includes ------------------------------------------------------------------*/
#include "nrf.h"
#include "spi_master.h"


/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define PORTRAIT 0
#define LANDSCAPE 1

#define BLACK 0
#define WHITE 1
#define INVERSE 2

#define SSD1306_LCDWIDTH                  128
#define SSD1306_LCDHEIGHT                 64

#define WIDTH	SSD1306_LCDWIDTH
#define HEIGHT	SSD1306_LCDHEIGHT

#define SSD1306_SETCONTRAST 0x81
#define SSD1306_DISPLAYALLON_RESUME 0xA4
#define SSD1306_DISPLAYALLON 0xA5
#define SSD1306_NORMALDISPLAY 0xA6
#define SSD1306_INVERTDISPLAY 0xA7
#define SSD1306_DISPLAYOFF 0xAE
#define SSD1306_DISPLAYON 0xAF

#define SSD1306_SETDISPLAYOFFSET 0xD3
#define SSD1306_SETCOMPINS 0xDA

#define SSD1306_SETVCOMDETECT 0xDB

#define SSD1306_SETDISPLAYCLOCKDIV 0xD5
#define SSD1306_SETPRECHARGE 0xD9

#define SSD1306_SETMULTIPLEX 0xA8

#define SSD1306_SETLOWCOLUMN 0x00
#define SSD1306_SETHIGHCOLUMN 0x10

#define SSD1306_SETSTARTLINE 0x40

#define SSD1306_MEMORYMODE 0x20
#define SSD1306_COLUMNADDR 0x21
#define SSD1306_PAGEADDR   0x22

#define SSD1306_COMSCANINC 0xC0
#define SSD1306_COMSCANDEC 0xC8

#define SSD1306_SEGREMAP 0xA0

#define SSD1306_CHARGEPUMP 0x8D

#define SSD1306_EXTERNALVCC 0x1
#define SSD1306_SWITCHCAPVCC 0x2

// Scrolling #defines
#define SSD1306_ACTIVATE_SCROLL 0x2F
#define SSD1306_DEACTIVATE_SCROLL 0x2E
#define SSD1306_SET_VERTICAL_SCROLL_AREA 0xA3
#define SSD1306_RIGHT_HORIZONTAL_SCROLL 0x26
#define SSD1306_LEFT_HORIZONTAL_SCROLL 0x27
#define SSD1306_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL 0x29
#define SSD1306_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL 0x2A


/* Exported macro ------------------------------------------------------------*/
#define BitSet(p,m) ((p) |= (1<<(m)))
#define BitReset(p,m) ((p) &= ~(1<<(m)))
#define BitFlip(p,m) ((p) ^= (1<<(m)))
#define BitWrite(c,p,m) ((c) ? BitSet(p,m) : BitReset(p,m))
#define BitIsSet(reg, bit) (((reg) & (1<<(bit))) != 0)
#define BitIsReset(reg, bit) (((reg) & (1<<(bit))) == 0)


/* Exported define -----------------------------------------------------------*/
extern unsigned char font3x5[];
extern unsigned char font5x8[];


/* Exported functions ------------------------------------------------------- */
void SSD1306_Init();

void SSD1306_Off();
void SSD1306_On();

void SSD1306_Command(uint8_t c);
void SSD1306_Data(uint8_t c);
uint8_t SPI_Send(uint8_t data);

void SSD1306_ReDraw();			//write display buffer to display
void SSD1306_ReDrawCLEAR();
void SSD1306_Invert(uint8_t i);	//set display option: !0 - Invert mode, 0 - Normal mode
void SSD1306_Brightness(uint8_t contrast);	//set display brightness: 0-255

void SSD1306_Clear(); 	//Clear display buffer
void SSD1306_Fill();	//Fill display buffer

void SSD1306_Pixel(int16_t x, int16_t y, uint16_t color);
void SSD1306_setFont(uint8_t* font);

//void SPI_Config(spi_master_instance_t *SPI, spi_master_config_t SPIInit);

void startscrollright(uint8_t start, uint8_t stop);
void startscrollleft(uint8_t start, uint8_t stop);

void startscrolldiagright(uint8_t start, uint8_t stop);
void startscrolldiagleft(uint8_t start, uint8_t stop);
void stopscroll();

void SSD1306_Rect(int x1, int y1, int x2, int y2);
	  void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
	  void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
void SSD1306_RectFill(int x1, int y1, int x2, int y2);
	  void drawFastVLineInternal(int16_t x, int16_t y, int16_t h, uint16_t color);
	  void drawFastHLineInternal(int16_t x, int16_t y, int16_t w, uint16_t color);

    void SSD1306_HLine(uint8_t y, uint8_t color);

void SSD1306_Char(uint8_t x, uint8_t y, char s);
void SSD1306_BigChar(uint8_t x, uint8_t y, char s, uint8_t scale);
void SSD1306_String(int x, int y, char *s);
void SSD1306_BigString(int x, int y, char *s, uint8_t scale);

void SSD1306_Num(int x, int y, int32_t num);
void SSD1306_NumWDesc(int x, int y, char *s, int32_t num);
void SSD1306_Float(int x, int y, float num, int precision);
void SSD1306_FloatWDesc(int x, int y, char *s, float num, int precision);

void SSD1306_Bitmap(int x, int y, char mass[]);
void SSD1306_InvertRect(int x, int y, int width, int height);
void SSD1306_EndSplash();

#endif //SSD1306_H
