/* Includes ------------------------------------------------------------------*/
#include "SSD1306.h"

#include "nrf.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"

//#include "debug.h"

#include "spi_master.h"
#include "app_util_platform.h"
#include <string.h>
#include "string.h"

enum state
{
	DISABLE = 0, ENABLE = !DISABLE
};

/* Private typedef -----------------------------------------------------------*/
struct _current_font
{
	uint8_t* font;
	uint8_t x_size;
	uint8_t y_size;
	uint8_t offset;
	uint8_t numchars;
	uint8_t inverse;
} cfont;

/* Private define ------------------------------------------------------------*/
//CS - 11
#define D_CS    11
//RESET - 12
#define D_RESET 12
//D/C - 13
#define D_DC    13
//CLK - 14
#define SPI_CLK 14
//Data - 15
#define SPI_MOSI 15

/* Private macro -------------------------------------------------------------*/
#define swap(i, j) {uint16_t t = i; i = j; j = t;}

#define RESET_H	nrf_gpio_pin_set(D_RESET);
#define RESET_L nrf_gpio_pin_clear(D_RESET);

#define CS_H nrf_gpio_pin_set(D_CS);
#define CS_L nrf_gpio_pin_clear(D_CS);

#define DC_H nrf_gpio_pin_set(D_DC);
#define DC_L nrf_gpio_pin_clear(D_DC);

/* Private variables ---------------------------------------------------------*/
uint8_t rotation = 1;
uint8_t orient;
uint8_t _transparent = 0;

