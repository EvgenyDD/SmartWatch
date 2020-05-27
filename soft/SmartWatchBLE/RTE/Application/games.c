/* Includes ------------------------------------------------------------------*/
#include "games.h"
#include "periph.h"

#include "sound.h"
#include "LSM303.h"
#include "dispatcher.h"
#include "BLE.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
#define swap(i, j) {uint16_t t = i; i = j; j = t;}

/* Private variables ---------------------------------------------------------*/
/* Extern variables ----------------------------------------------------------*/
extern volatile int16_t countDown;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
 * Function Name  :
 * Description    :
 *******************************************************************************/

#define PLATFORM_WIDTH	12
#define PLATFORM_HEIGHT	4
#define UPT_MOVE_NONE	0
#define UPT_MOVE_RIGHT	1
#define UPT_MOVE_LEFT	2
#define BLOCK_COLS		32
#define BLOCK_ROWS		5
#define BLOCK_COUNT		(BLOCK_COLS * BLOCK_ROWS)

bool btnExit(void);
bool btnRight(void);
bool btnLeft(void);
void draw(void);

typedef struct
{
	float x;
	float y;
	float velX;
	float velY;
} s_ball;

uint8_t uptMove;
s_ball ball;
bool blocks[(BLOCK_COLS * BLOCK_ROWS)];
uint8_t lives = 10;
uint16_t score;
uint8_t platformX;

const char block[] =
{
/*3,3,*/8, 8, 0x07, 0x07, 0x07, };

const char platform[] =
{ 8, 12, 0x60, 0x70, 0x50, 0x10, 0x30, 0xF0, 0xF0, 0x30, 0x10, 0x50, 0x70, 0x60 };

const char livesImg[] =
{
/*7*/8, 8, 0x06, 0x0F, 0x1F, 0x3E, 0x1F, 0x0F, 0x06, };

#define SSD1306_BITMAP(x,y,z) SSD1306_Bitmap((SSD1306_LCDHEIGHT-(x)),y,z)

extern uint8_t orient;

void Game1(uint16_t button)
{
	switch (button)
	{
	case START_COM:
		orient = LANDSCAPE;
		SSD1306_Clear();
		SSD1306_setFont(font5x8);
		SSD1306_StringCentered(0, 25, "Breakout",1);
		SSD1306_StringCentered(0, 15, "Game",1);

		SSD1306_ReDraw();
		nrf_delay_ms(1500);

		game1_start();
		break;
	case STOP_COM:
		orient = PORTRAIT;
		SSD1306_Clear();
		SSD1306_ReDraw();
		break;
	case BTN_MENU:
		MenuChangeMain();
		break;
	case BTN_UP:
		uptMove = UPT_MOVE_LEFT;
		break;
	case BTN_DOWN:
		uptMove = UPT_MOVE_RIGHT;
		break;
	}

	draw();
	countDown = 4*DISP_1s;
}

void game1_start()
{
	uptMove = UPT_MOVE_NONE;

	ball.x = SSD1306_LCDWIDTH / 2;
	ball.y = SSD1306_LCDHEIGHT - 10;
	ball.velX = -1;
	ball.velY = -1.1;

	//blocks = calloc(BLOCK_COUNT, 1);

	lives = 3;
	score = 0;
	platformX = (SSD1306_LCDWIDTH / 2) - (PLATFORM_WIDTH / 2);
}

bool btnExit()
{
	//free(blocks);
	//if(lives == 255)
	game1_start();
	/*else
	 animation_start(display_load, ANIM_MOVE_OFF);*/
	return true;
}

extern int ACCEL[3], MAGNET[3];

