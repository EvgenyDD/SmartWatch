/* Includes ------------------------------------------------------------------*/
#include "UI.h"
#include "nrf.h"
#include "string.h"
#include "SSD1306.h"
#include "light.h"
#include "timework.h"
#include "hardware.h"
#include "menu.h"
#include "LSM303.h"
#include "BMP180.h"
#include "twi_master.h"

#include "ble_app.h"
#include "dispatcher.h"
#include "shadow_logic.h"

#include "nrf_drv_rng.h"

#include "alarm.h"

//#define TimeDiff(AlarmH, AlarmM, TimeH, TimeM) (((AlarmH*60+AlarmM)-(TimeH*60+TimeM))>0 && ((AlarmH*60+AlarmM)-(TimeH*60+TimeM))<500)?((AlarmH*60+AlarmM)-(TimeH*60+TimeM)):0
char WeekDays[] = "\x8f\x8e\x8d\x82\x92\x90\x91\x90\x84\x97\x92\x82\x8f\x92\x8d\x91\x93\x81\x82\x91\x8a\x95\x93\x89"; /**/

#define LOG_NUM_OF_FIELDS (9)
#define LOG_NUM_CHARS (4+8+4+1)     //in one field
char LogBuffer[LOG_NUM_CHARS * LOG_NUM_OF_FIELDS];
char SleepLog[10 * 60];
uint16_t LogNumOfFields = 0;
uint16_t LogFirstField = 0;



//dispID AlarmRinger;

enum
{
	LOG_Null, LOG_Info, LOG_Warn, LOG_Err
} LOG_MSG_TYPES;

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Extern variables ----------------------------------------------------------*/
extern uint32_t RTCCounter;
extern char monthMass[];
extern volatile int16_t countDown;

extern bool BluetoothEnabled;

extern MenuItem* CurrWorkItem;
extern MenuItem Game;

/* System Tick Timer Variable */
extern uint32_t SysTickTimer;

extern int32_t correctionCoundown;
extern int32_t correctionValue;

/* Image resources */
extern const char menu_alarm[];
extern const char menu_stopwatch[];
extern const char menu_timedate[];
extern const char menu_torch[];
extern const char menu_rand[];
extern const char menu_games[];
extern const char menu_settings[];
extern const char menu_txt[];

extern const char BTicon[];
extern const char AlarmIcon[];
extern const char SoundFullIcon[];
extern const char SoundSilentIcon[];

extern MenuItem Settings;
extern MenuItem Random;
extern MenuItem Calendar;
extern MenuItem Alarm;
extern MenuItem Text;
extern MenuItem Game;
extern MenuItem Flashlight;
extern MenuItem Stopwatch;
extern MenuItem SetTime;
extern MenuItem TimerMode;

extern volatile uint8_t buttonCounter;
extern volatile uint16_t buttonState;

extern uint32_t UpTimeCounter;
extern uint32_t DownTimeCounter;

extern uint8_t orient;

extern bool stopWatchEnabled;
extern uint32_t stopWatchCounter;
extern uint32_t TimerCounter;

extern uint16_t failACnt;

extern uint32_t alarmNearestTimestamp;
extern AlarmType Alarmes[NUM_ALARM];

extern uint8_t sleepVibrator;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
 * Function Name  : TimeDiff
 * Description    : Return time difference between two timestamps
 *******************************************************************************/
//uint16_t TimeDiff(uint8_t AlarmH, uint8_t AlarmM, uint8_t TimeH, uint8_t TimeM, uint16_t maxDiff)
//{
//	int16_t trrr = (AlarmH * 60 + AlarmM) - (TimeH * 60 + TimeM);
//	if(trrr < maxDiff && trrr > 0)
//		return trrr;
//	trrr = (24 * 60 - (TimeH * 60 + TimeM)) + (AlarmH * 60 + AlarmM);
//	if(trrr < maxDiff && trrr > 0)
//		return trrr;
//	return 0;
//}

/*******************************************************************************
 * Function Name  : DrawInfoPanel
 * Description    :
 *******************************************************************************/
uint8_t DrawInfoPanel(uint8_t pos)
{
	/* Alarm countdown */
	if((alarmNearestTimestamp - RTCCounter) < 3600 * 10)
	{
		char tm[6] =
		{ '\0' };
		tm[0] = (alarmNearestTimestamp - RTCCounter) / 3600 / 10 + '0';
		tm[1] = (alarmNearestTimestamp - RTCCounter) / 3600 % 10 + '0';
		tm[2] = ':';
		tm[3] = (alarmNearestTimestamp - RTCCounter) / 60 % 60 / 10 + '0';
		tm[4] = (alarmNearestTimestamp - RTCCounter) / 60 % 60 % 10 + '0';
		SSD1306_setFont (font3x5);
		SSD1306_String(0, pos + 2, tm);
	}
	else if(alarmNearestTimestamp != 0xFFFFFFFF)
		SSD1306_Bitmap(0, pos, (char*) AlarmIcon);

	SSD1306_Bitmap(22, pos, (char*) SoundFullIcon);

	if(BluetoothEnabled)
	{
#define BT_ICON_POS 32
		SSD1306_Bitmap(BT_ICON_POS, pos, (char*) BTicon);
		drawFastHLine(BT_ICON_POS + 3, pos - 1, 2, WHITE); //this is very weird shit!!!!!!!!!!!!
	}

	// Draw VBat simbol & voltage
	MeasureVBat();
	SSD1306_setFont (font3x5);
	SSD1306_Float(46, pos + 1, (float) (VBat.result * 14.2134 + 34.024/*71/5+34*/) / 1000, 2);
	//SSD1306_Float(0, 123, (float)(1.57927*(VBat.result-229.781)), 1);
	//SSD1306_Num(0,123,(1.57927*(VBat.result-229.781)));

	SSD1306_Rect(44, pos + 7, 44 + 18, pos - 1);
	SSD1306_Rect(63, pos + 5, 63 + 1, pos + 1);

	drawFastHLine(0, pos - 2 - 1, 64, WHITE);

	return 1;
}


static uint8_t SubStopWatch(uint16_t code)
{
	SSD1306_setFont(font5x8);
	
	//DispDebugAlarm();
	
	char t[12];
	if(stopWatchCounter / 864000 != 0)
	{
		t[0] = (stopWatchCounter / 864000 / 10 != 0) ? ('0' + (char) (stopWatchCounter / 864000 / 10)) : ' '; //d
		t[1] = '0' + (char) (stopWatchCounter / 864000 % 10); //d
		t[2] = ' '; //:
		t[3] = 'd';
		SSD1306_BigString(1, 29, t, 1);
	}

	if(stopWatchCounter / 3600 != 0)
	{
		t[0] = (stopWatchCounter / 36000 % 24 / 10 != 0) ? ('0' + (char) (TimerCounter / 36000 % 24 / 10)) : ' '; //h
		t[1] = '0' + (char) (stopWatchCounter / 36000 % 24 % 10); //h
		t[2] = ' '; //:
		t[3] = 'h';
		SSD1306_BigString(1, 15, t, 1);
	}

	t[0] = (stopWatchCounter / 600 % 60 / 10) ? ('0' + (char) (stopWatchCounter / 600 % 60 / 10)) : ' '; //m
	t[1] = '0' + (char) (stopWatchCounter / 600 % 10); //m
	t[2] = ':'; //:
	t[3] = '0' + (char) (stopWatchCounter / 10 % 60 / 10); //s
	t[4] = '0' + (char) (stopWatchCounter / 10 % 10); //s
	t[5] = '.';
	t[6] = '0' + (char) (stopWatchCounter % 10); //ms
	t[7] = '\0'; 
	SSD1306_BigString(1, 1, t, 1);

	
	
	return 1;
}

uint8_t bitch;
static uint8_t SubPedometer(uint16_t code)
{
		SSD1306_setFont (font5x8);
	
extern uint32_t _L2_stepCounter, _L1_stepTime, TotalStepCounter, TotalStepTime;
	
	//SSD1306_Num(0,0,_L2_stepCounter);
	//SSD1306_Num(0,8,stepTotal);
	float x = 10* ((float)TotalStepCounter) / ((float)TotalStepTime);
	
extern uint16_t stepCountdown;
extern uint8_t stepCounterActive;
	
	SSD1306_Num(2,1,/*x*60*/_L2_stepCounter);
	SSD1306_Num(2,8,_L1_stepTime);
	SSD1306_Num(2,14,TotalStepCounter);
	SSD1306_Num(2,20,bitch);
	
	
	float y = x*0.176;
	y+= 0.574;
	y = y*TotalStepCounter;
	
	SSD1306_FloatWDesc(2, 60, "Tdy:", y/1000, 3);
	
	return 1;
}