// the memory buffer of the LCD
static uint8_t buffer[SSD1306_LCDHEIGHT * SSD1306_LCDWIDTH / 8] =
{
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

/* Extern variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
	
/*******************************************************************************
 * Function Name  : SPI_Send
 * Description    : Send data through SPI
 *******************************************************************************/
static uint8_t __INLINE SPI_Send(uint8_t data)
{
	NRF_SPI0->TXD = data;

	while(!NRF_SPI0->EVENTS_READY)
		;
	NRF_SPI0->EVENTS_READY = 0;

	return NRF_SPI0->RXD;
}

/*******************************************************************************
 * Function Name  : SSD1306_Init
 * Description    : Initialize display
 *******************************************************************************/
void SSD1306_Init()
{
	nrf_gpio_cfg_output(D_CS);
	nrf_gpio_cfg_output(D_DC);
	nrf_gpio_cfg_output(D_RESET);

	nrf_gpio_cfg_output(SPI_CLK);
	nrf_gpio_cfg_output(SPI_MOSI);
	NRF_SPI0->PSELSCK = SPI_CLK;
	NRF_SPI0->PSELMOSI = SPI_MOSI;
	NRF_SPI0->FREQUENCY = (uint32_t) 0x80000000; //0x02000000
	NRF_SPI0->CONFIG = (SPI_CONFIG_CPHA_Leading << SPI_CONFIG_CPHA_Pos) | (SPI_CONFIG_CPOL_ActiveHigh << SPI_CONFIG_CPOL_Pos)
		| (SPI_CONFIG_ORDER_MsbFirst << SPI_CONFIG_ORDER_Pos);
	NRF_SPI0->EVENTS_READY = 0U;
	NRF_SPI0->ENABLE = (SPI_ENABLE_ENABLE_Enabled << SPI_ENABLE_ENABLE_Pos);

	RESET_H;
	nrf_delay_us(10000);
	RESET_L;
	nrf_delay_us(50000);
	RESET_H;

	// Init sequence for 128x64 OLED module
	SSD1306_Command (SSD1306_DISPLAYOFF); // 0xAE
	SSD1306_Command (SSD1306_SETDISPLAYCLOCKDIV); // 0xD5
	SSD1306_Command(0x80); // the suggested ratio 0x80
	SSD1306_Command (SSD1306_SETMULTIPLEX); // 0xA8
	SSD1306_Command(0x3F);
	SSD1306_Command (SSD1306_SETDISPLAYOFFSET); // 0xD3
	SSD1306_Command(0x0); // no offset
	SSD1306_Command(SSD1306_SETSTARTLINE | 0x0); // line #0
	SSD1306_Command (SSD1306_CHARGEPUMP); // 0x8D
#if 0
	SSD1306_Command(0x10); // SSD1306_EXTERNALVCC
#else
	SSD1306_Command(0x14);
#endif

	SSD1306_Command (SSD1306_MEMORYMODE); // 0x20
	SSD1306_Command(0x00); // 0x0 act like ks0108
	SSD1306_Command(SSD1306_SEGREMAP | 0x1);
	SSD1306_Command (SSD1306_COMSCANDEC);
	SSD1306_Command (SSD1306_SETCOMPINS); // 0xDA
	SSD1306_Command(0x12);
	SSD1306_Command (SSD1306_SETCONTRAST); // 0x81

#if 0
	SSD1306_Command(0x9F); // SSD1306_EXTERNALVCC)
#else
	SSD1306_Command(0xCF);
#endif

	SSD1306_Command (SSD1306_SETPRECHARGE); // 0xd9

#if 0
	SSD1306_Command(0x22); // SSD1306_EXTERNALVCC)
#else
	SSD1306_Command(0xF1);
#endif

	SSD1306_Command (SSD1306_SETVCOMDETECT); // 0xDB
	SSD1306_Command(0x40);
	SSD1306_Command (SSD1306_DISPLAYALLON_RESUME); // 0xA4
	SSD1306_Command (SSD1306_NORMALDISPLAY); // 0xA6

	SSD1306_Command (SSD1306_DISPLAYON); //--turn on oled panel

	nrf_delay_us(100000);
	//SSD1306_ReDraw();

	//orient = LANDSCAPE;
	orient = PORTRAIT;
}

/*******************************************************************************
 * Function Name  : SSD1306_Command
 * Description    : Send command to display
 *******************************************************************************/
void SSD1306_Command(uint8_t command)
{
	DC_L;
	CS_L;
	SPI_Send(command);
	CS_H;
}

/*******************************************************************************
 * Function Name  : SSD1306_Data
 * Description    : Send data to display
 *******************************************************************************/
void SSD1306_Data(uint8_t data)
{
	DC_H;
	CS_L;
	SPI_Send(data);
	CS_H;
}


/*******************************************************************************
 * Function Name  :
 * Description    :
 *******************************************************************************/
// Activate a right handed scroll for rows start through stop
// Hint, the SSD1306_ReDraw is 16 rows tall. To scroll the whole SSD1306_ReDraw, run:
// SSD1306_ReDraw.scrollright(0x00, 0x0F)
void startscrollright(uint8_t start, uint8_t stop)
{
	SSD1306_Command (SSD1306_RIGHT_HORIZONTAL_SCROLL);
	SSD1306_Command(0X00);
	SSD1306_Command(start);
	SSD1306_Command(0X00);
	SSD1306_Command(stop);
	SSD1306_Command(0X00);
	SSD1306_Command(0XFF);
	SSD1306_Command (SSD1306_ACTIVATE_SCROLL);
}

/*******************************************************************************
 * Function Name  :
 * Description    :
 *******************************************************************************/
// Activate a right handed scroll for rows start through stop
// Hint, the SSD1306_ReDraw is 16 rows tall. To scroll the whole SSD1306_ReDraw, run:
// SSD1306_ReDraw.scrollright(0x00, 0x0F)
void startscrollleft(uint8_t start, uint8_t stop)
{
	SSD1306_Command (SSD1306_LEFT_HORIZONTAL_SCROLL);
	SSD1306_Command(0X00);
	SSD1306_Command(start);
	SSD1306_Command(0X00);
	SSD1306_Command(stop);
	SSD1306_Command(0X00);
	SSD1306_Command(0XFF);
	SSD1306_Command (SSD1306_ACTIVATE_SCROLL);
}

/*******************************************************************************
 * Function Name  :
 * Description    :
 *******************************************************************************/
// Activate a diagonal scroll for rows start through stop
// Hint, the SSD1306_ReDraw is 16 rows tall. To scroll the whole SSD1306_ReDraw, run:
// SSD1306_ReDraw.scrollright(0x00, 0x0F)
void startscrolldiagright(uint8_t start, uint8_t stop)
{
	SSD1306_Command (SSD1306_SET_VERTICAL_SCROLL_AREA);
	SSD1306_Command(0X00);
	SSD1306_Command (SSD1306_LCDHEIGHT);
	SSD1306_Command (SSD1306_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL);
	SSD1306_Command(0X00);
	SSD1306_Command(start);
	SSD1306_Command(0X00);
	SSD1306_Command(stop);
	SSD1306_Command(0X01);
	SSD1306_Command (SSD1306_ACTIVATE_SCROLL);
}

/*******************************************************************************
 * Function Name  :
 * Description    :
 *******************************************************************************/
// Activate a diagonal scroll for rows start through stop
// Hint, the SSD1306_ReDraw is 16 rows tall. To scroll the whole SSD1306_ReDraw, run:
// SSD1306_ReDraw.scrollright(0x00, 0x0F)
void startscrolldiagleft(uint8_t start, uint8_t stop)
{
	SSD1306_Command (SSD1306_SET_VERTICAL_SCROLL_AREA);
	SSD1306_Command(0X00);
	SSD1306_Command (SSD1306_LCDHEIGHT);
	SSD1306_Command (SSD1306_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL);
	SSD1306_Command(0X00);
	SSD1306_Command(start);
	SSD1306_Command(0X00);
	SSD1306_Command(stop);
	SSD1306_Command(0X01);
	SSD1306_Command (SSD1306_ACTIVATE_SCROLL);
}

/*******************************************************************************
 * Function Name  :
 * Description    :
 *******************************************************************************/
void stopscroll()
{
	SSD1306_Command (SSD1306_DEACTIVATE_SCROLL);
}

/*******************************************************************************
 * Function Name  :
 * Description    :
 *******************************************************************************/
void SSD1306_Off()
{
	SSD1306_Command (SSD1306_DISPLAYOFF);
	//NRF_SPI0->ENABLE = (SPI_ENABLE_ENABLE_Disabled << SPI_ENABLE_ENABLE_Pos);
}

/*******************************************************************************
 * Function Name  : SSD1306_On
 * Description    : Turn display on
 *******************************************************************************/
void SSD1306_On()
{
	//NRF_SPI0->ENABLE = (SPI_ENABLE_ENABLE_Enabled << SPI_ENABLE_ENABLE_Pos);
	SSD1306_Command (SSD1306_DISPLAYON);
}

/*******************************************************************************
 * Function Name  : SSD1306_ReDraw
 * Description    : Send display buffer to display
 *******************************************************************************/
void SSD1306_ReDraw()
{
	SSD1306_Command (SSD1306_COLUMNADDR);
	SSD1306_Command(0); // Column start address (0 = reset)
	SSD1306_Command(SSD1306_LCDWIDTH - 1); // Column end address (127 = reset)

	SSD1306_Command (SSD1306_PAGEADDR);
	SSD1306_Command(0); // Page start address (0 = reset)

	SSD1306_Command(7); // Page end address
	//SSD1306_LCDHEIGHT == 32 SSD1306_Command(3); // Page end address
	//SSD1306_LCDHEIGHT == 16 SSD1306_Command(1); // Page end address

	DC_H;
	CS_L;

	for(uint16_t i = 0; i < (SSD1306_LCDWIDTH * SSD1306_LCDHEIGHT / 8); i++)
		SPI_Send(buffer[i]);

	CS_H;
}

/*******************************************************************************
 * Function Name  : SSD1306_ReDrawCLEAR
 * Description    : Clear the display (but not display buffer)
 *******************************************************************************/
void SSD1306_ClearScreen()
{
	SSD1306_Command (SSD1306_COLUMNADDR);
	SSD1306_Command(0); // Column start address (0 = reset)
	SSD1306_Command(SSD1306_LCDWIDTH - 1); // Column end address (127 = reset)

	SSD1306_Command (SSD1306_PAGEADDR);
	SSD1306_Command(0); // Page start address (0 = reset)

	SSD1306_Command(7); // Page end address
	//SSD1306_LCDHEIGHT == 32 SSD1306_Command(3); // Page end address
	//SSD1306_LCDHEIGHT == 16 SSD1306_Command(1); // Page end address

	DC_H;
	CS_L;

	for(uint16_t i = 0; i < (SSD1306_LCDWIDTH * SSD1306_LCDHEIGHT / 8); i++)
		SPI_Send(0);

	CS_H;
}

/*******************************************************************************
 * Function Name  : SSD1306_Brightness
 * Description    : Set display brightness (contrast)
 * Input          : contrast
 *******************************************************************************/
void SSD1306_Brightness(uint8_t contrast)
{
	// the range of contrast to too small to be really useful
	SSD1306_Command (SSD1306_SETCONTRAST);
	SSD1306_Command(contrast);
}

/*******************************************************************************
 * Function Name  : SSD1306_Invert
 * Description    : Invert display picture (but not display buffer)
 *******************************************************************************/
void SSD1306_InvertScreen(uint8_t state)
{
	state ? SSD1306_Command(SSD1306_INVERTDISPLAY) : SSD1306_Command(SSD1306_NORMALDISPLAY);
}

/******************/
/* DISPLAY BUFFER */
/******************/

/*******************************************************************************
 * Function Name  : SSD1306_Clear
 * Description    : Clear display buffer
 *******************************************************************************/
// clear everything
void SSD1306_Clear()
{
	ClearBuffer(buffer, SSD1306_LCDHEIGHT * SSD1306_LCDWIDTH / 8, 0);
}

/*******************************************************************************
 * Function Name  : SSD1306_Fill
 * Description    : Fill display buffer
 *******************************************************************************/
void SSD1306_Fill()
{
	for(uint32_t i = 0; i < SSD1306_LCDHEIGHT * SSD1306_LCDWIDTH / 8; i++)
		buffer[i] = 0xFF;
}

/*******************************************************************************
 * Function Name  : SSD1306_Pixel
 * Description    : Set pixel in display buffer
 * Input		     : position (x, y) and color (WHITE, BLACK, INVERSE)
 *******************************************************************************/
void SSD1306_Pixel(int16_t x, int16_t y, uint16_t color)
{
	if(orient == PORTRAIT)
	{
		swap(x, y);
		if((x < 0) || (x >= SSD1306_LCDWIDTH) || (y < 0) || (y >= SSD1306_LCDHEIGHT))
			return;
	}
	if(orient == LANDSCAPE)
	{
		if((x < 0) || (x >= SSD1306_LCDWIDTH) || (y < 0) || (y >= SSD1306_LCDHEIGHT))
			return;
	}

	// check rotation, move pixel around if necessary
	switch (orient)
	{
	case 1:
		//swap(x, y);
		x = WIDTH - x - 1;
		break;

	case 2:
		x = WIDTH - x - 1;
		y = HEIGHT - y - 1;
		break;

	case 3:
		swap(x, y);
		y = HEIGHT - y - 1;
		break;
	}

	// x is - column
	switch (color)
	{
	case WHITE:
		BitSet(buffer[x + (y / 8) * SSD1306_LCDWIDTH], y & 7);
		break;

	case BLACK:
		BitReset(buffer[x + (y / 8) * SSD1306_LCDWIDTH], y & 7);
		break;

	case INVERSE:
		BitFlip(buffer[x + (y / 8) * SSD1306_LCDWIDTH], y & 7);
		break;
	}

}

/*******************************************************************************
 * Function Name  :
 * Description    :
 * Input		 :
 *******************************************************************************/
void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color)
{
	uint8_t bSwap = DISABLE;
	switch (rotation)
	{
	case 0:
		// 0 degree rotation, do nothing
		break;
	case 1:
		// 90 degree rotation, swap x & y for rotation, then invert x
		bSwap = ENABLE;
		swap(x, y);
		//x = WIDTH - x - 1;
		break;
	case 2:
		// 180 degree rotation, invert x and y - then shift y around for height.
		x = WIDTH - x - 1;
		y = HEIGHT - y - 1;
		x -= (w - 1);
		break;
	case 3:
		// 270 degree rotation, swap x & y for rotation, then invert y  and adjust y for w (not to become h)
		bSwap = ENABLE;
		swap(x, y);
		y = HEIGHT - y - 1;
		y -= (w - 1);
		break;
	}

	if(bSwap)
	{
		drawFastVLineInternal(x, y, w, color);
	}
	else
	{
		drawFastHLineInternal(x, y, w, color);
	}
}

