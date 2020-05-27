/* Includes ------------------------------------------------------------------*/
#include "menu.h"
#include "UI.h"
#include "hardware.h"
#include "SSD1306.h"
#include "games.h"
#include "dispatcher.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
extern uint16_t xxxx;
extern volatile int16_t countDown;


M_M(Menu,       NULL_MENU,NULL_MENU,MainClock,NULL_MENU, menuScreen);

M_M(MainClock,  BIGClock,BIGClock,NULL_MENU,Menu, MainScreen);
M_M(BIGClock,   TextClock1,MainClock,NULL_MENU,Menu, BIGScreen);
M_M(TextClock1, MainClock,BIGClock,NULL_MENU,Menu, TextScreen1);
M_M(ExtndClock, MainClock,BIGClock,NULL_MENU,NULL_MENU, ExtendScreen);

M_M(BinarClock, ExtndClock,ExtndClock,NULL_MENU,Menu, BinarScreen);





M_M(Stopwatch,  NULL_MENU,NULL_MENU,MainClock,NULL_MENU, StopWatchCb);
M_M(TimerMode,      NULL_MENU,NULL_MENU,MainClock,NULL_MENU, TimerCb);
	
M_M(Flashlight, NULL_MENU,NULL_MENU,MainClock,NULL_MENU, FlashLight);
M_M(Settings,   NULL_MENU,NULL_MENU,MainClock,NULL_MENU, SettScreen);
M_M(Random,     NULL_MENU,NULL_MENU,MainClock,NULL_MENU, SlotSpin);
M_M(Calendar,   NULL_MENU,NULL_MENU,MainClock,NULL_MENU, CalendarDraw);
M_M(Alarm,      NULL_MENU,NULL_MENU,MainClock,NULL_MENU, SetAlarm);
M_M(Text,       NULL_MENU,NULL_MENU,MainClock,NULL_MENU, NULL_FUNC);

M_M(Game,       NULL_MENU,NULL_MENU,MainClock,NULL_MENU, Game1);

M_M(SetTime,       NULL_MENU,NULL_MENU,Settings,NULL_MENU, TDScreen);

M_M(LogScrn,	NULL_MENU,NULL_MENU,MainClock,NULL_MENU, LogScreen);
M_M(SlpScrn,	NULL_MENU,NULL_MENU,MainClock,NULL_MENU, DrawSleepLog);

M_M(AllData,	NULL_MENU,NULL_MENU,MainClock,NULL_MENU, AllDataDisplayCb);

MenuItem*	CurrWorkItem = &MainClock;
MenuItem*	NextWorkItem;
MenuItem*   CurrScreen;

static uint32_t command = 0;
static uint32_t menuWasChanged=0;

MenuItem Null_Menu =
{ (void*) 0, (void*) 0, (void*) 0, (void*) 0, /*NULL_FUNC,*/NULL_FUNC /*,NULL_FUNC*/};

/* Extern variables ----------------------------------------------------------*/
extern uint8_t orient;
extern int j;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
 * Function Name  :
 * Description    :
 *******************************************************************************/
void SetStartScreen()
{
	MenuChange (&MainClock);
	CurrScreen = &MainClock;
}

void MenuChangeMain()
{
	MenuChange(CurrScreen);
	countDown = 5*DISP_1s;
}

void MenuChange(MenuItem *newMenu)
{
	NextWorkItem = newMenu;
	menuWasChanged = 1;
}

void UIShortButton(uint16_t state)
{
	
	if(CHILD == &NULL_MENU)
	{
		/* in menu */
	}
	else
	{
		/* in screen */
		if(state == BTN_UP)
		{
			CurrScreen = NEXT;
			MenuChange (NEXT);
		}

		if(state == BTN_DOWN)
		{
			CurrScreen = PREVIOUS;
			MenuChange (PREVIOUS);
		}

		if(state == BTN_MENU)
		{
			MenuChange (CHILD);
			return;
		}
	}

	command = state;
}

void UILongButton(uint16_t state)
{
	if(state == BTN_MENU_LONG)
	{
		if(PARENT != &NULL_MENU)
			MenuChangeMain();
		else
			MenuChange (&LogScrn);
	}

	if(CurrScreen == CurrWorkItem)
	{
		if(state == BTN_UP_LONG)
		{
			MenuChange (&SlpScrn);
		}
		if(state == BTN_DOWN_LONG)
		{
			MenuChange (&AllData);
		}
		if(state == BTN_DOWN_LONG)
		{
			MenuChange (&AllData);
		}
		if(state == BTN_LEFT_LONG)
		{
			MenuChange (&ExtndClock);
		}

		if(state == (BTN_DOWN_LONG | BTN_MENU_LONG))
		{
			MenuChange (&Flashlight);
		}

		if(state == (BTN_UP_LONG | BTN_MENU_LONG))
		{
			MenuChange (&Stopwatch);
		}

		if(state == (BTN_RIGHT_LONG | BTN_MENU_LONG))
		{
			MenuChange (&TimerMode);
		}
	}

	command = state;
}


extern uint32_t alarmNearestTimestamp, RTCCounter;
void MenuCallback()
{
	static uint32_t lastRTCCounter = 0;
	static uint8_t status = 255;
	//orient = PORTRAIT;
	
	if(CurrWorkItem != &ExtndClock) //special for graph plotting function
	if(status==1) 
		SSD1306_Clear();

	//Critical block!!!
	do
	{
		if(menuWasChanged > 0)
		{
			if(CurrWorkItem->Callback != NULL_FUNC)
				CurrWorkItem->Callback(STOP_COM);

			CurrWorkItem = NextWorkItem;
			
			SSD1306_Clear();//!!!

			if(CurrWorkItem->Callback != NULL_FUNC)
				CurrWorkItem->Callback(START_COM);

			menuWasChanged = 0;
		}
		else
		{
			uint32_t shot = command;
			
			if(shot == 0 && lastRTCCounter != RTCCounter)
				shot = NEW_SECOND;
			
			if(CurrWorkItem->Callback != NULL_FUNC)
				status = CurrWorkItem->Callback(shot);
			
			if(command == shot && command > 0)
				command = 0;
			else
				lastRTCCounter = RTCCounter;
		}
	}
	while(0);

	
	extern uint32_t RTCCounter,alarmNearestTimestamp;
//	SSD1306_Num(0,50,DispatcherGetFreeSpace());
	//SSD1306_Num(0,8,alarmNearestTimestamp);
	//SSD1306_Num(0,0,alarmNearestTimestamp-RTCCounter);
	
	if(status>=1)
		SSD1306_ReDraw();
}
