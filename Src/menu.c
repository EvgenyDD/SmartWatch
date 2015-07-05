/* Includes ------------------------------------------------------------------*/
#include "menu.h"
#include "UI.h"
#include "hardware.h"
#include "SSD1306.h"
#include "games.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
extern uint16_t xxxx;

uint16_t command = 0;

MenuItem*	CurrWorkItem;
MenuItem*   CurrScreen;

M_M(MainClock,  BinarClock,ExtndClock,NULL_MENU,Menu, NULL_FUNC,MainScreen,NULL_FUNC);
M_M(BinarClock, ExtndClock,MainClock,NULL_MENU,Menu, NULL_FUNC,BinarScreen,NULL_FUNC);
M_M(ExtndClock, MainClock,BinarClock,NULL_MENU,Menu, NULL_FUNC,ExtendScreen,NULL_FUNC);

M_M(Menu,       NULL_MENU,NULL_MENU,MainClock,NULL_MENU, NULL_FUNC,menuScreen,NULL_FUNC);

M_M(Stopwatch,  NULL_MENU,NULL_MENU,MainClock,NULL_MENU, NULL_FUNC,NULL_FUNC,NULL_FUNC);
    M_M(Timer,      NULL_MENU,NULL_MENU,MainClock,NULL_MENU, NULL_FUNC,NULL_FUNC,NULL_FUNC);
M_M(Flashlight, NULL_MENU,NULL_MENU,MainClock,NULL_MENU, NULL_FUNC,NULL_FUNC,NULL_FUNC);
M_M(Settings,   NULL_MENU,NULL_MENU,MainClock,NULL_MENU, NULL_FUNC,SettScreen,NULL_FUNC);
M_M(Random,     NULL_MENU,NULL_MENU,MainClock,NULL_MENU, NULL_FUNC,NULL_FUNC,NULL_FUNC);
M_M(Calendar,   NULL_MENU,NULL_MENU,MainClock,NULL_MENU, NULL_FUNC,NULL_FUNC,NULL_FUNC);
M_M(Alarm,      NULL_MENU,NULL_MENU,MainClock,NULL_MENU, NULL_FUNC,SetAlarm,NULL_FUNC);
M_M(Text,       NULL_MENU,NULL_MENU,MainClock,NULL_MENU, NULL_FUNC,NULL_FUNC,NULL_FUNC);

M_M(Game,       NULL_MENU,NULL_MENU,MainClock,NULL_MENU, Game1Intro,Game1,Game1End);

M_M(SetTime,       NULL_MENU,NULL_MENU,Settings,NULL_MENU, NULL_FUNC,TDScreen,NULL_FUNC);

/*WORK_MENU(,  NULL_MENU,NULL_MENU,NULL_MENU,NULL_MENU, NULL_FUNC,NULL_FUNC,NULL_FUNC);
WORK_MENU(Stopwatch,  NULL_MENU,NULL_MENU,NULL_MENU,NULL_MENU, NULL_FUNC,NULL_FUNC,NULL_FUNC);
WORK_MENU(Stopwatch,  NULL_MENU,NULL_MENU,NULL_MENU,NULL_MENU, NULL_FUNC,NULL_FUNC,NULL_FUNC);
WORK_MENU(Stopwatch,  NULL_MENU,NULL_MENU,NULL_MENU,NULL_MENU, NULL_FUNC,NULL_FUNC,NULL_FUNC);
WORK_MENU(Stopwatch,  NULL_MENU,NULL_MENU,NULL_MENU,NULL_MENU, NULL_FUNC,NULL_FUNC,NULL_FUNC);*/


/*(_Set_OnOff, NULL_MENU, 	NULL_MENU, 	NULL_MENU, 	NULL_MENU,	NULL_FUNC);
M_M(_Set_OnTim, NULL_MENU,	NULL_MENU, 	NULL_MENU, 	NULL_MENU,	NULL_FUNC);
M_M(_Set_OffTim,NULL_MENU, 	NULL_MENU, 	NULL_MENU, NULL_MENU,	NULL_FUNC);*/


MenuItem	Null_Menu = {(void*)0, (void*)0, (void*)0, (void*)0, NULL_FUNC, NULL_FUNC,NULL_FUNC};

/* Extern variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  :
* Description    :
* Input		 :
*******************************************************************************/

/*******************************************************************************
* Function Name  :
* Description    :
*******************************************************************************/
void SetStartScreen()
{
    MenuChange(&MainClock);
    CurrScreen = &MainClock;
}

void MenuChangeMain()
{
    MenuChange(CurrScreen);
}

void MenuChange(MenuItem *newMenu)
{
    if(CurrWorkItem->EnterFunc != NULL_FUNC)
        CurrWorkItem->LeaveFunc();
    CurrWorkItem = newMenu;
    if(CurrWorkItem->EnterFunc != NULL_FUNC)
        CurrWorkItem->EnterFunc();
    //MenuCallback(0);
}

extern int j;
void UIShortButton(uint16_t state)
{
    if(CHILD == &NULL_MENU)
    {
        /* in menu */
        j=100;
    }
    else
    {
        /* in screen */
        if(state == BTN_UP)
        {
            CurrScreen = NEXT;
            MenuChange(NEXT);
        }

        if(state == BTN_DOWN)
        {
            CurrScreen = PREVIOUS;
            MenuChange(PREVIOUS);
        }

        if(state == BTN_MENU)
        {
            MenuChange(CHILD);
            return;
        }
    }

    command = state;
    //MenuCallback(state);
}

void UILongButton(uint16_t state)
{
    if(state == BTN_MENU_LONG)
        if(PARENT != &NULL_MENU)
            MenuChangeMain();
}


void MenuCallback()
{
    bool temp = false;

    if(CurrWorkItem->Callback != NULL_FUNC)
    {
        SSD1306_Clear();
        if(command) temp = true;
        CurrWorkItem->Callback(command);
        if(temp) {command = 0; temp = false;}
        SSD1306_ReDraw();
    }

}