/*******************************************************************************
 * Function Name  :
 * Description    :
 * Input		 :
 *******************************************************************************/
void drawFastHLineInternal(int16_t x, int16_t y, int16_t w, uint16_t color)
{
	// Do bounds/limit checks
	if(y < 0 || y >= HEIGHT)
	{
		return;
	}

	// make sure we don't try to draw below 0
	if(x < 0)
	{
		w += x;
		x = 0;
	}

	// make sure we don't go off the edge of the SSD1306_ReDraw
	if((x + w) > WIDTH)
	{
		w = (WIDTH - x);
	}

	int8_t zero = 0;
	// if our width is now negative, punt
	if(w <= 0)
	{
		zero = 1;/*return;*/
	}

	// set up the pointer for  movement through the buffer
	register uint8_t *pBuf = buffer;
	// adjust the buffer pointer for the current row
	pBuf += ((y / 8) * SSD1306_LCDWIDTH);
	// and offset x columns in
	pBuf += x;

	register uint8_t mask = 1 << (y & 7);

	if(!zero)
	{

		switch (color)
		{
		case WHITE:
			while(w--)
			{
				*pBuf++ |= mask;
			}
			;
			break;
		case BLACK:
			mask = ~mask;
			while(w--)
			{
				*pBuf++ &= mask;
			}
			;
			break;
		case INVERSE:
			while(w--)
			{
				*pBuf++ ^= mask;
			}
			;
			break;
		}
	}
	else
	{
		switch (color)
		{
		case WHITE:
			while(w++)
			{
				*pBuf-- |= mask;
			}
			;
			break;
		case BLACK:
			mask = ~mask;
			while(w++)
			{
				*pBuf-- &= mask;
			}
			;
			break;
		case INVERSE:
			while(w++)
			{
				*pBuf-- ^= mask;
			}
			;
			break;
		}
	}
}

