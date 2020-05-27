/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef SHADOW_LOGIC_H
#define SHADOW_LOGIC_H

/* Includes ------------------------------------------------------------------*/
#include "dispatcher.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void Count100ms(void);
void Count1s(void);

void HourBuzzDo(void);
void HourBuzzCallback(uint8_t);

void UpdateCorrectionCounter(void);
void UpdateSwitchModes(uint8_t mode);

void Vibrate(uint16_t delay);

#endif //SHADOW_LOGIC_H
