/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef APP_DISPATCHER_H
#define APP_DISPATCHER_H

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/* Exported types ------------------------------------------------------------*/
enum
{
	DISP_TYPE_NULL = 0, DISP_TYPE_REPEATED, DISP_TYPE_SINGLE, DISP_TYPE_CYCLED, DISP_TYPE_SINGLE_TIMESTAMP
};
typedef unsigned char dispID;
typedef void (*CbPtr)(void);

typedef struct
{
	uint8_t mode;
	uint8_t OnOff;

	uint32_t delayCounter;
	uint32_t triggerTimeStamp;
	uint8_t cycles;
	CbPtr pCallback;
} Disp_Typedef;



/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
#define DISP_100ms	(1)
#define DISP_1s		(10)
#define DISP_1m		(600)
#define DISP_1Hr	(DISP_1s*3600)

/* Exported functions ------------------------------------------------------- */
void DispatcherInit(void);

void DispatcherCallback(void);
void DispatcherEnable(void);
void DispatcherDisable(void);

dispID DispatcherAddEvnt(uint8_t mode, uint32_t time, CbPtr X, uint16_t cycles);
void DispatcherRemoveEvnt(dispID X);

void DispatcherSyspendEvnt(dispID X);
void DispatcherResumeEvnt(dispID X);

uint16_t DispatcherGetFreeSpace(void);

#endif //APP_DISPATCHER_H