/*******************************************************************************
 * Function Name  :
 * Description    :
 *******************************************************************************/
void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color)
{
	uint8_t bSwap = DISABLE;
	switch (rotation)
	{
	case 0:
		break;
	case 1:
		// 90 degree rotation, swap x & y for rotation, then invert x and adjust x for h (now to become w)
		bSwap = ENABLE;
		swap(x, y);
		x = WIDTH - x - 1;
		x -= (h - 1);
		break;
	case 2:
		// 180 degree rotation, invert x and y - then shift y around for height.
		x = WIDTH - x - 1;
		y = HEIGHT - y - 1;
		y -= (h - 1);
		break;
	case 3:
		// 270 degree rotation, swap x & y for rotation, then invert y
		bSwap = ENABLE;
		swap(x, y);
		y = HEIGHT - y - 1;
		break;
	}

	if(bSwap)
	{
		drawFastHLineInternal(x, y, h, color);
	}
	else
	{
		drawFastVLineInternal(x, y, h, color);
	}
}

void SSD1306_VLine(uint8_t x, uint8_t y, uint8_t height, uint8_t color)
{
	for(uint8_t i=0; i<height; i++)
	{
		SSD1306_Pixel(x,y+i, color);
	}
}

void SSD1306_HLine(uint8_t x, uint8_t y, uint8_t width, uint8_t color)
{
	for(uint8_t i=0; i<width; i++)
	{
		SSD1306_Pixel(x+i,y, color);
	}
}