static uint8_t SubTimer(uint16_t code)
{
	SSD1306_setFont(font5x8);
	
	char t[6] =
	{ '\0' };

	if(TimerCounter / 86400 != 0)
	{
		t[0] = (TimerCounter / 86400 / 10 != 0) ? ('0' + (char) (TimerCounter / 86400 / 10)) : ' '; //d
		t[1] = '0' + (char) (TimerCounter / 86400 % 10); //d
		t[2] = ' '; //:
		t[3] = 'd';
		SSD1306_BigString(1, 29, t, 1);
	}

	if(TimerCounter / 3600 != 0)
	{
		t[0] = (TimerCounter / 3600 % 24 / 10 != 0) ? ('0' + (char) (TimerCounter / 3600 % 24 / 10)) : ' '; //h
		t[1] = '0' + (char) (TimerCounter / 3600 % 24 % 10); //h
		t[2] = ' '; //:
		t[3] = 'h';
		SSD1306_BigString(1, 15, t, 1);
	}

	t[4] = '0' + (char) (TimerCounter % 10); //s
	t[3] = '0' + (char) (TimerCounter % 60 / 10); //s
	t[2] = ':'; //:
	t[1] = '0' + (char) (TimerCounter / 60 % 10); //m
	t[0] = '0' + (char) (TimerCounter / 60 % 60 / 10); //m
	SSD1306_BigString(1, 1, t, 1);
	
	return 1;
}

static uint8_t SubAccelGraph(uint16_t code)
{
	SSD1306_setFont(font5x8);
	
	if(code == START_COM)
		countDown = 5*DISP_1s;
	
	int ACCEL[3];
	LSM303_GetAccelData(ACCEL);
	
	static uint8_t f=1;
	if(++f>=63)
		f = 0;
	
	SSD1306_Pixel(f, 10+ACCEL[XAxis]/110, WHITE);
	SSD1306_Pixel(f, 30+ACCEL[YAxis]/110, WHITE);
	SSD1306_Pixel(f, 50+ACCEL[ZAxis]/110, WHITE);
	
	if(f == 0)
	{
		f = 1;
		return 1;
	}
	
	return 2;
}

static uint8_t SubAltimeter(uint16_t code)
{
	SSD1306_setFont(font5x8);
	BMP180_Init();
	float xx;
	getPressure(&xx);
	SSD1306_FloatWDesc(2, 1, "P:", xx, 1);
	SSD1306_FloatWDesc(2, 11, "Pmm:", xx*0.0075, 1);

	SSD1306_FloatWDesc(2, 31, "Mtr:", pressureToAltitude(100000, xx), 1);
	return 1;
}
extern int ACCEL[3], MAGNET[3];


static uint8_t SubSleepGraph(uint16_t code)
{
	SSD1306_setFont(font5x8);
	
	static int16_t tiltOffset = 0;
	
	LSM303_GetAccelData(ACCEL);
	#define tilt (ACCEL[YAxis] - tiltOffset)
	
	switch (code)
	{
	case START_COM:
		countDown = 15*DISP_1s;
		tiltOffset = ACCEL[YAxis];
		break;
	case BTN_UP:
	case BTN_DOWN:
		countDown = 10*DISP_1s;
		break;
	case BTN_LEFT:
	case BTN_RIGHT:
		break;
//	case BTN_MENU:
//		break;
	}
	
	static int16_t ptrSleepData = 300;
	
	if(tilt > 300)
		ptrSleepData += (tilt-200)/40;
	else if(tilt < -300)
		ptrSleepData += (tilt+200)/40;
	
	if(ptrSleepData < 0)
		ptrSleepData = 0;
	else if(ptrSleepData >= 600-61)
		ptrSleepData = 600-61;
	//SSD1306_Num(2,2, ptrSleepData);
	
	//y 68 max 
	
	SSD1306_setFont(font3x5);
	for(uint16_t i=1, k=0; i<63; i++, k++)
	{
		if(((ptrSleepData + k) % 10) == 0)
		{
			SSD1306_VLine(i, 8, 3, WHITE);			
		}
		
		SSD1306_VLine(i, 15+1, SleepLog[ptrSleepData + k], WHITE);			
		
		
		if(((ptrSleepData + k) % 60) == 0)
		{
			SSD1306_VLine(i, 8, 5, WHITE);
			SSD1306_Num(i,2,(ptrSleepData + k)/60);
		}
		else if(((ptrSleepData + k) % 30) == 0)
		{
			SSD1306_Num(i,2,30);		
		}
	}
	
	return 1;
}

static uint8_t SubWeatherPresr(uint16_t code)
{
	SSD1306_setFont(font5x8);
	BMP180_Init();
	float xx;
	getPressure(&xx);
	SSD1306_FloatWDesc(2, 1, "P:", xx, 1);
	SSD1306_FloatWDesc(2, 11, "Pmm:", xx*0.0075, 1);
	
	return 1;
}

extern const unsigned char fontSTD_mykefont2[];

/*******************************************************************************
 * Function Name  : MainScreen
 * Description    :
 *******************************************************************************/
uint8_t MainScreen(uint16_t code)
{
	static uint8_t state = 1;
	
	if(state==1) 
		SSD1306_Clear();	


	SSD1306_setFont(font5x8);
	char t[15];
	RTC_TimeDateTypeDef aaa;
	RTC_CntToTimeDate(RTCCounter, &aaa);
	TimeToString(t, &aaa);
	

	char temp[3] = { '\0' };
	temp[0] = t[0];
	temp[1] = t[1];
	SSD1306_BigString(0, 98, temp, 2);
	temp[0] = t[3];
	temp[1] = t[4];
	SSD1306_BigString(24, 98, temp, 2);
	temp[0] = t[6];
	temp[1] = t[7];
	
	SSD1306_setFont ((uint8_t *)fontSTD_mykefont2);
	SSD1306_BigString(48, 100, temp, 1);

	SSD1306_setFont (font5x8);
	
	char date[7] = { '\0' };

	for(uint8_t i = 0; i < 3; i++)
		date[i] = monthMass[aaa.month * 3 + i];

	date[3] = ' ';
	date[4] = aaa.date / 10 + '0';
	date[5] = aaa.date % 10 + '0';

	SSD1306_StringCentered(0, 90, date, 1);
	
	SSD1306_Num(5, 80, aaa.year);

	char tmp[4] = { '\0' };
	for(uint8_t x = 0, i = aaa.weekday * 3; i < aaa.weekday * 3 + 3; i++, x++)
		tmp[x] = WeekDays[i];
	SSD1306_String(40, 80, tmp);

	MeasureLight();
	SSD1306_Brightness(Light.result / 3);

	//SSD1306_NumWDesc(0, 0, "L=", Light.result);
	//SSD1306_Num(48, 0, j);

	if(code == BTN_RIGHT_LONG)
	{
		if(BluetoothEnabled == false)
		{
			advertising_start();
			BluetoothEnabled = true;
		}
		else
		{
			BLE_Stop();
			BluetoothEnabled = false;
		}
	}
		#define SUB_SCRN_NUM	7
	static uint8_t ptrSubScreen=0;
	if(code == BTN_RIGHT)
	{
		if(++ptrSubScreen >= SUB_SCRN_NUM)
			ptrSubScreen = 0;
		code = START_COM;
	}
	if(code == BTN_LEFT)
	{
		if(--ptrSubScreen >= SUB_SCRN_NUM)
			ptrSubScreen = SUB_SCRN_NUM-1;
		code = START_COM;
	}
		
	if(BTN_ANY(code))
		countDown = 5*DISP_1s;			

	typedef uint8_t (*SubScrnType)(uint16_t);
	const SubScrnType SubScrn[] =
	{	
		SubPedometer,
		SubStopWatch, 	
		SubTimer, 				
		SubAccelGraph, 	
		SubAltimeter,  				
		SubSleepGraph,
		SubWeatherPresr
	}; 
	
	const char subScrnName[] = "PedStWTmrAccAltSlpPrsssssssssssssssssssssssssssss";
	for(uint8_t i = 0; i < 3; i++)
		date[i] = subScrnName[ptrSubScreen * 3 + i];
	
	date[3] = '\0';
	
#define CCC (63-14)							
	SSD1306_setFont (font3x5);								
	SSD1306_HLine(0, 70, ptrSubScreen*CCC/(SUB_SCRN_NUM-1), WHITE); 
	SSD1306_HLine(14+ptrSubScreen*CCC/(SUB_SCRN_NUM-1), 70, 63-14-ptrSubScreen*CCC/SUB_SCRN_NUM, WHITE);
	SSD1306_HLine(ptrSubScreen*CCC/(SUB_SCRN_NUM-1), 78, 14, WHITE); 
	SSD1306_String(ptrSubScreen*CCC/(SUB_SCRN_NUM-1)+2,72,date);
	SSD1306_VLine(ptrSubScreen*CCC/(SUB_SCRN_NUM-1),70,8,WHITE);
	SSD1306_VLine(ptrSubScreen*CCC/(SUB_SCRN_NUM-1)+14,70,8,WHITE);
	
	for(uint8_t i=0; i<SUB_SCRN_NUM; i++)
	{
		if(i == ptrSubScreen) continue;
		SSD1306_Pixel(i*63/SUB_SCRN_NUM+7, 74, WHITE);
	}
	
	SSD1306_VLine(0, 0, 70, WHITE);
	SSD1306_VLine(63, 0, 70, WHITE);
	SSD1306_HLine(0,0,63, WHITE);
	
	state = SubScrn[ptrSubScreen](code);

	/* Dembel Time */
	/*RTC_TimeDateTypeDef Start={2015,07,06, 10,30,0, 1}, End={2015,07,31, 8,0,0, 5};
	 float percent = 100* (float)(RTCCounter-RTC_TimeDateToCnt(&Start)) / (float)(RTC_TimeDateToCnt(&End)-RTC_TimeDateToCnt(&Start));

	 char d[25];
	 ftoa_(percent, d, 3);
	 strcat_(d, "%");
	 SSD1306_String(0, 8, d);
	 if((RTC_TimeDateToCnt(&End)-RTCCounter)/3600 < 600)
	 SSD1306_Num(0,64, (RTC_TimeDateToCnt(&End)-RTCCounter)/3600);*/

	DrawInfoPanel(120);

	return state;
}

