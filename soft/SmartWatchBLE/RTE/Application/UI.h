/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef UI_H
#define UI_H

/* Includes ------------------------------------------------------------------*/
#include "nrf.h"
#include <stdbool.h>

/* Exported types ------------------------------------------------------------*/


/* Exported constants --------------------------------------------------------*/
enum
{
	LOG_NO, LOG_INFO, LOG_WARN, LOG_ERR
};

/* Exported macro ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
uint8_t DrawInfoPanel(uint8_t pos);

uint8_t MainScreen(uint16_t);
uint8_t BinarScreen(uint16_t);
uint8_t TextScreen1(uint16_t opCode);
uint8_t TextScreen2(uint16_t opCode);
uint8_t ExtendScreen(uint16_t);

uint8_t RandomCb(uint16_t opCode);

uint8_t menuScreen(uint16_t);
uint8_t SettScreen(uint16_t);
uint8_t TDScreen(uint16_t);
uint8_t SetAlarm(uint16_t);
uint8_t BIGScreen(uint16_t);
uint8_t CalendarDraw(uint16_t);
uint8_t FlashLight(uint16_t);
uint8_t StopWatchCb(uint16_t code);
uint8_t TimerCb(uint16_t code);

uint8_t AllDataDisplayCb(uint16_t);

void AlarmRing(void);
void AlarmUpdate(void);

uint8_t LogScreen(uint16_t state);
uint8_t LogAddInfo(char * s, int WarnLvl, uint8_t volume);

uint8_t DrawSleepLog(uint16_t state);

uint16_t TimeDiff(uint8_t, uint8_t, uint8_t, uint8_t, uint16_t);

#endif //UI_H