/*******************************************************************************
 * Function Name  :
 * Description    :
 *******************************************************************************/
void drawFastVLineInternal(int16_t x, int16_t __y, int16_t __h, uint16_t color)
{
	// do nothing if we're off the left or right side of the screen
	if(x < 0 || x >= WIDTH)
	{
		return;
	}

	// make sure we don't try to draw below 0
	if(__y < 0)
	{
		// __y is negative, this will subtract enough from __h to account for __y being 0
		__h += __y;
		__y = 0;

	}

	// make sure we don't go past the height of the SSD1306_ReDraw
	if((__y + __h) > HEIGHT)
	{
		__h = (HEIGHT - __y);
	}

	// if our height is now negative, punt
	if(__h <= 0)
	{
		return;
	}

	// this display doesn't need ints for coordinates, use local byte registers for faster juggling
	register uint8_t y = __y;
	register uint8_t h = __h;

	// set up the pointer for fast movement through the buffer
	register uint8_t *pBuf = buffer;
	// adjust the buffer pointer for the current row
	pBuf += ((y / 8) * SSD1306_LCDWIDTH);
	// and offset x columns in
	pBuf += x;

	// do the first partial byte, if necessary - this requires some masking
	register uint8_t mod = (y & 7);
	if(mod)
	{
		// mask off the high n bits we want to set
		mod = 8 - mod;

		// note - lookup table results in a nearly 10% performance improvement in fill* functions
		// register uint8_t mask = ~(0xFF >> (mod));
		static uint8_t premask[8] =
		{ 0x00, 0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE };
		register uint8_t mask = premask[mod];

		// adjust the mask if we're not going to reach the end of this byte
		if(h < mod)
		{
			mask &= (0XFF >> (mod - h));
		}

		switch (color)
		{
		case WHITE:
			*pBuf |= mask;
			break;
		case BLACK:
			*pBuf &= ~mask;
			break;
		case INVERSE:
			*pBuf ^= mask;
			break;
		}

		// fast exit if we're done here!
		if(h < mod)
		{
			return;
		}

		h -= mod;

		pBuf += SSD1306_LCDWIDTH;
	}

	// write solid bytes while we can - effectively doing 8 rows at a time
	if(h >= 8)
	{
		if(color == INVERSE)
		{ // separate copy of the code so we don't impact performance of the black/white write version with an extra comparison per loop
			do
			{
				*pBuf = ~(*pBuf);

				// adjust the buffer forward 8 rows worth of data
				pBuf += SSD1306_LCDWIDTH;

				// adjust h & y (there's got to be a faster way for me to do this, but this should still help a fair bit for now)
				h -= 8;
			} while(h >= 8);
		}
		else
		{
			// store a local value to work with
			register uint8_t val = (color == WHITE) ? 255 : 0;

			do
			{
				// write our value in
				*pBuf = val;

				// adjust the buffer forward 8 rows worth of data
				pBuf += SSD1306_LCDWIDTH;

				// adjust h & y (there's got to be a faster way for me to do this, but this should still help a fair bit for now)
				h -= 8;
			} while(h >= 8);
		}
	}

	// now do the final partial byte, if necessary
	if(h)
	{
		mod = h & 7;
		// this time we want to mask the low bits of the byte, vs the high bits we did above
		// register uint8_t mask = (1 << mod) - 1;
		// note - lookup table results in a nearly 10% performance improvement in fill* functions
		static uint8_t postmask[8] =
		{ 0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F };
		register uint8_t mask = postmask[mod];
		switch (color)
		{
		case WHITE:
			*pBuf |= mask;
			break;
		case BLACK:
			*pBuf &= ~mask;
			break;
		case INVERSE:
			*pBuf ^= mask;
			break;
		}
	}
}

/*******************************************************************************
 * Function Name  :
 * Description    :
 *******************************************************************************/
void SSD1306_Rect(int x1, int y1, int x2, int y2)
{
	if(x1 > x2)
		swap(x1, x2);	

	if(y1 > y2)
		swap(y1, y2);

	drawFastHLine(x1, y1, x2 - x1+1, 1);
	drawFastHLine(x1, y2, x2 - x1 + 1, 1);

	drawFastVLine(x1, 128-y1, y1-y2, 1);
	drawFastVLine(x2, 128-y1, y1-y2, 1);
}