void draw()
{
	SSD1306_Clear();

	//NRF_WDT->RR[0] = 0x6E524635; //Reload watchdog register 0

	bool gameEnded = false; //((score >= BLOCK_COUNT) || (lives == 255));

	uint8_t platformXtmp = platformX;

	// Move platform
	/*if(uptMove == UPT_MOVE_RIGHT)
	 platformXtmp += 3;
	 else if(uptMove == UPT_MOVE_LEFT)
	 platformXtmp -= 3;
	 uptMove = UPT_MOVE_NONE;*/

	LSM303_GetAccelData(ACCEL);

	static uint8_t loseWeight = 0;
	if(++loseWeight >= 2)
	{
		loseWeight = 0;
		platformXtmp += ACCEL[XAxis] / 80;
	}

	// Make sure platform stays on screen
	if(platformXtmp > 180)
		platformXtmp = 0;
	else if(platformXtmp > SSD1306_LCDWIDTH - PLATFORM_WIDTH)
		platformXtmp = SSD1306_LCDWIDTH - PLATFORM_WIDTH;

	// Draw platform
	SSD1306_Bitmap(platformXtmp, 0, (char*) platform);

//draw_bitmap(platformXtmp, SSD1306_LCDHEIGHT - 8, platform, 12, 8, NOINVERT, 0);

	platformX = platformXtmp;

	// Move ball
	if(!gameEnded)
	{
		ball.x += ball.velX;
		ball.y += ball.velY;
	}

	bool blockCollide = false;
	uint8_t ballX = ball.x;
	uint8_t ballY = ball.y;

	// Block collision
	uint8_t idx = 0;
	for(uint8_t x = 0; x < BLOCK_COLS; x++)
	{
		for(uint8_t y = 0; y < BLOCK_ROWS; y++)
		{
			if(!blocks[idx] && ballX >= x * 4 && ballX < (x * 4) + 4 && ballY >= (y * 4) + 8 && ballY < (y * 4) + 8 + 4)
			{
				//buzzer_buzz(100, TONE_2KHZ, VOL_UI, PRIO_UI, NULL);
				//led_flash(LED_GREEN, 50, 255);
				blocks[idx] = true;
				blockCollide = true;
				score++;
			}
			idx++;
		}
	}

	// Side wall collision
	if(ballX > SSD1306_LCDWIDTH - 2)
	{
		ball.x = SSD1306_LCDWIDTH - 2;
		ball.velX *= -1;
	}
	if(ball.x < 0)
	{
		ball.x = 0;
		ball.velX *= -1;
	}

	// Platform collision
	bool platformCollision = false;
	if(!gameEnded && ballY >= SSD1306_LCDHEIGHT - PLATFORM_HEIGHT
		&& ballY < 240&& ballX >= platformX && ballX <= platformX + PLATFORM_WIDTH)
	{
		platformCollision = true;
		SoundBeep(3000, 30); //buzzer_buzz(200, TONE_5KHZ, VOL_UI, PRIO_UI, NULL);
		ball.y = SSD1306_LCDHEIGHT - PLATFORM_HEIGHT;
		if(ball.velY > 0)
			ball.velY *= -1;
		ball.velX = ((float) rand() / (RAND_MAX / 2)) - 1; // -1.0 to 1.0
	}

	// Top/bottom wall collision
	if(!gameEnded && !platformCollision && (ballY > SSD1306_LCDHEIGHT - 2 || blockCollide))
	{
		if(ballY > 240)
		{
			SoundBeep(4000, 30); //buzzer_buzz(200, TONE_2_5KHZ, VOL_UI, PRIO_UI, NULL);
			ball.y = 0;
		}
		else if(!blockCollide)
		{
			SoundBeep(4500, 40); //buzzer_buzz(200, TONE_2KHZ, VOL_UI, PRIO_UI, NULL);
			SoundBeep(4300, 40); //buzzer_buzz(200, TONE_2KHZ, VOL_UI, PRIO_UI, NULL);
			ball.y = SSD1306_LCDHEIGHT - 1;
			lives--;
		}
		ball.velY *= -1;
	}

	// Draw ball
	for(uint8_t u = 0; u < 2; u++)
		for(uint8_t o = 0; o < 2; o++)
			SSD1306_Pixel(ball.x + o, 63 - (ball.y + u), WHITE);
	//SSD1306_BITMAP(ball.x, ball.y, (char*)ballImg);

	//draw_bitmap(ball.x, ball.y, ballImg, 2, 8, NOINVERT, 0);

	// Draw blocks
	idx = 0;
	for(uint8_t x = 0; x < BLOCK_COLS; x++)
	{
		for(uint8_t y = 0; y < BLOCK_ROWS; y++)
		{
			if(!blocks[idx])
			{
				for(uint8_t i = 0; i < 3; i++)
					for(uint8_t j = 0; j < 3; j++)
						SSD1306_Pixel(x * 4 + i, 63 - ((y * 4) + 8 + j), WHITE);
			}

			//SSD1306_BITMAP(x * 4, (y * 4) + 8, (char*)block);

//				draw_bitmap(x * 4, (y * 4) + 8, block, 3, 8, NOINVERT, 0);
			idx++;
		}
	}

	// Draw score
//	char buff[6];
//	sprintf_P(buff, PSTR("%u"), score);
//	draw_string(buff, false, 0, 0);
	SSD1306_Num(0, 10, score);

	// Draw lives
	if(lives != 255)
	{
		for(uint8_t i = 0; i < lives; i++)
			SSD1306_BITMAP((SSD1306_LCDWIDTH - (3*8)) + (8*i), 1, (char*)livesImg);
//			draw_bitmap((SSD1306_LCDWIDTH - (3*8)) + (8*i), 1, livesImg, 7, 8, NOINVERT, 0);
	}

	// Got all blocks
	if(score >= BLOCK_COUNT)
		SSD1306_String(0, 32, "YOU WIN");
//		draw_string_P(PSTR(STR_WIN), false, 50, 32);

	// No lives left (255 because overflow)
	if(lives == 255)
	{
		SSD1306_String(45, 25, "YOU LOSE");
		SSD1306_ReDraw();
		nrf_delay_ms(2000);
		UIShortButton (START_COM);
		MenuCallback();
	}

//		draw_string_P(PSTR(STR_GAMEOVER), false, 34, 32);

	SSD1306_ReDraw();
}