extern const unsigned char fontSTD_numSevenSegNumFont[];
extern const unsigned char fontSTD_numCalibriBold[];
extern const unsigned char fontSTD_numDotMatrix_XL_Num[];
extern const unsigned char fontSTD_numArialNumFontPlus[];
extern const unsigned char fontSTD_numSevenSegNumFontPlusPlus[];

/*******************************************************************************
 * Function Name  : BIGScreen
 * Description    :
 *******************************************************************************/
uint8_t BIGScreen(uint16_t code)
{
	static uint8_t ptr = 0;

	const uint8_t *FontMass[] =
	{ fontSTD_numSevenSegNumFont, fontSTD_numCalibriBold, fontSTD_numDotMatrix_XL_Num, fontSTD_numArialNumFontPlus //,
		//fontSTD_numSevenSegNumFontPlusPlus
		};

	switch (code)
	{
	case START_COM:
		countDown = 5*DISP_1s;
		break;
	case STOP_COM:
		countDown = 3*DISP_1s;
		break;
	case BTN_UP:
	case BTN_RIGHT:
		if(++ptr > (sizeof(FontMass) / 4 - 1))
			ptr = 0;
		SSD1306_setFont((uint8_t*) FontMass[ptr]);
		break;
	case BTN_LEFT:
	case BTN_DOWN:
		if(--ptr > (sizeof(FontMass) / 4 - 1))
			ptr = sizeof(FontMass) / 4 - 1;
		SSD1306_setFont((uint8_t*) FontMass[ptr]);
		break;
//	case BTN_MENU:
//		break;
	}
	SSD1306_setFont((uint8_t*) FontMass[ptr]);

	char t[15];
	RTC_TimeDateTypeDef aaa;
	RTC_CntToTimeDate(RTCCounter, &aaa);
	TimeToString(t, &aaa);

	char temp[3] =
	{ '\0' };
	
	temp[0] = t[0];
	temp[1] = t[1];
	SSD1306_StringCentered(0, 75, temp,1);
	temp[0] = t[3];
	temp[1] = t[4];
	SSD1306_StringCentered(0, 15, temp,1);
	
	SSD1306_setFont (font5x8);
	char date[11] = "  /  /    ";
	date[0] = aaa.date/10%10+'0';
	date[1] = aaa.date%10+'0';
	
	date[3] = aaa.month/10%10+'0';
	date[4] = aaa.month%10+'0';
	
	date[6] = '2';
	date[7] = '0';
	date[8] = aaa.year/10%10+'0';
	date[9] = aaa.year%10+'0';

	SSD1306_StringCentered(0, 0, date,1);
	
	SSD1306_VLine(0, 10, aaa.second>=30?30*3:aaa.second*3, WHITE);
	SSD1306_VLine(63, 10, aaa.second<=30?0:(aaa.second-30)*3, WHITE);
	
	if(aaa.second>=10)
		SSD1306_HLine(0, 10+10*3, 2, WHITE);
	if(aaa.second>=20)
		SSD1306_HLine(0, 10+20*3, 2, WHITE);
	if(aaa.second>=30)
		SSD1306_HLine(0, 10+30*3, 2, WHITE);
	
	if(aaa.second>=40)
		SSD1306_HLine(62, 10+10*3, 2, WHITE);
	if(aaa.second>=50)
		SSD1306_HLine(62, 10+20*3, 2, WHITE);
	if(aaa.second>=60)
		SSD1306_HLine(62, 10+30*3, 2, WHITE);
	

	return 1;
}

/*******************************************************************************
 * Function Name  : BinarScreen
 * Description    :
 *******************************************************************************/
uint8_t BinarScreen(uint16_t code)
{

	return 1;
}

/** @brief Function for getting vector of random numbers.
 *
 * @param[out] p_buff                               Pointer to unit8_t buffer for storing the bytes.
 * @param[in]  length                               Number of bytes to take from pool and place in p_buff.
 *
 * @retval     Number of bytes actually placed in p_buff.
 */
uint8_t random_vector_generate(uint8_t * p_buff, uint8_t size)
{
	uint8_t available;
	uint32_t err_code;
	err_code = nrf_drv_rng_bytes_available(&available);
	APP_ERROR_CHECK(err_code);
	uint8_t length = (size < available) ? size : available;
	err_code = nrf_drv_rng_rand(p_buff, length);
	APP_ERROR_CHECK(err_code);
	return length;
}


/*******************************************************************************
 * Function Name  : TextScreen1
 * Description    :
 *******************************************************************************/