/*******************************************************************************
 * Function Name  :
 * Description    :
 *******************************************************************************/
void SSD1306_RectFill(int x1, int y1, int x2, int y2, int8_t color)
{
	for(uint16_t i = x1; i < x2; i++)
		for(uint16_t k = y1; k < y2; k++)
			SSD1306_Pixel(k, i, color);

}

/*******************************************************************************
 * Function Name  :
 * Description    :
 *******************************************************************************/
void SSD1306_setFont(uint8_t* font)
{
	cfont.font = font;
	cfont.x_size = cfont.font[0];
	cfont.y_size = cfont.font[1];
	cfont.offset = cfont.font[2];
	cfont.numchars = cfont.font[3];
	cfont.inverse = cfont.font[4];
}

/*******************************************************************************
 * Function Name  :
 * Description    :
 *******************************************************************************/
void SSD1306_Char(uint8_t x, uint8_t y, char s)
{
	SSD1306_MultChar(x, y, s, 1);
}

/*******************************************************************************
 * Function Name  :
 * Description    :
 *******************************************************************************/
void SSD1306_MultChar(uint8_t x, uint8_t y, char s, uint8_t scale)
{
#define FONT_WIDTH cfont.x_size
#define FONT_HEIGHT cfont.y_size

	uint32_t temp = ((s - cfont.offset) * FONT_WIDTH * (FONT_HEIGHT < 8 ? 8 : FONT_HEIGHT) / 8) + 5;

	if(BitIsSet(cfont.inverse, 7))
	{ //STD FONTS	
		for(int16_t j = FONT_HEIGHT * scale - 1; j > -1; j--)
		{
			for(int8_t k = 0; k < FONT_WIDTH / 8; k++)
			{
				char frame = cfont.font[temp++];
				for(uint8_t i = 0; i < 8 * scale; i++)
				{
					if(BitIsSet(frame, 7 - i / scale))
						SSD1306_Pixel(x + k * 8 * scale + i, y + j, WHITE);
					else
					{
						if(!_transparent)
							SSD1306_Pixel(x + k * 8 * scale + i, y + j, BLACK);
					}
				}
			}
			if(j % (scale) != 0)
				temp -= 2;
		}
	}
	else
	{ //NOT STD FONTS		
		for(uint16_t j = 0; j < FONT_WIDTH * (FONT_HEIGHT / 8 + FONT_HEIGHT % 8 ? 1 : 0) * scale; j++)
		{
			char frame = cfont.font[temp];

			for(uint8_t i = 0; i < FONT_HEIGHT * scale; i++)
			{
				if(BitIsSet(frame, cfont.inverse & 0x01 ? (7 - i / scale) : (i / scale)))
					SSD1306_Pixel(x + j, y + i, WHITE);
				else
				{
					if(!_transparent)
						SSD1306_Pixel(x + j, y + i, BLACK);
				}
			}
			if(j % scale == (scale - 1))
				temp++;
		}
	}

#if 0
	uint8_t t = x;
	x = y;
	y = t;

	char frame;
	uint16_t temp;

	if (!_transparent)
	{
		if (orient == PORTRAIT)
		{
			//setXY(x, y, x+cfont.x_size-1, y+cfont.y_size-1);

			temp = ((s - cfont.offset) * ((cfont.y_size / 8) * cfont.x_size))
			+ 5;

			for (uint16_t j = 0;
				j < ((cfont.y_size / 8) * cfont.x_size) * scale; j++)
			{
				frame = cfont.font[temp];

				for (uint8_t i = 0; i < 8 * scale; i++)
				{
					if (BitIsSet(frame,
							cfont.inverse ? (7 - i / scale) : (i / scale)))
					SSD1306_Pixel(x + i, y + j, WHITE);
					else
					SSD1306_Pixel(x + i, y + j, BLACK);
				}
				if (j % scale == (scale - 1))
				temp++;
			}
		}
		else
		{ //LANDSCAPE
			temp = ((s - cfont.offset) * ((cfont.x_size / 8) * cfont.y_size))
			+ 5;

			for (uint16_t j = 0; j < ((cfont.x_size / 8) * cfont.y_size);
				j += (cfont.x_size / 8))
			{
				//setXY(x,y+(j/(cfont.x_size/8)),x+cfont.x_size-1,y+(j/(cfont.x_size/8)));

				for (int zz = (cfont.x_size / 8) - 1; zz >= 0; zz--)
				{
					frame = cfont.font[temp + zz];
					for (uint8_t i = 0; i < 8; i++)
					{
						if ((frame & (1 << i)) != 0)
						SSD1306_Pixel(y + j, x + i, WHITE);
						else
						SSD1306_Pixel(y + j, x + i, BLACK);
					}
				}
				temp += (cfont.x_size / 8);
			}
		}
	}
	else
	{
		/*temp = ((c-cfont.offset)*((cfont.x_size/8)*cfont.y_size))+5 ;
		 for(uint16_t j=0; j<cfont.y_size; j++)
		 {
		 for(int zz=0; zz<(cfont.x_size/8); zz++)
		 {
		 frame = cfont.font[temp+zz];
		 for(uint8_t i=0; i<8; i++)
		 {
		 setXY(x+i+(zz*8), y+j, x+i+(zz*8)+1, y+j+1);

		 if((frame & (1<<(7-i))) != 0)
		 SSD1306_Pixel(fillColor);
		 }
		 }
		 temp += (cfont.x_size/8);
		 }*/
	}
#endif
}

