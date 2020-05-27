/* Includes ------------------------------------------------------------------*/
#include "alarm.h"
#include "dispatcher.h"
#include "shadow_logic.h"


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
AlarmType Alarmes[NUM_ALARM] = {{ 7, 50, 0x1F, true } , { 9, 0, 0x60, true }};

dispID alarmEntity, updAlarmEntity;
dispID alarmRingEntity;

uint32_t alarmNearestTimestamp;


/* Extern variables ----------------------------------------------------------*/
extern uint32_t RTCCounter;


/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  : 
* Description    : 
*******************************************************************************/
void AlarmUpdate()
{
	uint32_t timeStamp = 0xFFFFFFFF;

	for(uint8_t k = 0; k < NUM_ALARM; k++)
	{
		if(Alarmes[k].active == true && Alarmes[k].day > 0)
		{
			for(uint8_t i = 0; i < 7; i++)
			{
				if(BitIsSet(Alarmes[k].day, i))
				{
					RTC_TimeDateTypeDef RTCAlarm;
					RTC_CntToTimeDate(RTCCounter, &RTCAlarm);

					RTCAlarm.hour = Alarmes[k].hour;
					RTCAlarm.minute = Alarmes[k].minute;
					RTCAlarm.second = 0;
					uint32_t diff = RTC_TimeDateToCnt(&RTCAlarm);

					if(i > RTCAlarm.weekday)
						diff += 24*3600 * (i - RTCAlarm.weekday);
					else if(i < RTCAlarm.weekday)
						diff += 24*3600 * (7 - RTCAlarm.weekday + i);

					if(diff > RTCCounter && diff < timeStamp)
						timeStamp = diff;
				}
			}
		}
	}

	if(timeStamp != 0xFFFFFFFF)
	{
		alarmEntity = DispatcherAddEvnt(DISP_TYPE_SINGLE_TIMESTAMP, timeStamp, AlarmRing, 0);
		//updAlarmEntity = DispatcherAddEvnt(DISP_TYPE_SINGLE_TIMESTAMP, timeStamp+1, AlarmUpdate, 0); //update alarmes after alarm ring
	}

	alarmNearestTimestamp = timeStamp;
}

void AlarmRing()
{
	//static uint8_t phase = 0;
	SoundBeep(400, 250);
	
	Vibrate(3*DISP_100ms);

//	const uint8_t buttonsNum[5] =
//	{ BTN_0, BTN_1, BTN_2, BTN_3, BTN_4 };

	alarmRingEntity = DispatcherAddEvnt(DISP_TYPE_SINGLE, DISP_1s, AlarmRing, 0);
}

void AlarmRing2()
{
	return;
}


void AlarmDisable()
{
	DispatcherRemoveEvnt(alarmEntity);
	DispatcherRemoveEvnt(updAlarmEntity);
}


extern Disp_Typedef DispMass[20 + 1];
void DispDebugAlarm()
{
	SSD1306_NumWDesc(0,1,"ARE=",alarmRingEntity);
	SSD1306_NumWDesc(0,10,"uAE=",updAlarmEntity);
	SSD1306_NumWDesc(0,20,"ARE#=",DispMass[alarmRingEntity].mode);
	SSD1306_NumWDesc(0,30,"uAE#=",DispMass[updAlarmEntity].mode);
	
	SSD1306_NumWDesc(0,40,"free=",DispatcherGetFreeSpace());
}