uint8_t TextScreen1(uint16_t code)
{
const char S_HOURS_1[] = "\x97\x80\x91"; /*Р§РђРЎ*/
const char S_HOURS_234[] = "\x97\x80\x91\x80"; /*Р§РђРЎРђ*/
const char S_HOURS_5[] = "\x97\x80\x91\x8e\x82"; /*Р§РђРЎРћР’*/
const char S_HALF[] = "\x8f\x8e\x8b"; /*РџРћР›*/
const char S_WITHOUT[] = "\x81\x85\x87"; /*Р‿Р•Р—*/
const char S_MINUTE_1[] = "\x8c\x88\x8d\x93\x92\x80"; /*РњР�РќРЈРўРђ*/
const char S_MINUTE_2[] = "\x8c\x88\x8d\x93\x92\x9b"; /*РњР�РќРЈРўР«*/
const char S_MINUTE_3[] = "\x8c\x88\x8d\x93\x92"; /*РњР�РќРЈРў*/
	
	if(code == START_COM)
		SSD1306_setFont (font5x8);
	
	RTC_TimeDateTypeDef aaa;
	RTC_CntToTimeDate(RTCCounter, &aaa);
	aaa.hour = aaa.hour > 12 ? aaa.hour - 12 : aaa.hour;

	if(aaa.minute == 0)
	{ //Begin of hour
		SSD1306_BigNum(20, 100, aaa.hour, 3);
		char *ptr = (aaa.hour == 1) ? (char*) S_HOURS_1 : (aaa.hour < 5 ? (char*) S_HOURS_234 : (char*) S_HOURS_5);
		SSD1306_StringCentered(0, 50, ptr, 2);
	}
	else if(aaa.minute == 30)
	{ //half an hour
		SSD1306_StringCentered(0, 100, (char*) S_HALF, 2);
		SSD1306_BigNum(30, 50, aaa.hour + 1 == 24 ? 12 : aaa.hour + 1, 3);
		char temp[10] =
		{ '\0' };
		temp[0] = '\xA3';
		temp[1] = '\xAE';
		SSD1306_StringCentered(0, 17, temp, 2);
	}
	else if(aaa.minute >= 40)
	{ //without 20 min <
		SSD1306_StringCentered(0, 100, (char*) S_WITHOUT, 2);
		SSD1306_BigNum(15, 60, 60 - aaa.minute, 3);
		SSD1306_BigNum(15, 15, aaa.hour + 1 == 24 ? 12 : aaa.hour + 1, 3);
	}
	else
	{ //0-40 min
		SSD1306_BigNum(aaa.minute < 10 ? 25 : 15, 100, aaa.minute, 3);
		char *ptr =
			(aaa.minute == 1 || aaa.minute == 21 || aaa.minute == 31) ?
				(char*) S_MINUTE_1 :
				((aaa.minute % 10 == 2 || aaa.minute % 10 == 3 || aaa.minute % 10 == 4) && aaa.minute / 10 != 1 ?
					(char*) S_MINUTE_2 : (char*) S_MINUTE_3);

		SSD1306_StringCentered(0, 60, ptr, 2);

		char temp[10] =
		{ '\0' };
		uint8_t var = aaa.hour + 1 == 24 ? 12 : aaa.hour + 1;
		if(var < 10)
			temp[0] = var % 10 + '0';
		else
		{
			temp[0] = var / 10 + '0';
			temp[1] = var % 10 + '0';
		}
		SSD1306_StringCentered(0, 18, temp, 3);

		temp[0] = '\xA3';
		temp[1] = '\xAE';
		SSD1306_StringCentered(0, 2, temp, 2);
	}

	return 1;
}

//extern  int ACCEL[3], MAGNET[3];

/*******************************************************************************
 * Function Name  : ExtendScreen
 * Description    :
 *******************************************************************************/
uint8_t ExtendScreen(uint16_t code)
{
#if 0
	static uint8_t m = 0;
	static uint8_t i = 0;
	static int16_t ymax = 0, ymin = 0;
	
	if(code == START_COM)
		orient = LANDSCAPE;
	if(code == STOP_COM)
	{
		orient = PORTRAIT;
		return 0;
	}
	if(code == BTN_LEFT)
	{
		if(++m >= 3)
			m = 0;
	}
	if(code == BTN_RIGHT)
	{
		SSD1306_Clear();
		ymax = ymin = 0;
		i = 0;
	}

	//SSD1306_Num(0, 50, countDown);
//    SSD1306_Num(0, 70, xxxx);

	orient = LANDSCAPE;
	countDown = 5*DISP_1s;

	int ACCEL[3];
	LSM303_GetAccelData(ACCEL);

	/*SSD1306_String(0,0,"_:_____");
	 SSD1306_String(0,9,"_:_____");
	 SSD1306_String(0,18,"_:_____");

	 SSD1306_NumWDesc(0,0,"X:", ACCEL[XAxis]);
	 SSD1306_NumWDesc(0,9,"Y:", ACCEL[YAxis]);
	 SSD1306_NumWDesc(0,18,"Z:", ACCEL[ZAxis]); */
	SSD1306_Char(0, 0, 'x' + m);

	SSD1306_Pixel(i, ACCEL[m]/34+32, WHITE);
	if(code == NEW_SECOND)
		for(uint8_t k=0; k<64; k++)
			SSD1306_Pixel(i, k, WHITE);
	if(ACCEL[m]<ymin) ymin = ACCEL[m];
	if(ACCEL[m]>ymax) ymax = ACCEL[m];
	
	SSD1306_Num(8,0,ymin);
	SSD1306_Num(60, 0, ymax);

	if(i != 128)
		i++;
	
	nrf_delay_ms(15);
#else
	uint8_t data[2];
	LSM303_Read(REG_INT2_THS, data, 2);
	uint8_t thrs = data[0];
	uint8_t durr = data[1];	
	
	static uint8_t ptr = 0; 

	if(code == BTN_DOWN)
	{
		if(ptr==0)
			thrs-=10;
		else if (ptr == 1)
			durr-=10;
	}
	if(code == BTN_UP)
	{
		if(ptr==0)
			thrs+=10;
		else if (ptr == 1)durr+=10;
	}
	if(code == BTN_LEFT)
	{
		if(ptr==0)
			thrs--;
		else if (ptr == 1)durr--;
	}
	if(code == BTN_RIGHT)
	{
		if(ptr==0)
			thrs++;
		else if (ptr == 1)durr++;
	}
	if(code == BTN_MENU)
	{
		if(++ptr >= 3)
			ptr = 0;
	}

	if(BTN_ANY(code))
	{
		LSM303_Init_ShadowGesture();
		LSM303_Write(REG_INT2_THS, thrs);
		LSM303_Write(REG_INT2_DUR, durr);
		countDown = 5*DISP_1s;
	}
	SSD1306_Clear();
	SSD1306_setFont (font5x8);
	
	SSD1306_NumWDesc(10, 70, "THS:", thrs);
	SSD1306_NumWDesc(10, 60, "DUR:", durr);
	
	SSD1306_InvertRect(5, ptr==0?70:(ptr==1?60:50), 50, 9);
	
	nrf_delay_ms(20);

#endif

	return 1;
}



/*******************************************************************************
 * Function Name  :
 * Description    :
 *******************************************************************************/
uint8_t menuScreen(uint16_t code)
{
	/* Icon navigation */
	static uint8_t i = 0, j = 0;

	if(code)
		countDown = 5*DISP_1s;
	
	switch (code)
	{
	case START_COM:
		countDown = 5*DISP_1s;
		break;
	case STOP_COM:
		countDown = 5*DISP_1s;
		break;
	case BTN_RIGHT:
		if(++j > 1)
			j = 0;
		break;
	case BTN_LEFT:
		if(--j > 1)
			j = 1;
		break;
	case BTN_UP:
		if(++i > 3)
			i = 0;
		break;
	case BTN_DOWN:
		if(--i > 3)
			i = 3;
		break;
	case BTN_MENU:
		if(j == 0)
		{
			if(i == 0)
			{
				MenuChange(&Settings);
				return 1;
			}
			if(i == 1)
			{
				MenuChange(&Random);
				return 1;
			}
			if(i == 2)
			{
				MenuChange(&Calendar);
				return 1;
			}
			if(i == 3)
			{
				MenuChange(&Alarm);
				return 1;
			}
		}
		else
		{
			if(i == 0)
			{
				MenuChange(&TimerMode);
				return 1;
			}
			if(i == 1)
			{
				MenuChange(&Game);
				return 1;
			}
			if(i == 2)
			{
				MenuChange(&Flashlight);
				return 1;
			}
			if(i == 3)
			{
				MenuChange(&Stopwatch);
				return 1;
			}
		}
		break;
	}

	/* Draw Menu Icons */
	SSD1306_Bitmap(0, 96, (char*) menu_alarm);
	SSD1306_Bitmap(32, 96, (char*) menu_stopwatch);
	SSD1306_Bitmap(0, 64, (char*) menu_timedate);
	SSD1306_Bitmap(32, 64, (char*) menu_torch);
	SSD1306_Bitmap(0, 32, (char*) menu_rand);
	SSD1306_Bitmap(32, 32, (char*) menu_games);
	SSD1306_Bitmap(0, 0, (char*) menu_settings);
	SSD1306_Bitmap(32, 0, (char*) menu_txt);

	/* Flashing selected icon */
	if(SysTickTimer % 8 < 6)
		SSD1306_InvertRect(j * 32, i * 32, 32, 32);

	return 1;
}

