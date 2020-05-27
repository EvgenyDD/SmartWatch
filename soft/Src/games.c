/* Includes ------------------------------------------------------------------*/
#include "games.h"

#include "sound.h"
#include "LSM303.h"


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
#define swap(i, j) {uint16_t t = i; i = j; j = t;}

/* Private variables ---------------------------------------------------------*/
/* Extern variables ----------------------------------------------------------*/
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

typedef struct{
	float x;
	float y;
	float velX;
	float velY;
}s_ball;

uint8_t uptMove;
s_ball ball;
bool blocks[(BLOCK_COLS * BLOCK_ROWS)];
uint8_t lives=10;
uint16_t score;
uint8_t platformX;

extern int j;

const char block[] ={
    /*3,3,*/8,8,
	0x07,0x07,0x07,
};

const char platform[] ={
    12,8,
	0x60,0x70,0x50,0x10,0x30,0xF0,0xF0,0x30,0x10,0x50,0x70,0x60,
};

const char ballImg[] ={
    /*2,2,*/8,8,
	0x03,0x03,
};

const char livesImg[] ={
    /*7*/8,8,
	0x06,0x0F,0x1F,0x3E,0x1F,0x0F,0x06,
};

#define SSD1306_PIXEL(x,y,z) SSD1306_Pixel((SSD1306_LCDHEIGHT-(x)),y,z)
#define SSD1306_BITMAP(x,y,z) SSD1306_Bitmap((SSD1306_LCDHEIGHT-(x)),y,z)

extern uint8_t orient;

void Game1Intro()
{
    orient = LANDSCAPE;
    SSD1306_Clear();

    SSD1306_String(0, 25, "Breakout");
    SSD1306_String(0, 15, "  Game  ");

    SSD1306_ReDraw();
    nrf_delay_ms(1500);

    game1_start();
}

void Game1End()
{
    orient = PORTRAIT;
    SSD1306_Clear();
    SSD1306_ReDraw();
}


void Game1(uint16_t button)
{
    if(button == BTN_MENU)
    {
        MenuChangeMain();
    }
    if(button == BTN_UP) uptMove = UPT_MOVE_LEFT;
    if(button == BTN_DOWN) uptMove = UPT_MOVE_RIGHT;

    draw();
    j = 40;
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


extern volatile int ACCEL[3], MAGNET[3];


void draw()
{
    SSD1306_Clear();

    NRF_WDT->RR[0] = 0x6E524635; //Reload watchdog register 0

	bool gameEnded = false;//((score >= BLOCK_COUNT) || (lives == 255));

	uint8_t platformXtmp = platformX;

	// Move platform
	/*if(uptMove == UPT_MOVE_RIGHT)
		platformXtmp += 3;
	else if(uptMove == UPT_MOVE_LEFT)
		platformXtmp -= 3;
	uptMove = UPT_MOVE_NONE;*/

	static uint8_t loseWeight = 0;
	if(++loseWeight >= 2)
    {
        loseWeight = 0;
        platformXtmp += ACCEL[XAxis]/80;
    }

	// Make sure platform stays on screen
	if(platformXtmp > 250)
		platformXtmp = 0;
	else if(platformXtmp > SSD1306_LCDWIDTH - PLATFORM_WIDTH)
		platformXtmp = SSD1306_LCDWIDTH - PLATFORM_WIDTH;

	// Draw platform
	SSD1306_Bitmap(platformXtmp, 0, (char*)platform);

//draw_bitmap(platformXtmp, SSD1306_LCDHEIGHT - 8, platform, 12, 8, NOINVERT, 0);

	platformX = platformXtmp;

	// Move ball
	if(!gameEnded)
	{
		ball.x += ball.velX;
		ball.y += ball.velY;
	}

	bool blockCollide = false;
	const uint8_t ballX = ball.x;
	const uint8_t ballY = ball.y;

	// Block collision
	uint8_t idx = 0;
	for(uint8_t x=0; x<BLOCK_COLS; x++)
	{
		for(uint8_t y=0; y<BLOCK_ROWS; y++)
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
		if(ballX > 240)
			ball.x = 0;
		else
			ball.x = SSD1306_LCDWIDTH - 2;
		ball.velX *= -1;
	}

	// Platform collision
	bool platformCollision = false;
	if(!gameEnded && ballY >= SSD1306_LCDHEIGHT - PLATFORM_HEIGHT && ballY < 240 && ballX >= platformX && ballX <= platformX + PLATFORM_WIDTH)
	{
		platformCollision = true;
//		SoundBeep(3000, 30);//buzzer_buzz(200, TONE_5KHZ, VOL_UI, PRIO_UI, NULL);
		ball.y = SSD1306_LCDHEIGHT - PLATFORM_HEIGHT;
		if(ball.velY > 0)
			ball.velY *= -1;
		ball.velX = ((float)rand() / (RAND_MAX / 2)) - 1; // -1.0 to 1.0
	}

	// Top/bottom wall collision
	if(!gameEnded && !platformCollision && (ballY > SSD1306_LCDHEIGHT - 2 || blockCollide))
	{
		if(ballY > 240)
		{
//			SoundBeep(4000, 30);//buzzer_buzz(200, TONE_2_5KHZ, VOL_UI, PRIO_UI, NULL);
			ball.y = 0;
		}
		else if(!blockCollide)
		{
//			SoundBeep(4500, 30);//buzzer_buzz(200, TONE_2KHZ, VOL_UI, PRIO_UI, NULL);
			ball.y = SSD1306_LCDHEIGHT - 1;
			//lives--;
		}
		ball.velY *= -1;
	}

	// Draw ball
	for(uint8_t u=0; u<2; u++)
        for(uint8_t o=0; o<2; o++)
            SSD1306_PIXEL(ball.y+u, ball.x+o, WHITE);
	//SSD1306_BITMAP(ball.x, ball.y, (char*)ballImg);

    //draw_bitmap(ball.x, ball.y, ballImg, 2, 8, NOINVERT, 0);

	// Draw blocks
	idx = 0;
	for(uint8_t x=0; x<BLOCK_COLS; x++)
	{
		for(uint8_t y=0; y<BLOCK_ROWS; y++)
		{
			if(!blocks[idx])
            {
                for(uint8_t i=0; i<3; i++)
                    for(uint8_t j=0; j<3; j++)
                        SSD1306_PIXEL((y*4)+8+j, x*4+i, WHITE);
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
    SSD1306_Num(0,0,score);

	// Draw lives
	if(lives != 255)
	{
		for(uint8_t i=0; i<lives; i++)
            SSD1306_BITMAP((SSD1306_LCDWIDTH - (3*8)) + (8*i), 1, (char*)livesImg);
//			draw_bitmap((SSD1306_LCDWIDTH - (3*8)) + (8*i), 1, livesImg, 7, 8, NOINVERT, 0);
	}


	// Got all blocks
	if(score >= BLOCK_COUNT)
        SSD1306_String(0,32, "YOU WIN");
//		draw_string_P(PSTR(STR_WIN), false, 50, 32);

	// No lives left (255 because overflow)
	if(lives == 255)
        SSD1306_String(0,32, "YOU LOSE");
//		draw_string_P(PSTR(STR_GAMEOVER), false, 34, 32);
SSD1306_ReDraw();

}

