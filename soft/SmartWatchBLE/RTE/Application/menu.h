/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef MENU_H
#define MENU_H

/* Includes ------------------------------------------------------------------*/
#include "nrf.h"

/* Exported types ------------------------------------------------------------*/
typedef void (*funcPtr)(void);
typedef uint8_t (*callPtr)(uint16_t);
//typedef void (*WriteFuncPtr)(const char*);

typedef struct
{
	void *Next;
	void *Previous;
	void *Parent;
	void *Child;
//	funcPtr     EnterFunc;
	callPtr Callback;
//    funcPtr     LeaveFunc;
} MenuItem;

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
#define M_M(Name,   Next, Previous, Parent, Child,  /*EnterFunc,*/ Callback/*, LeaveFunc*/) \
    extern MenuItem Next;     \
	extern MenuItem Previous; \
	extern MenuItem Parent;   \
	extern MenuItem Child;  \
	MenuItem Name = {(void*)&Next, (void*)&Previous, (void*)&Parent, (void*)&Child,  /*(funcPtr)EnterFunc,*/(callPtr)Callback/*,(funcPtr)LeaveFunc*/}

/* Exported define -----------------------------------------------------------*/
#define NULL_MENU 	Null_Menu
#define NULL_FUNC  (void*)0

#define PREVIOUS   ( CurrWorkItem->Previous )
#define NEXT       ( CurrWorkItem->Next )
#define PARENT     ( CurrWorkItem->Parent )
#define CHILD      ( CurrWorkItem->Child )
#define ENTERFUNC  ( CurrWorkItem->EnterFunc )
//#define SELECTFUNC ( CurrWorkItem->SelectFunc )

/* Exported functions ------------------------------------------------------- */
void MenuChange(MenuItem*);
void SetStartScreen(void);
void MenuChangeMain(void);
void UIShortButton(uint16_t state);
void UILongButton(uint16_t state);
void MenuCallback(void);

#endif //MENU_H