/*******************************************************************************
 * Function Name  : SettScreen
 * Description    :
 *******************************************************************************/
uint8_t SettScreen(uint16_t code)
{
	static uint8_t ptr = 0;

	SSD1306_setFont (font5x8);
	SSD1306_String(0, 120, "Time");
	SSD1306_String(0, 110, "Sound");
	SSD1306_String(0, 100, "Vibro");
	SSD1306_String(0, 90, "RESET");

	if(code)
		countDown = 5*DISP_1s;
	switch (code)
	{
	case START_COM:
		countDown = 5*DISP_1s;
		break;
	case STOP_COM:
		countDown = 5*DISP_1s;
		break;
	case BTN_RIGHT:
		break;
	case BTN_LEFT:
		break;
	case BTN_DOWN:
		if(++ptr > 3)
			ptr = 0;
		break;
	case BTN_UP:
		if(--ptr > 3)
			ptr = 3;
		break;
	case BTN_MENU:
		if(ptr == 0)
			MenuChange(&SetTime);
		if(ptr == 3)
			NVIC_SystemReset();
		break;
	}
	SSD1306_InvertRect(0, 120 + ptr * (-10), 63, 10);

	return 1;
}

/*******************************************************************************
 * Function Name  : TDScreen
 * Description    :
 *******************************************************************************/
uint8_t TDScreen(uint16_t code)
{
	static uint8_t ptr = 0;

	SSD1306_setFont (font5x8);
	char t[15];
	RTC_TimeDateTypeDef aaa;
	RTC_CntToTimeDate(RTCCounter, &aaa);
	TimeToString(t, &aaa);

	if(code)
		countDown = 10*DISP_1s;
	switch (code)
	{
	case START_COM:
		countDown = 10*DISP_1s;
		AlarmDisable();
		break;
	case STOP_COM:
		countDown = 5*DISP_1s;
		UpdateCorrectionCounter();
		AlarmUpdate();
		break;
	case BTN_LEFT:
		if(ptr == 0)
			RTCCounter -= 3600;			
		if(ptr == 1)
			RTCCounter -= 60;		
		if(ptr == 2)
			RTCCounter = (RTCCounter/60)*60;		
		if(ptr == 3)
			RTCCounter -= 3600*24;	
		if(ptr == 4)
			RTCCounter -= 3600*24*30;	
		if(ptr == 5)
			RTCCounter -= 3600*24*365;	
		if(ptr == 6)
			correctionValue -= 1;	
		if(ptr == 7)
			sleepVibrator -= 1;
		break;
	case BTN_RIGHT:
		if(ptr == 0)
			RTCCounter += 3600;		
		if(ptr == 1)
			RTCCounter += 60;	
		if(ptr == 2)
			RTCCounter = (RTCCounter/60)*60 + 30;		
		if(ptr == 3)
			RTCCounter += 3600*24;	
		if(ptr == 4)
			RTCCounter += 3600*24*30;	
		if(ptr == 5)
			RTCCounter += 3600*24*365;	
		if(ptr == 6)
			correctionValue += 1;	
		if(ptr == 7)
			sleepVibrator += 1;
		break;
	case BTN_DOWN:
		if(++ptr > 7)
			ptr = 0;
		break;
	case BTN_UP:
		if(--ptr > 7)
			ptr = 7;
		break;
	case BTN_MENU:
		MenuChange (PARENT);
		break;
	}

	//RTCCounter = RTC_TimeDateToCnt(&aaa);
	RTC_CntToTimeDate(RTCCounter, &aaa);

	SSD1306_NumWDesc(0, 120, "Hr :", aaa.hour);
	SSD1306_NumWDesc(0, 110, "Min:", aaa.minute);
	SSD1306_NumWDesc(0, 100, "Sec:", aaa.second);

	char mon[4] = "";
	for(uint8_t i = 0; i < 3; i++)
		mon[i] = monthMass[aaa.month * 3 + i];

	SSD1306_NumWDesc(0, 90, "Dat:", aaa.date);
	SSD1306_String(0, 80, mon);
	SSD1306_NumWDesc(0, 70, "Yr: ", aaa.year);
	SSD1306_FloatWDesc(0, 60, "", 86400.0/(float)correctionValue, 3);

	
	SSD1306_NumWDesc(0, 50, "SV:", sleepVibrator);
	

	SSD1306_InvertRect(0, 120 + ptr * (-10), 63, 10);

	return 1;
}

/*******************************************************************************
 * Function Name  : SetAlarm
 * Description    :
 *******************************************************************************/
uint8_t SetAlarm(uint16_t code)
{
	static uint8_t ptrX = 0, ptrY = 0;

	if(code)
		countDown = 10*DISP_1s;
	switch (code)
	{
	case START_COM:
		countDown = 10*DISP_1s;
		AlarmDisable();
		break;
	case STOP_COM:
		countDown = 5*DISP_1s;
		AlarmUpdate();
		break;
	case BTN_LEFT:
		if(--ptrX > 11)
			ptrX = 11;
		break;
	case BTN_RIGHT:
		if(++ptrX > 11)
			ptrX = 0;
		break;
	case BTN_UP:
		if(ptrX == 0)
			if(Alarmes[ptrY].hour / 10 < 2)
				Alarmes[ptrY].hour += (Alarmes[ptrY].hour + 10) > 23 ? 20 - Alarmes[ptrY].hour : 10;
		if(ptrX == 1)
			if(Alarmes[ptrY].hour != 19 && Alarmes[ptrY].hour != 9 && Alarmes[ptrY].hour != 23)
				Alarmes[ptrY].hour++;
		if(ptrX == 2)
			if(Alarmes[ptrY].minute / 10 < 5)
				Alarmes[ptrY].minute += 10;
		if(ptrX == 3)
			if(Alarmes[ptrY].minute % 10 != 9)
				Alarmes[ptrY].minute++;
		if(ptrX == 4)
			Alarmes[ptrY].active = !Alarmes[ptrY].active;
		if(ptrX > 4)
			BitFlip(Alarmes[ptrY].day, ptrX - 5);
		break;
	case BTN_DOWN:
		if(ptrX == 0)
			if(Alarmes[ptrY].hour / 10 > 0)
				Alarmes[ptrY].hour -= 10;
		if(ptrX == 1)
			if(Alarmes[ptrY].hour % 10 != 0)
				Alarmes[ptrY].hour--;
		if(ptrX == 2)
			if(Alarmes[ptrY].minute / 10 > 0)
				Alarmes[ptrY].minute -= 10;
		if(ptrX == 3)
			if(Alarmes[ptrY].minute % 10 != 0)
				Alarmes[ptrY].minute--;
		if(ptrX == 4)
			Alarmes[ptrY].active = !Alarmes[ptrY].active;
		if(ptrX > 4)
			BitFlip(Alarmes[ptrY].day, ptrX - 5);
		break;
	case BTN_MENU:
		if(++ptrY > 9)
			ptrY = 0;
		break;
	case BTN_UP_LONG:
		Alarmes[ptrY].active = !Alarmes[ptrY].active; //toggle on/off
		break;
	case BTN_LEFT_LONG:
		Alarmes[ptrY].day = 0xFF; //all days - on
		break;
	case BTN_RIGHT_LONG:
		Alarmes[ptrY].day = 0x1F; //weekdays days - on
		break;
	case BTN_DOWN_LONG:
		Alarmes[ptrY].day = 0x00; //all days - off
		break;
	}

	SSD1306_setFont (font3x5);
	SSD1306_String(35, 120, "MTWTFSS");

	for(uint8_t i = 0; i < 10; i++)
	{
		char s[15];
		s[0] = Alarmes[i].hour / 10 + '0';
		s[1] = Alarmes[i].hour % 10 + '0';
		s[2] = ':';
		s[3] = Alarmes[i].minute / 10 + '0';
		s[4] = Alarmes[i].minute % 10 + '0';
		s[5] = '\0';

		SSD1306_setFont (font5x8);
		SSD1306_String(0, 110 + i * (-10), s);

		for(uint8_t j = 0; j < 8; j++)
		{
			if(j == 0)
				s[0] = Alarmes[i].active ? 'Y' : 'n';
			else
				s[j] = BitIsSet(Alarmes[i].day, j - 1) ? '+' : '-';
		}
		s[8] = '\0';

		SSD1306_setFont(font3x5);
		SSD1306_String(31, 110 + i * (-10), s);
	}

	const uint8_t constpos[] =
	{ 0, 6 * 1, 6 * 3, 6 * 4, 31 + 0, 31 + 4 * 1, 31 + 4 * 2, 
	31 + 4 * 3, 31 + 4 * 4, 31 + 4 * 5, 31 + 4 * 6, 31 + 4 * 7 };

	SSD1306_InvertRect(constpos[ptrX], 110 + ptrY * (-10), ptrX < 4 ? 5 : 3, 9);

	return 1;
}