///////////////////////////////////////////////////////////////////////////////
///////////////////////////// SLOT MACHINE ////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
const uint8_t SlotSeven[] = { 16,16,
0x00,0x00,0x00,0x1C,0x1C,0x1C,0x1C,0x9C,0xDC,0xFC,0xFC,0x7C,0x3C,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x78,0x7F,0x7F,0x07,0x00,0x00,0x00,0x00,0x00,0x00,
};
const uint8_t SlotApple[] = { 16,16,
0x00,0x80,0xC0,0xE0,0xF0,0xF0,0xE0,0xFC,0xFE,0xEE,0xF6,0xF0,0xE0,0xC0,0x00,0x00,
0x00,0x1F,0x3F,0x7F,0xFF,0xFF,0xFF,0x7F,0xFF,0xFF,0xFF,0xFF,0x7F,0x3F,0x0F,0x00,
};
const uint8_t SlotBar[] = { 16,16,
0x1F,0xDF,0xDF,0x1F,0xFF,0xFF,0x1F,0xDF,0xDF,0x1F,0xFF,0x1F,0xDF,0xDF,0x1F,0xFF,
0xE0,0xEE,0xEE,0xEE,0xE0,0xFF,0xE0,0xFD,0xFD,0xE0,0xFF,0xE0,0xFC,0xF8,0xE6,0xFF,
};
const uint8_t SlotBell[] = { 16,16,
0x00,0x00,0x00,0x00,0x80,0xF8,0xFC,0xFE,0xFE,0xFC,0xF8,0x80,0x00,0x00,0x00,0x00,
0x00,0x00,0x10,0x1C,0x1F,0x1F,0x5F,0xDF,0xDF,0x5F,0x1F,0x1F,0x1C,0x10,0x00,0x00,
};
const uint8_t SlotCherry[] = { 16,16,
0x00,0x00,0x00,0x00,0xC0,0x70,0x18,0x0C,0x06,0x0C,0x38,0xE4,0x06,0x0E,0x0E,0x04,
0x00,0x1C,0x3E,0x3E,0x3F,0x1C,0x00,0x00,0x00,0x1C,0x3E,0x3F,0x3E,0x3E,0x1C,0x00,
};