/*******************************************************************************
 * Function Name  :
 * Description    :
 *******************************************************************************/
void SSD1306_String(int x, int y, char *s)
{
	while(*s != '\0')
	{
		SSD1306_Char(x, y, *s);
		x += cfont.x_size + 1;

		s++;
	}

	/*uint16_t memClr = fillColor;
	 ILI9327_setColor(backColor);
	 ILI9327_RectFill(x, y, x+cfont.x_size*6, y+cfont.y_size);
	 ILI9327_setColor(memClr);*/
}

/*******************************************************************************
 * Function Name  :
 * Description    :
 *******************************************************************************/
void SSD1306_StringCentered(int x, int y, char *s, uint8_t scale)
{
	if(scale == 0) scale = 1;
	int len = strlen(s);
	if((x+len*(cfont.x_size+1))<64)
		x+= (64-len*(cfont.x_size*scale+1))/2;
		
	while(*s != '\0')
	{
		SSD1306_MultChar(x, y, *s, scale);
		x += cfont.x_size*scale + 1;

		s++;
	}

	/*uint16_t memClr = fillColor;
	 ILI9327_setColor(backColor);
	 ILI9327_RectFill(x, y, x+cfont.x_size*6, y+cfont.y_size);
	 ILI9327_setColor(memClr);*/
}


/*******************************************************************************
 * Function Name  :
 * Description    :
 *******************************************************************************/
void SSD1306_StringPtr(int x, int y, char *s, int start, int end)
{
	for(uint16_t i = start; i < end; i++)
	{
		SSD1306_Char(x, y, *(s + i));
		x += cfont.x_size + 1;
	}
}

/*******************************************************************************
 * Function Name  :
 * Description    :
 *******************************************************************************/
void SSD1306_BigString(int x, int y, char *s, uint8_t scale)
{
	while(*s != '\0')
	{
		SSD1306_MultChar(x, y, *s++, scale);
		x += cfont.x_size * scale + 1;
	}

	/*uint16_t memClr = fillColor;
	 ILI9327_setColor(backColor);
	 ILI9327_RectFill(x, y, x+cfont.x_size*6, y+cfont.y_size);
	 ILI9327_setColor(memClr);*/
}

/*******************************************************************************
 * Function Name  :
 * Description    :
 *******************************************************************************/
void SSD1306_Num(int x, int y, int32_t num)
{
	char d[25];
	itoa_(num, d);
	SSD1306_String(x, y, d);
}

/*******************************************************************************
 * Function Name  :
 * Description    : Display big number with scale
 *******************************************************************************/
void SSD1306_BigNum(int x, int y, int32_t num, int scale)
{
	char /*t[4], */d[25] =
	{ '\0' };

	itoa_(num, d);
	/*if(num<10) d[0] = '0';
	 strcat_(d, t);  */

	SSD1306_BigString(x, y, d, scale);
}

/*******************************************************************************
 * Function Name  :
 * Description    :
 *******************************************************************************/
void SSD1306_Float(int x, int y, float num, int precision)
{
	char d[25];
	ftoa_(num, d, precision);
	SSD1306_String(x, y, d);
}

/*******************************************************************************
 * Function Name  :
 * Description    :
 *******************************************************************************/
void SSD1306_NumWDesc(int x, int y, char *s, int32_t num)
{
	char o[strlen(s) + 15], d[25];
	o[0] = '\0';
	strcat_(o, s);
	itoa_(num, d);
	strcat_(o, d);
	SSD1306_String(x, y, o);
}

/*******************************************************************************
 * Function Name  :
 * Description    :
 *******************************************************************************/
void SSD1306_FloatWDesc(int x, int y, char *s, float num, int precision)
{
	char o[strlen(s) + 18], d[25];
	o[0] = '\0';
	strcat_(o, s);
	ftoa_(num, d, precision);
	strcat_(o, d);
	SSD1306_String(x, y, o);
}

void SSD1306_InvertRect(int x, int y, int width, int height)
{
	for(int i = 0; i < width; i++)
		for(int j = 0; j < height; j++)
			SSD1306_Pixel(x + i, y + j, INVERSE);
}