/*******************************************************************************
 * Function Name  : CalendarDraw
 * Description    :
 *******************************************************************************/
uint8_t CalendarDraw(uint16_t code)
{
	static uint8_t month=1;
	static uint16_t year=1;

	RTC_TimeDateTypeDef cur;
	RTC_CntToTimeDate(RTCCounter, &cur);
	
	switch(code)
	{
	case START_COM:
		countDown = 15*DISP_1s;
		month = cur.month;
		year = cur.year;
		break;
	case STOP_COM:
		countDown = 5*DISP_1s;
		break;
	case BTN_DOWN:
		year--;
		break;
	case BTN_UP:
		year++;
		break;
	case BTN_RIGHT:
		if(++month >= 13)
		{
			month = 1;
			year++;
		}
		break;
	case BTN_LEFT:
		month--;
		if(month >= 13 || month == 0)
		{
			month = 12;
			year--;
		}
		break;
	case BTN_MENU:
		MenuChangeMain();
		countDown = 5*DISP_1s;
		break;
	}
	
	if(BTN_ANY(code) == 0 && code != START_COM) //if code 
		return 0;

#define yz (year - ((14 - month) / 12))
	uint8_t start = (7000 - 1 + (1 + yz + yz / 4 - yz / 100 + yz / 400 + (31 * (month + 12 * ((14 - month) / 12) - 2)) / 12)) % 7;

	SSD1306_setFont (font5x8);
	for(uint8_t i = 0; i < 7; i++)
		SSD1306_Char(0, 88 + i * (-10), WeekDays[i * 3]);

	char yr[7] = "<", mon[6] = "<...>", tmp[5];

	for(uint8_t i = 0; i < 3; i++)
		mon[i + 1] = monthMass[month * 3 + i];

	itoa_(year, tmp);
	strcat_(yr, tmp);
	strcat_(yr, ">");

	SSD1306_String(15, 119, yr);
	SSD1306_String(18, 111, mon);
	SSD1306_InvertRect(0, 111, 63, 17);

	SSD1306_setFont (font3x5);
	for(uint8_t i = 0; i < 7; i++)
	{
		for(uint8_t j = 0; j < 6; j++)
		{
			uint8_t num = i + j * 7 - start + 1;
			uint8_t monthNumDays, monthDays[1+12] = {0,31,28,31,30,31,30,30,31,30,31,30,31};
			if((month == 2) && (year%4 ==0)) //leap year
				monthNumDays = 29;
			else
				monthNumDays = monthDays[month];
			if(num <= monthNumDays && num > 0)
				SSD1306_Num(10 + j * 11, 90 - i * 10, num);

			if(num == cur.date && cur.month == month && cur.year == year)
				SSD1306_InvertRect(10 + j * 11 - 2, 90 - i * 10 - 2, 11, 9);
		}
	}

	return 1;
}

/*******************************************************************************
 * Function Name  : FlashLight
 * Description    :
 * Input		     :
 *******************************************************************************/
uint8_t FlashLight(uint16_t state)
{
	switch (state)
	{
	case BTN_RIGHT:
	case BTN_LEFT:
	case BTN_UP:
	case BTN_DOWN:
	case BTN_MENU:
		MenuChangeMain();
		countDown = 5*DISP_1s;
		break;
	}

	SSD1306_Brightness(255);
	SSD1306_Fill();
	countDown = 4*DISP_1s;

	return 1;
}

/*******************************************************************************
 * Function Name  : RandomScreen
 * Description    :
 * Input		     :
 *******************************************************************************/
uint8_t RandomScreen(uint16_t state)
{

	return 1;
}

/*******************************************************************************
 * Function Name  : LogScreen
 * Description    :
 * Input		     :
 *******************************************************************************/
#define DRAW_LOG_STRING(j,k,LogBuffer) \
	do{	\
		SSD1306_setFont(font3x5);									\
		SSD1306_StringPtr(0, 112-j*12, LogBuffer, k*LOG_NUM_CHARS, k*LOG_NUM_CHARS+4);     \
		SSD1306_StringPtr(0, 112-j*12-6, LogBuffer, k*LOG_NUM_CHARS+12, k*LOG_NUM_CHARS+16);	\
		SSD1306_setFont(font5x8);                                   \
		SSD1306_StringPtr(17, 112-j*12-5, LogBuffer, k*LOG_NUM_CHARS+4, k*LOG_NUM_CHARS+12); \
		if(LogBuffer[k*LOG_NUM_CHARS+16]) drawFastHLine(18, 112-j*12-7, LogBuffer[k*LOG_NUM_CHARS+16], WHITE); \
	}while(0);

uint8_t LogScreen(uint16_t state)
{
	if(state == BTN_MENU)
		LogFirstField = LogNumOfFields = 0;

	/* display current time in format MMSSm */
	SSD1306_setFont (font5x8);
	char t[11] =
	{ '\0' };
#if 0
	uint32_t tick = SysTickTimer/10;
//t[10] = '0' + (char)(SysTickTimer%10);
	t[9] = '0' + (char)(tick%10);//s
	t[8] = '0' + (char)(tick%60/10);//s
	t[7] = '0' + (char)(tick/60%10);//m
	t[6] = '0' + (char)(tick/60%60/10);//m
	t[5] = ':';//:
	t[2] = '/';
	t[4] = '0' + (char)(tick/3600%24%10);//h
	t[3] = '0' + (char)(tick/3600%24/10);//h
	t[1] = '0' + (char)(tick/86400%10);//d
	t[0] = '0' + (char)(tick/86400/10);//d
#else
	RTC_TimeDateTypeDef v;
	RTC_CntToTimeDate(RTCCounter, &v);
	t[9] = '0' + (char) (v.second % 10); //s
	t[8] = '0' + (char) (v.second / 10); //s
	t[7] = '0' + (char) (v.minute % 10); //m
	t[6] = '0' + (char) (v.minute / 10); //m
	t[5] = ':'; //:
	t[2] = '/';
	t[4] = '0' + (char) (v.hour % 10); //h
	t[3] = '0' + (char) (v.hour / 10); //h
	t[1] = '0' + (char) (v.date % 10); //d
	t[0] = '0' + (char) (v.date / 10); //d
#endif

	SSD1306_String(0, 120 - 1, t);
	SSD1306_InvertRect(48, 120 - 1, 12, 9);

	/* Display circular Log buffer */
	if(LogNumOfFields > (LOG_NUM_OF_FIELDS - 1))
	{
		uint8_t j = 0;
		for(uint8_t k = LogFirstField; k < LOG_NUM_OF_FIELDS; k++, j++)
			DRAW_LOG_STRING(j, k, LogBuffer);

		for(uint8_t k = 0; k < LogFirstField; k++, j++)
			DRAW_LOG_STRING(j, k, LogBuffer);
	}
	else
		for(uint8_t k = 0; k < LogNumOfFields; k++)
			DRAW_LOG_STRING(k, k, LogBuffer);

	countDown = 4*DISP_1s;

	return 1;
}