#define SLOT_OFFSET 70
static void SlotSpinNTimes(uint8_t numOfSpin, uint8_t numOfSpinners, uint8_t endnum)
{
	int16_t pos = SLOT_OFFSET+1;
	
	for(uint8_t i=0; i<numOfSpin; pos--)
	{
		SSD1306_BitmapPartial3(46,pos,SlotCherry, SLOT_OFFSET+2,SLOT_OFFSET+17,numOfSpinners);
		SSD1306_BitmapPartial3(46,pos+18,SlotApple, SLOT_OFFSET+2,SLOT_OFFSET+17,numOfSpinners);
		SSD1306_BitmapPartial3(46,pos+18*2,SlotBell, SLOT_OFFSET+2,SLOT_OFFSET+17,numOfSpinners);
		SSD1306_BitmapPartial3(46,pos+18*3,SlotSeven, SLOT_OFFSET+2,SLOT_OFFSET+17,numOfSpinners);
		SSD1306_BitmapPartial3(46,pos+18*4,SlotBar, SLOT_OFFSET+2,SLOT_OFFSET+17,numOfSpinners);
			SSD1306_BitmapPartial3(46,pos+18*5,SlotCherry, SLOT_OFFSET+2,SLOT_OFFSET+17,numOfSpinners);
		
		SSD1306_ReDraw();
		
		if(pos <= SLOT_OFFSET - 18*5 + 1)
		{
			pos = SLOT_OFFSET+1;
			i++;
		}
	}
	countDown = 15*DISP_1s;	
	
	while(pos != (endnum*18-1))
	{
		SSD1306_BitmapPartial3(46,pos,SlotCherry, SLOT_OFFSET+2,SLOT_OFFSET+17,numOfSpinners);
		SSD1306_BitmapPartial3(46,pos+18,SlotApple, SLOT_OFFSET+2,SLOT_OFFSET+17,numOfSpinners);
		SSD1306_BitmapPartial3(46,pos+18*2,SlotBell, SLOT_OFFSET+2,SLOT_OFFSET+17,numOfSpinners);
		SSD1306_BitmapPartial3(46,pos+18*3,SlotSeven, SLOT_OFFSET+2,SLOT_OFFSET+17,numOfSpinners);
		SSD1306_BitmapPartial3(46,pos+18*4,SlotBar, SLOT_OFFSET+2,SLOT_OFFSET+17,numOfSpinners);
			SSD1306_BitmapPartial3(46,pos+18*5,SlotCherry, SLOT_OFFSET+2,SLOT_OFFSET+17,numOfSpinners);
		
		SSD1306_ReDraw();
		
		if(--pos <= SLOT_OFFSET - 18*5 + 1)
			pos = SLOT_OFFSET+1;
	}	
	countDown = 15*DISP_1s;		
}

uint8_t SlotSpin(uint16_t code)
{
	static uint8_t isRunning = 1;
	
	isRunning = 0;
	switch(code)
	{
	case START_COM:
		APP_ERROR_CHECK(nrf_drv_rng_init(NULL));
		SSD1306_setFont(font5x8);
		isRunning = 1;
		break;
	case STOP_COM:
		nrf_drv_rng_uninit();
		break;
	case BTN_UP:
	case BTN_RIGHT:
	case BTN_LEFT:
	case BTN_DOWN:
	case BTN_MENU:
		isRunning = 1;
		break;
	}
	
	countDown = 15*DISP_1s;		

	uint8_t p_buff[3];
	uint8_t length = random_vector_generate(p_buff, 3);
	
	if(isRunning == 0) return 0;
 	

	SSD1306_Clear();
	SSD1306_Rect(0,SLOT_OFFSET,19,SLOT_OFFSET+19);
	SSD1306_Rect(22,SLOT_OFFSET,22+19,SLOT_OFFSET+19);
	SSD1306_Rect(44,SLOT_OFFSET,44+19,SLOT_OFFSET+19);
	
	SlotSpinNTimes(2,3,p_buff[0]%5);
	SlotSpinNTimes(2,2,p_buff[1]%5);
	SlotSpinNTimes(2,1,p_buff[2]%5);
		
	return 0;
}




