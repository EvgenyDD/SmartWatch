/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef UI_H
#define UI_H

/* Includes ------------------------------------------------------------------*/
#include "nrf.h"
#include "stdbool.h"


typedef struct{
    uint8_t hour;
    uint8_t minute;
    uint8_t day;
    bool active;
}AlarmType;


/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void MainScreen(uint16_t);
void BinarScreen(uint16_t);
void ExtendScreen(uint16_t);
void menuScreen(uint16_t);
void SettScreen(uint16_t);
void TDScreen(uint16_t);
void SetAlarm(uint16_t code);

#endif //UI_H
