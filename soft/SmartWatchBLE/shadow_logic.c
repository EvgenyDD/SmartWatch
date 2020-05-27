/* Includes ------------------------------------------------------------------*/
#include "shadow_logic.h"
#include "LSM303.h"
#include "UI.h"
#include "hardware.h"
#include "nrf_gpio.h"
#include "dispatcher.h"
#include "alarm.h"
#include "string.h"


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
volatile uint32_t RTCCounter;
uint32_t TimerCounter = 0;

volatile int16_t countDown = 10*DISP_1s;
volatile uint16_t vibroCounter = 0;

//default offset 116sec/week = 16.571sec/day
int32_t correctionValue = -5213; //+1 sec/day  = 86400/sec
int32_t correctionCoundown;

bool stopWatchEnabled = false;
uint32_t stopWatchCounter = 0;

/* Extern variables ----------------------------------------------------------*/
 extern uint32_t UpTimeCounter, DownTimeCounter;
 extern AlarmType Alarmes[10];
 extern char SleepLog[];
 
 extern uint32_t _L2_stepCounter, _L1_stepTime, TotalStepCounter, TotalStepTime;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  : Count100ms
* Description    : 
*******************************************************************************/
void Count100ms(void)
{
	if(countDown >= 0) countDown--;
		
	if(stopWatchEnabled == true)
		stopWatchCounter++;
	
	if(vibroCounter)
	{
		vibroCounter--;
		if(vibroCounter == 0)
			nrf_gpio_pin_clear(VIBRO);
	}
}


/*******************************************************************************
* Function Name  : UpdateSwitchModes
* Description    : 
*******************************************************************************/
void UpdateSwitchModes(uint8_t mode)
{
	/* CHeck if MIDNIGHT come -> turn on Sleep Tracker Accelerometer mode */
	if( (RTCCounter % (3600 * 24) == 0 		&& mode == 0) || 
		(RTCCounter % (3600 * 24) < 3600*8 	&& mode == 1) )
	{
		LSM303_Init_SleepTracker();
		ClearBuffer((uint8_t*)SleepLog, 600, 0);
		LogAddInfo("A>sleep", LOG_INFO, 20);
	}

	/* Check if the MAN - AWOKE */ //experimental - current at 8 A.M.
	/* -> turn on Gestures Tracker Accelerometer mode */
	if( (RTCCounter % (3600 * 24) == 3600 * 8 && mode == 0) || 
		(RTCCounter % (3600 * 24) >= 3600 * 8 && mode == 1))
	{
		LSM303_Init_ShadowGesture();
		LogAddInfo("A>gest", LOG_INFO, 40);
	}
}


/*******************************************************************************
* Function Name  : Count1s
* Description    : 
*******************************************************************************/
void Count1s(void)
{
	RTCCounter++;
	
	// Hour events
	if(RTCCounter % 3600 == 0)
	{
		uint8_t hour = (RTCCounter % (24 * 3600)) / 3600;
		
		HourBuzzCallback(hour);
		
		UpdateSwitchModes(0);
		
		if(hour == 0)
		{
			TotalStepTime = 0;
			TotalStepCounter = 0;
		}
	}

	if(TimerCounter) //process timer application
	{
		TimerCounter--;
		if(TimerCounter == 0)
			DispatcherAddEvnt(DISP_TYPE_SINGLE, 1, AlarmRing, 0);
	}
	
//	RTC_TimeDateTypeDef temp;
//	RTC_CntToTimeDate(RTCCounter, &temp);
	
	static uint8_t dddjdjdjdjjd = 0;
	if (++dddjdjdjdjjd >= 2)
	{
		dddjdjdjdjjd = 0;
		DispatcherAddEvnt(DISP_TYPE_SINGLE, DISP_1s, AlarmRing2, 0);
	}

	(countDown == -1) ? DownTimeCounter++ : UpTimeCounter++;
}

/*******************************************************************************
* Function Name  : Count1s
* Description    : Make Hour Buzz
*******************************************************************************/
void HourBuzzDo(void)
{	
	/* Buzz 1-3 time */
	static uint8_t phase = 0;
	if(phase >= 2) phase = 0;
	
	if(phase == 0)
		nrf_gpio_pin_set(VIBRO);
	else
		nrf_gpio_pin_clear(VIBRO);
	
	phase++;
}

/*******************************************************************************
* Function Name  : HourBuzzCallback
* Description    : Create task to make Hour Buzz
*******************************************************************************/
void HourBuzzCallback(uint8_t hour)
{
	if(hour < 8) return;
	
	DispatcherAddEvnt(	DISP_TYPE_CYCLED, 
						((hour + 2) % 3 + 1) == 1 ? 2*DISP_100ms : 1*DISP_100ms, 
						HourBuzzDo, 
						((hour + 2) % 3 + 1)*2);
}

/*******************************************************************************
* Function Name  : UpdateCorrectionCounter
* Description    : 
*******************************************************************************/
void UpdateCorrectionCounter(void)
{
	correctionCoundown = correctionValue;
}

/*******************************************************************************
* Function Name  : Vibrate
* Description    : 
*******************************************************************************/
void Vibrate(uint16_t delay)
{
	nrf_gpio_pin_set(VIBRO);
	vibroCounter = delay;
}