void SSD1306_Bitmap(int x, int y, char mass[])
{
#define width mass[0]
#define height mass[1]

	// if(height >= 8)
	{
		for(uint8_t i = 0; i < height; i++)
			for(uint8_t j = 0; j < width; j++)
			{
				if(BitIsSet(mass[2 + i + width * (height / 8 - 1 - (j / 8))], 7 - j % 8))
					SSD1306_Pixel(x + i, y + j, WHITE);
				else
					SSD1306_Pixel(x + i, y + j, BLACK);
			}
	}
	/* else
	 {
	 for(uint8_t i=0; i<width; i++)
	 for(uint8_t j=0; j<height; j++)
	 {
	 if(BitIsSet(mass[2+i+width*(height/8-1-(j/8))], j))
	 SSD1306_Pixel(x+j, y+i, WHITE);
	 else
	 SSD1306_Pixel(x+j, y+i, BLACK);
	 }
	 }*/

}

void SSD1306_BitmapPartial(int x, int y, const char mass[],int y1, int y2)
{
#define width mass[0]
#define height mass[1]

	if(y+width<y1 || y+0>y2) return;
	// if(height >= 8)
	{
		for(uint8_t i = 0; i < height; i++)
			for(uint8_t j = 0; j < width; j++)
			{
				if(BitIsSet(mass[2 + i + width * (height / 8 - 1 - (j / 8))], 7 - j % 8))
				{
					if((y+j)>=y1 && (y+j)<=y2)
					SSD1306_Pixel(x + i, y + j, WHITE);
				}
				else
				{
					if((y+j)>=y1 && (y+j)<=y2)
					SSD1306_Pixel(x + i, y + j, BLACK);
				}
			}
	}
}

void SSD1306_BitmapPartial3(int x, int y, const uint8_t mass[],int y1, int y2, int n)
{
#define width mass[0]
#define height mass[1]

	if(y+width<y1 || y+0>y2) return;
	// if(height >= 8)
	{
		for(uint8_t i = 0; i < height; i++)
			for(uint8_t j = 0; j < width; j++)
			{
				if(BitIsSet(mass[2 + i + width * (height / 8 - 1 - (j / 8))], 7 - j % 8))
				{
					if((y+j)>=y1 && (y+j)<=y2)
					{
						SSD1306_Pixel(x + i, y + j, WHITE);
						if(n>1)
							SSD1306_Pixel(x + i-22, y + j, WHITE);
						if(n>2)
							SSD1306_Pixel(x + i-44, y + j, WHITE);
					}
				}
				else
				{
					if((y+j)>=y1 && (y+j)<=y2)
					{
						SSD1306_Pixel(x + i, y + j, BLACK);
						if(n>1)
							SSD1306_Pixel(x + i-22, y + j, BLACK);
						if(n>2)
							SSD1306_Pixel(x + i-44, y + j, BLACK);
					}
				}
			}
	}
}



void SSD1306_HLineFull(uint8_t y, uint8_t color)
{
	SSD1306_Command (SSD1306_COLUMNADDR);
	SSD1306_Command(y); // Column start address (0 = reset)
	SSD1306_Command(y); // Column start address (0 = reset)

	SSD1306_Command (SSD1306_PAGEADDR);
	SSD1306_Command(0); // Page start address (0 = reset)
	SSD1306_Command(7); // Page end address

	DC_H;
	CS_L;
	for(uint16_t j = 0; j < (8); j++)
	{
		if(color == BLACK)
			SPI_Send(0);
		else
			SPI_Send(0xFF);
	}
	CS_H;
}

void SSD1306_EndSplash()
{
	for(uint8_t i = 0; i < 63; i++)
	{
		/*drawFastHLine(0,127-i, 64, BLACK);
		 drawFastHLine(0,127-i-1, 64, WHITE);
		 drawFastHLine(0,0+i+1, 64, WHITE);
		 drawFastHLine(0,0+i, 64, BLACK);
		 SSD1306_ReDraw();*/
		SSD1306_HLineFull(i, BLACK);
		SSD1306_HLineFull(i + 1, WHITE);
		SSD1306_HLineFull(i + 2, WHITE);
		SSD1306_HLineFull(127 - i, BLACK);
		SSD1306_HLineFull(127 - i - 1, WHITE);
		SSD1306_HLineFull(127 - i - 2, WHITE);

		nrf_delay_ms(3);
	}
}

void SSD1306_Checkbox(int x, int y, bool state)
{
	SSD1306_Rect(x, y, x + 5, y + 5);
	if(state)
	{
		SSD1306_Pixel(x + 2, y + 1, WHITE);
		SSD1306_Pixel(x + 1, y + 2, WHITE);
		SSD1306_Pixel(x + 3, y + 2, WHITE);
		SSD1306_Pixel(x + 4, y + 3, WHITE);
		SSD1306_Pixel(x + 4, y + 4, WHITE);
	}
}

