/* Includes ------------------------------------------------------------------*/
#include "dispatcher.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define DISP_MAX_ENTRIES	20

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
volatile uint32_t SysTickTimer;
uint32_t UpTimeCounter;
uint32_t DownTimeCounter;

/*static*/ Disp_Typedef DispMass[DISP_MAX_ENTRIES + 1];
static uint8_t dispatcherEnabled;


/* Extern variables ----------------------------------------------------------*/
extern volatile uint32_t SysTickTimer, RTCCounter;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
void DispatcherInit(void)
{
	for(uint8_t i = 1; i <= DISP_MAX_ENTRIES; i++)
	{
		DispMass[i].mode = DISP_TYPE_NULL;
		DispMass[i].OnOff = 0;
	}
	dispatcherEnabled = 1;
}
extern uint8_t xcv;
/*******************************************************************************
 * Function Name  : 
 * Description    : 
 *******************************************************************************/
void DispatcherCallback(void)
{
	// main system 100ms clock
	SysTickTimer++; 
	
	if(dispatcherEnabled == 0)
		return;

	for(uint8_t i = 1; i <= DISP_MAX_ENTRIES; i++)
	{
		if((DispMass[i].mode != DISP_TYPE_NULL) && (DispMass[i].OnOff > 0))
		{
			if(DispMass[i].mode == DISP_TYPE_SINGLE)
			{
				if(SysTickTimer >= DispMass[i].triggerTimeStamp)
				{
					DispatcherRemoveEvnt(i);
					DispMass[i].pCallback();
					continue;
				}
			}
			else if(DispMass[i].mode == DISP_TYPE_REPEATED)
			{
				if(SysTickTimer >= DispMass[i].triggerTimeStamp)
				{
					DispMass[i].triggerTimeStamp = SysTickTimer + DispMass[i].delayCounter;
					DispMass[i].pCallback();
					continue;
				}
			}
			else if(DispMass[i].mode == DISP_TYPE_CYCLED)
			{
				if(SysTickTimer >= DispMass[i].triggerTimeStamp)
				{
					if(--DispMass[i].cycles > 0)
						DispMass[i].triggerTimeStamp = SysTickTimer + DispMass[i].delayCounter;
					else
						DispatcherRemoveEvnt(i);
					DispMass[i].pCallback();
					continue;
				}
			}
			else if(DispMass[i].mode == DISP_TYPE_SINGLE_TIMESTAMP)
			{
				if(RTCCounter >= DispMass[i].triggerTimeStamp)
				{
					DispatcherRemoveEvnt(i);
					DispMass[i].pCallback();
					continue;
				}
			}
		}
	}
}

void DispatcherEnable(void){
	dispatcherEnabled = 1;
}

void DispatcherDisable(void){
	dispatcherEnabled = 0;
}

dispID DispatcherAddEvnt(uint8_t mode, uint32_t time, CbPtr X, uint16_t cycles)
{
	uint8_t ID = 0;

	for(uint8_t i = 1; i <= DISP_MAX_ENTRIES; i++)
		if(DispMass[i].mode == DISP_TYPE_NULL)
		{
			DispMass[i].mode = mode;
			DispMass[i].pCallback = (CbPtr) X;
			DispMass[i].OnOff = 1;
			ID = i;

			DispMass[i].triggerTimeStamp = SysTickTimer + time;

			if(mode == DISP_TYPE_SINGLE)
				DispMass[i].delayCounter = 0;
			else if(mode == DISP_TYPE_REPEATED)
				DispMass[i].delayCounter = time;
			else if(mode == DISP_TYPE_CYCLED)
			{
				DispMass[i].delayCounter = time;
				DispMass[i].cycles = cycles;
			}
			else if(mode == DISP_TYPE_SINGLE_TIMESTAMP)
				DispMass[i].triggerTimeStamp = time;
			
			return ID;
		}
		
	return ID;
}

void DispatcherRemoveEvnt(dispID X)
{
	if(X == 0 || X > DISP_MAX_ENTRIES) return;
	DispMass[X].mode = DISP_TYPE_NULL;
}

void DispatcherSyspendEvnt(dispID X)
{
	if(X == 0 || X > DISP_MAX_ENTRIES) return;
	DispMass[X].OnOff = 0;
}

void DispatcherResumeEvnt(dispID X)
{
	if(X == 0 || X > DISP_MAX_ENTRIES) return;
	DispMass[X].OnOff = 1;
}


uint16_t DispatcherGetFreeSpace()
{
	uint16_t num = 0;
	
	for(uint8_t i = 1; i <= DISP_MAX_ENTRIES; i++)
		if(DispMass[i].mode == DISP_TYPE_NULL)
			num++;
		
	return num;
}