uint8_t LogAddInfo(char * s, int WarnLvl, uint8_t volume)
{
	const char Warn[] = " IWE"; // NULL/Info/Warning/Error

	/* add timestamp to log message */
	char t[LOG_NUM_CHARS];
#if 0
	int var = SysTickTimer/10;
	t[15] = '0' + (char)(var%10);
	t[14] = '0' + (char)((var%60)/10);
	t[13] = '0' + (char)((var/60)%10);
	t[12] = '0' + (char)((var/600)%10);
	t[3] = '0' + (char)(var/3600%24%10);
	t[2] = '0' + (char)(var/3600%24/10);
	t[1] = '0' + (char)(var/86400%10);
	t[0] = '0' + (char)(var/86400/10);
#else
	RTC_TimeDateTypeDef v;
	RTC_CntToTimeDate(RTCCounter, &v);
	t[15] = '0' + (char) (v.second % 10); //s
	t[14] = '0' + (char) (v.second / 10); //s
	t[13] = '0' + (char) (v.minute % 10); //m
	t[12] = '0' + (char) (v.minute / 10); //m

	t[3] = '0' + (char) (v.hour % 10); //h
	t[2] = '0' + (char) (v.hour / 10); //h
	t[1] = '0' + (char) (v.date % 10); //d
	t[0] = '0' + (char) (v.date / 10); //d
#endif

	/* Write Log message to circular Log buffer */
	int len = strlen(s);
	for(uint8_t i = 4; i < 12 || i < (len + 4); i++)
		t[i] = (i - 4 < len) ? (*(s + i - 4)) : ' ';

	t[11] = Warn[WarnLvl > 3 ? 0 : WarnLvl];
	t[16] = volume > 45 ? 45 : volume;

	for(uint16_t i = 0; i < LOG_NUM_CHARS; i++)
		LogBuffer[LogFirstField * LOG_NUM_CHARS + i] = t[i];

	if(++LogFirstField >= LOG_NUM_OF_FIELDS)
		LogFirstField = 0;

	if(LogNumOfFields <= (LOG_NUM_OF_FIELDS - 1))
		LogNumOfFields++;

	return 1;
}

uint8_t DrawSleepLog(uint16_t state)
{
	if(state == BTN_DOWN)
		for(uint16_t i = 0; i < 8 * 60; i++)
			SleepLog[i] = 0;

	for(uint8_t i = 1; i < 8; i++)
		for(uint8_t j = 1; j < 6; j++)
		{
			drawFastVLine(i * 8, j * 20, 3, WHITE);
			drawFastHLine(i * 8 - 1, j * 20 + 1+5, 3, WHITE);
		}

	for(uint8_t i = 0; i < 8; i++)
		for(uint8_t j = 0; j < 60; j++)
			if(SleepLog[60 * i + j] > 0)
			{
				SSD1306_Pixel(4 + i * 8, 6 + j * 2, WHITE);
				SSD1306_Pixel(4 + i * 8, 6 + j * 2 + 1, WHITE);
			}

	countDown = 4*DISP_1s;

	return 1;
}

uint8_t AllDataDisplayCb(uint16_t state)
{
	static uint8_t displayPage = 0;

		int temp1[3];
	
	static uint8_t modeR = 4;
	switch (state)
	{
	case BTN_RIGHT:
		break;
	case BTN_LEFT:
		break;
	case BTN_UP:
		if(++displayPage > 3)
			displayPage = 0;
		break;
	case BTN_DOWN:
		if(--displayPage > 3)
			displayPage = 3;
		break;
	case BTN_MENU:
		modeR++;
		if(modeR == 5) modeR = 0;
		//MenuChangeMain();
		if(modeR == 0)
		{
			SoundBeep(2000, 500);
		}
		if(modeR==1)
		{
			SoundBeep(500, 2500);
		}
		if(modeR == 2)
		{ //delete!!!!!!!!!!!
			nrf_gpio_pin_set(VIBRO);
			nrf_delay_ms(500);
			nrf_gpio_pin_clear(VIBRO);
			nrf_delay_ms(100);
		}
		if(modeR == 3)
		{
			//delete!!!!!!!!!
			nrf_gpio_pin_set(VIBRO);
			nrf_delay_ms(500);
			LSM303_GetAccelData(temp1);
			nrf_delay_ms(500);
			nrf_gpio_pin_clear(VIBRO);
			nrf_delay_ms(100);
		}
		if(modeR == 4)
			LSM303_Init_ShadowGesture();
	
		break;
	}

	SSD1306_setFont (font5x8);

	if(displayPage == 0)
	{
		SSD1306_String(5, 120, "<Hardware>");

		int temp1[3];
		LSM303_GetAccelData(temp1);
		SSD1306_NumWDesc(0, 120 - 8 * 1, "aX:", temp1[XAxis]);
		SSD1306_NumWDesc(0, 120 - 8 * 2, "aY:", temp1[YAxis]);
		SSD1306_NumWDesc(0, 120 - 8 * 3, "aZ:", temp1[ZAxis]);
		LSM303_GetMagnetData(temp1);
		SSD1306_NumWDesc(0, 118 - 8 * 4, "mX:", temp1[XAxis]);
		SSD1306_NumWDesc(0, 118 - 8 * 5, "mY:", temp1[YAxis]);
		SSD1306_NumWDesc(0, 118 - 8 * 6, "mZ:", temp1[ZAxis]);

		BMP180_Init();
		float xx;
		getPressure(&xx);
		SSD1306_FloatWDesc(0, 116 - 8 * 7, "P:", xx, 1);
		SSD1306_FloatWDesc(0, 116 - 8 * 8-3, "Pmm:", xx*0.0075, 1);

//		getPressure(&xx);
//		SSD1306_FloatWDesc(0, 116 - 8 * 8, "P:", pressureToAltitude(100000, xx), 1);

		getTemperature(&xx);
		SSD1306_FloatWDesc(0, 116 - 8 * 10, "T:", xx, 1);
		
		SSD1306_Num(0,0,modeR);

		//SSD1306_NumWDesc(0, 116 - 8 * 10, "Fcnt:", failACnt);
	}
	else if(displayPage == 1)
	{
		SSD1306_String(15, 120, "<Time>");

		/* display current time in format MMSSm */
		char t[11] =
		{ '\0' };
		uint32_t tick = SysTickTimer / 10;
		//t[10] = '0' + (char)(SysTickTimer%10);
		t[9] = '0' + (char) (tick % 10); //s
		t[8] = '0' + (char) (tick % 60 / 10); //s
		t[7] = '0' + (char) (tick / 60 % 10); //m
		t[6] = '0' + (char) (tick / 60 % 60 / 10); //m
		t[5] = ':'; //:
		t[4] = '0' + (char) (tick / 3600 % 24 % 10); //h
		t[3] = '0' + (char) (tick / 3600 % 24 / 10); //h
		t[2] = ':'; //:
		t[1] = '0' + (char) (tick / 86400 % 10); //d
		t[0] = '0' + (char) (tick / 86400 / 10); //d
		SSD1306_String(0, 120 - 8 * 1, t);

		t[10] = '0' + (char) (UpTimeCounter % 10); //s
		tick = UpTimeCounter / 10;
		t[9] = '0' + (char) (tick % 10); //s
		t[8] = '0' + (char) (tick % 60 / 10); //s
		t[7] = '0' + (char) (tick / 60 % 10); //m
		t[6] = '0' + (char) (tick / 60 % 60 / 10); //m
		t[5] = ':'; //:
		t[4] = '0' + (char) (tick / 3600 % 24 % 10); //h
		t[3] = '0' + (char) (tick / 3600 % 24 / 10); //h
		t[2] = ':'; //:
		t[1] = '0' + (char) (tick / 86400 % 10); //d
		t[0] = '0' + (char) (tick / 86400 / 10); //d
		SSD1306_String(0, 120 - 8 * 2, t);

		t[10] = '0' + (char) (DownTimeCounter % 10); //s
		tick = DownTimeCounter / 10;
		t[9] = '0' + (char) (tick % 10); //s
		t[8] = '0' + (char) (tick % 60 / 10); //s
		t[7] = '0' + (char) (tick / 60 % 10); //m
		t[6] = '0' + (char) (tick / 60 % 60 / 10); //m
		t[5] = ':'; //:
		t[4] = '0' + (char) (tick / 3600 % 24 % 10); //h
		t[3] = '0' + (char) (tick / 3600 % 24 / 10); //h
		t[2] = ':'; //:
		t[1] = '0' + (char) (tick / 86400 % 10); //d
		t[0] = '0' + (char) (tick / 86400 / 10); //d
		SSD1306_String(0, 120 - 8 * 3, t);

		SSD1306_Num(0, 120 - 8 * 5, UpTimeCounter);
		SSD1306_Num(0, 120 - 8 * 6, DownTimeCounter);

		SSD1306_Num(0, 120 - 8 * 7, TimerCounter);
		SSD1306_Num(0, 120 - 8 * 8, stopWatchCounter);

		SSD1306_Num(0, 120 - 8 * 10, correctionCoundown);
	}
	else if(displayPage == 2)
	{
		SSD1306_String(10, 120, "<List 1>");

		SSD1306_Num(0, 120 - 8 * 1, LSM303_ReadReg(REG_CTRL_1));
		nrf_delay_ms(5);
		SSD1306_Num(0, 120 - 8 * 2, LSM303_ReadReg(REG_CTRL_2));
		nrf_delay_ms(5);
		SSD1306_Num(0, 120 - 8 * 3, LSM303_ReadReg(REG_CTRL_3));
		nrf_delay_ms(5);
		SSD1306_Num(0, 120 - 8 * 4, LSM303_ReadReg(REG_CTRL_4));
		nrf_delay_ms(5);
		SSD1306_Num(0, 120 - 8 * 5, LSM303_ReadReg(REG_CTRL_5));
		nrf_delay_ms(5);
		SSD1306_Num(0, 120 - 8 * 6, LSM303_ReadReg(REG_CTRL_6));
		nrf_delay_ms(5);
		SSD1306_Num(0, 120 - 8 * 7, LSM303_ReadReg(REG_HP_FILT_RST));
		nrf_delay_ms(5);
		SSD1306_Num(0, 120 - 8 * 8, LSM303_ReadReg(REG_REF));
		nrf_delay_ms(5);
		SSD1306_Num(0, 120 - 8 * 9, LSM303_ReadReg(REG_STATUS));
		nrf_delay_ms(5);
		SSD1306_Num(0, 120 - 8 * 10, LSM303_ReadReg(REG_INT1_CFG));
		nrf_delay_ms(5);
		SSD1306_Num(0, 120 - 8 * 11, LSM303_ReadReg(REG_INT1_SRC));
		nrf_delay_ms(5);

		SSD1306_Num(32, 120 - 8 * 1, LSM303_ReadReg(REG_INT1_THS));
		nrf_delay_ms(5);
		SSD1306_Num(32, 120 - 8 * 2, LSM303_ReadReg(REG_INT1_DUR));
		nrf_delay_ms(5);
		SSD1306_Num(32, 120 - 8 * 3, LSM303_ReadReg(REG_INT2_CFG));
		nrf_delay_ms(5);
		SSD1306_Num(32, 120 - 8 * 4, LSM303_ReadReg(REG_INT2_SRC));
		nrf_delay_ms(5);
		SSD1306_Num(32, 120 - 8 * 5, LSM303_ReadReg(REG_INT2_THS));
		nrf_delay_ms(5);
		SSD1306_Num(32, 120 - 8 * 6, LSM303_ReadReg(REG_INT2_DUR));
		nrf_delay_ms(5);
		SSD1306_Num(32, 120 - 8 * 7, LSM303_ReadReg(REG_CLICK_CFG));
		nrf_delay_ms(5);
		SSD1306_Num(32, 120 - 8 * 8, LSM303_ReadReg(REG_CLICK_SRC));
		nrf_delay_ms(5);
		SSD1306_Num(32, 120 - 8 * 9, LSM303_ReadReg(REG_CLICK_THS));
		nrf_delay_ms(5);
		SSD1306_Num(32, 120 - 8 * 10, LSM303_ReadReg(REG_TIME_LIM));
		nrf_delay_ms(5);
		SSD1306_Num(32, 120 - 8 * 11, LSM303_ReadReg(REG_TIME_LAT));
		nrf_delay_ms(5);

	}
	else if(displayPage == 3)
	{
		SSD1306_String(0, 120, "<List 2>");
	}

	countDown = 4*DISP_1s;

	return 1;
}

