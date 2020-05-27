/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _ALARM_H
#define _ALARM_H

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
#include "periph.h"


/* Exported types ------------------------------------------------------------*/
typedef struct
{
	uint8_t hour;
	uint8_t minute;
	uint8_t day;
	bool active;
} AlarmType;

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
#define NUM_ALARM 10


/* Exported functions ------------------------------------------------------- */
void AlarmUpdate(void);
void AlarmRing(void);
void AlarmDisable(void);

void AlarmRing2(void);

#endif //_ALARM_H