uint8_t StopWatchCb(uint16_t code)
{
	if(code)
		countDown = 5*DISP_1s;

	switch (code)
	{
	case BTN_RIGHT:
		break;
	case BTN_LEFT:
		break;
	case BTN_UP:
		stopWatchEnabled = !stopWatchEnabled;
		break;
	case BTN_DOWN:
		stopWatchCounter = 0;
		stopWatchEnabled = false;
		break;
	case BTN_MENU:
		MenuChangeMain();
		break;

	}

	SSD1306_setFont (font5x8);
	SSD1306_BigString(10, 112, "Stop", 2);
	SSD1306_BigString(5, 96, "Watch", 2);

	char t[15] =
	{ '\0' };
	if(stopWatchCounter / 864000 != 0)
	{
		t[0] = (stopWatchCounter / 864000 / 10 != 0) ? ('0' + (char) (stopWatchCounter / 864000 / 10)) : ' '; //d
		t[1] = '0' + (char) (stopWatchCounter / 864000 % 10); //d
		t[2] = ' '; //:
		t[3] = 'd';
		SSD1306_BigString(10, 80, t, 2);
	}

	if(stopWatchCounter / 3600 != 0)
	{
		t[0] = (stopWatchCounter / 36000 % 24 / 10 != 0) ? ('0' + (char) (TimerCounter / 36000 % 24 / 10)) : ' '; //h
		t[1] = '0' + (char) (stopWatchCounter / 36000 % 24 % 10); //h
		t[2] = ' '; //:
		t[3] = 'h';
		SSD1306_BigString(10, 60, t, 2);
	}

	t[4] = '0' + (char) (stopWatchCounter / 10 % 10); //s
	t[3] = '0' + (char) (stopWatchCounter / 10 % 60 / 10); //s
	t[2] = ':'; //:
	t[1] = '0' + (char) (stopWatchCounter / 600 % 10); //m
	t[0] = (stopWatchCounter / 600 % 60 / 10) ? ('0' + (char) (stopWatchCounter / 600 % 60 / 10)) : ' '; //m
	SSD1306_BigString(5, 40, t, 2);

	t[4] = '\0'; //:
	t[2] = t[3] = '0';
	t[1] = '0' + (char) (stopWatchCounter % 10); //ms
	t[0] = '.';
	SSD1306_BigString(15, 20, t, 2);

	SSD1306_NumWDesc(10, 0, "FLG=", stopWatchEnabled);

	return 1;
}

uint8_t TimerCb(uint16_t code)
{
	if(code)
		countDown = 5*DISP_1s;

	switch (code)
	{
	case BTN_RIGHT:
		TimerCounter += 30 * 60;
		break;
	case BTN_LEFT:
		TimerCounter += 10 * 60;
		break;
	case BTN_UP:
		TimerCounter += 4 * 60 * 60;
		break;
	case BTN_DOWN:
		TimerCounter += 60;
		break;
	case BTN_MENU:
		MenuChangeMain();
		break;
	case BTN_UP_LONG | BTN_DOWN_LONG:
		TimerCounter = 0;
		break;
	}

	SSD1306_setFont (font5x8);

	SSD1306_BigString(5, 112, "Timer", 2);

	char t[6] =
	{ '\0' };

	if(TimerCounter / 86400 != 0)
	{
		t[0] = (TimerCounter / 86400 / 10 != 0) ? ('0' + (char) (TimerCounter / 86400 / 10)) : ' '; //d
		t[1] = '0' + (char) (TimerCounter / 86400 % 10); //d
		t[2] = ' '; //:
		t[3] = 'd';
		SSD1306_BigString(10, 80, t, 2);
	}

	if(TimerCounter / 3600 != 0)
	{
		t[0] = (TimerCounter / 3600 % 24 / 10 != 0) ? ('0' + (char) (TimerCounter / 3600 % 24 / 10)) : ' '; //h
		t[1] = '0' + (char) (TimerCounter / 3600 % 24 % 10); //h
		t[2] = ' '; //:
		t[3] = 'h';
		SSD1306_BigString(10, 60, t, 2);
	}

	t[4] = '0' + (char) (TimerCounter % 10); //s
	t[3] = '0' + (char) (TimerCounter % 60 / 10); //s
	t[2] = ':'; //:
	t[1] = '0' + (char) (TimerCounter / 60 % 10); //m
	t[0] = '0' + (char) (TimerCounter / 60 % 60 / 10); //m
	SSD1306_BigString(5, 20, t, 2);

	SSD1306_Checkbox(10, 0, 1);
	SSD1306_String(20, 0, "Auto");

	return 1;
}
