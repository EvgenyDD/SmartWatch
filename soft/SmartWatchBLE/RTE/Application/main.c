/* Includes ------------------------------------------------------------------*/
#include "nrf.h"

#include "BLE_App.h"
#include "app_timer.h"
#include "nrf_soc.h"

#include "periph.h"
#include "SSD1306.h"
#include "LSM303.h"
#include "BMP180.h"

#include "menu.h"
#include "UI.h" 
#include "dispatcher.h"
#include "string.h"

#include "shadow_logic.h" 

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
const uint8_t COMPILE_TIME[] = __TIME__;
const uint8_t COMPILE_DATE[] = __DATE__;

volatile static uint8_t buttonCounter = 0;
volatile static uint16_t buttonState = 0;


volatile uint32_t _L2_stepCounter = 0;	
uint32_t _L1_stepTime = 0;

uint32_t TotalStepTime = 0;

volatile uint32_t TotalStepCounter = 0;	


uint16_t stepCountdown = 0;
uint8_t _L2_stepCounterActive = 0; //flag

uint8_t sleepVibrator = 0;


bool Evnt_Hour = false, Evnt_WakeUp = true, Evnt_Browsing = false;

int ACCEL[3], MAGNET[3];

bool BluetoothEnabled = false;


static const uint8_t buttonsNum[5] = { BTN_0, BTN_1, BTN_2, BTN_3, BTN_4 };



/* Extern variables ----------------------------------------------------------*/
extern char SleepLog[8 * 60];

extern volatile uint32_t RTCCounter,SysTickTimer;
extern volatile int16_t countDown;

extern int32_t correctionCoundown;

extern dispID alarmRingEntity;


/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
 * Function Name  : 
 * Description    : 
 *******************************************************************************/
void GPIOTE_IRQHandler(void)
{
	// This handler will be run after wakeup from system ON (GPIO wakeup)
	if(NRF_GPIOTE->EVENTS_PORT)
	{
		NRF_GPIOTE->EVENTS_PORT = 0;

		for(uint8_t i = 0; i < 5; i++)
			if(!nrf_gpio_pin_read(buttonsNum[i]))
				BitSet(buttonState, i);
			 
		if(alarmRingEntity)
		{
			DispatcherRemoveEvnt(alarmRingEntity); 
			AlarmUpdate();
			alarmRingEntity = 0;
		}

		if(countDown == -1 || Evnt_WakeUp == false)
		{
			if(Evnt_Browsing == false)
				Evnt_Browsing = true;
			else
				Evnt_WakeUp = true; 
		}
		else
			buttonCounter = 1;
	}
}


/*******************************************************************************
 * Function Name  : 
 * Description    : 
 *******************************************************************************/
void ButtonProcess()
{
#define DEBOUNCE        3
#define SHORT_PRESS     4
#define LONG_PRESS      9

	if(++buttonCounter == DEBOUNCE)
	{
		for(uint8_t i = 0; i < 5; i++)
			if(nrf_gpio_pin_read(buttonsNum[i]) == 0)
				BitSet(buttonState, i);
		return;
	}
	if(buttonCounter >= SHORT_PRESS && buttonCounter < LONG_PRESS)
	{
		uint16_t temp = 0;

		for(uint8_t i = 0; i < 5; i++)
			if(nrf_gpio_pin_read(buttonsNum[i]) == 0)
				BitSet(temp, i);

		if(temp == 0)
		{
			SoundBeep(3000, 30); 
			UIShortButton(buttonState);
			
			if(buttonState == BTN_LEFT)
			{
				_L2_stepCounter = _L1_stepTime = 0;
				TotalStepCounter =0;
				TotalStepTime =0;
			}
			
			buttonState = buttonCounter = 0;
		}
		return;
	}
	if(buttonCounter == LONG_PRESS)
	{
		buttonState = 0;
		for(uint8_t i = 0; i < 5; i++)
			if(nrf_gpio_pin_read(buttonsNum[i]) == 0)
				BitSet(buttonState, i + 5);

		if(buttonState)
		{
			if(buttonState == (BTN_UP_LONG|BTN_LEFT_LONG|BTN_RIGHT_LONG|BTN_DOWN_LONG)) //all 4 right-side buttons are long pressed together
				NVIC_SystemReset();

			SoundBeep(4000, 50);
			UILongButton(buttonState);
		}

		buttonState = buttonCounter = 0;
	}
}


/*******************************************************************************
 * Function Name  : 
 * Description    : 
 *******************************************************************************/
void RTC1_IRQHandler(void)
{
	int vall[3];
static uint8_t _L0_stepTime = 0;
	
	if((NRF_RTC1->EVENTS_TICK != 0) && ((NRF_RTC1->INTENSET & RTC_INTENSET_TICK_Msk) != 0))
	{
		NRF_RTC1->EVENTS_TICK = 0;
		
		//Reload watchdog register 0
		NRF_WDT->RR[0] = 0x6E524635; 
		
		// buttons handler
		if(buttonCounter > 0)
			ButtonProcess();
		
		_L0_stepTime++;
		
CallDispSecondTime:		
		// events dispatcher
		DispatcherCallback();
		DispatcherEnable(); //for corr-- 
		
		// Correct clock
		if(correctionCoundown>0)
		{
			correctionCoundown--;
			if(correctionCoundown == 0)
			{
				UpdateCorrectionCounter(); 
				//LogAddInfo ("++Corr", LOG_INFO, 0);
				goto CallDispSecondTime;
			} 
		}
		else if(correctionCoundown < 0)
		{
			correctionCoundown++;
			if(correctionCoundown == 0)
			{
				UpdateCorrectionCounter();
				//LogAddInfo("--Corr", LOG_INFO, 0);
				DispatcherDisable(); //next time DispatcherCallback() won't work
			}
		}

///////////////////////////////////////////////////////////////////////////////		
/////////////////////////////// STEP COUNTER //////////////////////////////////
///////////////////////////////////////////////////////////////////////////////		
#define _L0_STEP_MIN_LEN (DISP_100ms*2)
#define _L0_STEP_MAX_LEN (DISP_1s*1)	
	
#define _L2_STEPS_INTERV_DET	(20) 
#define _L2_STEPS_INTERV_TIME	(DISP_1s*17)
#define STEP_INACT_VAL 170
		
		// Level 0 - threshold detection
		// Level 1 - step detection
		// Level 2 - step + basic filtration
		// Level 3 - complete trip detection
extern uint8_t bitch;
		
		static uint8_t penetrationFlag = 0;
		static uint8_t _L2_stepInactiveTimer = 0;
		
		if(nrf_gpio_pin_read(LSM303_INT2))
		{
			//nrf_gpio_pin_set(LED_WHT);
			if(penetrationFlag == 0) //don't enter here multiple times))
			{ //threshold detection
				
				if(_L2_stepInactiveTimer == 100)
					_L2_stepInactiveTimer = 0;
				
				if(_L0_stepTime > _L0_STEP_MIN_LEN && _L0_stepTime < _L0_STEP_MAX_LEN)
				{ //step detection
					 
					if(++_L2_stepCounter >= _L2_STEPS_INTERV_DET)
					{
						if(_L2_stepInactiveTimer < _L2_STEPS_INTERV_TIME)
						{ 
							if(countDown == -1)
								TWI_ENABLE;

							LSM303_ReadReg(REG_INT1_SRC);

							if(countDown == -1)
								TWI_DISABLE; 
							
							if(abs(vall[YAxis]) >-50 )
							{
								TotalStepTime += _L1_stepTime;
								TotalStepCounter += _L2_stepCounter;
							}
						}
						
						_L1_stepTime = 0;
						_L2_stepCounter = 0;
						_L2_stepInactiveTimer = 0;
					}
					else
					{
						_L1_stepTime += _L0_stepTime;
						_L2_stepInactiveTimer = 0;
					}
				}
				
				_L0_stepTime = 0;
				
				penetrationFlag = 1; 
			}
			
			/*TWI_ENABLE;  
			uint8_t val, val2;
			LSM303_Read(REG_INT2_SRC, &val, 1);
			
			TWI_DISABLE;*/
		}
		else
		{
			penetrationFlag = 0; 
			//nrf_gpio_pin_clear(LED_WHT);
		}
		
		if(_L2_stepInactiveTimer < 100) _L2_stepInactiveTimer++;
		
		if(_L2_stepInactiveTimer == 100)
		{
			_L1_stepTime = 0;
			_L2_stepCounter = 0;
		}
			
		
		bitch = _L2_stepInactiveTimer;
		
		//if(stepCountdown<DISP_1Hr) stepCountdown++;
		
///////////////////////////////////////////////////////////////////////////////		
///////////////////////// Gesture * Show time /////////////////////////////////
///////////////////////////////////////////////////////////////////////////////				
		static uint8_t shadowGestureCounter = 0;
		static bool isOnSide = false;

		/* Gestures: show time */
		if(countDown == -1 && (RTCCounter % (3600 * 24)) > 3600 * 8)
		{ 
			//nrf_gpio_pin_write(LED_WHT, nrf_gpio_pin_read(LSM303_INT1));
			if(nrf_gpio_pin_read (LSM303_INT1))
			{ 
				isOnSide = true;
				shadowGestureCounter = 0;
			}  
			else if(isOnSide && shadowGestureCounter == 0)
			{
				/* was on a side and now is not on the side */
				/* start counter  if it was off */
				isOnSide = false;
				shadowGestureCounter = 7;
			}
		}

		if(shadowGestureCounter && !isOnSide)
		{
			if(--shadowGestureCounter == 0)
			{
				
				if(countDown == -1)
					TWI_ENABLE;
				//nrf_delay_ms(5);
				LSM303_GetAccelData(vall);
//			if(vall[XAxis] == 0 && vall[YAxis] == 0 && vall[ZAxis] == 0)
//			{
//				failACnt++;
//				LogAddInfo("AFail", LOG_ERR, 0);
//				LSM303_Init_ShadowGesture();
//			}
				if(countDown == -1)
					TWI_DISABLE;

				/* check if clock position is gorizontal */
				if(abs(vall[XAxis]) < 150 && vall[YAxis] < 150)
					Evnt_Browsing = true;
			}
		}
		
///////////////////////////////////////////////////////////////////////////////		
///////////////////////// Sleep tracker ///////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////	
		if((RTCCounter % (3600 * 24)) < (3600 * 10))
		{
			if(nrf_gpio_pin_read (LSM303_INT1))
			{ 
				//nrf_gpio_pin_toggle(LED_WHT);
				if(countDown == -1)
					TWI_ENABLE;
				 
				LSM303_ReadReg(REG_INT1_SRC);
				
				if(countDown == -1)
					TWI_DISABLE;
				
				static uint32_t prevRTC = 0;
				static uint8_t minuteSleepCounter = 0;
				static uint8_t newMinuteCounter = 0;
				if(RTCCounter != prevRTC)
				{					 
					prevRTC = RTCCounter; 
					
					if(SleepLog[(RTCCounter%(3600*24))/60] < 50)
						SleepLog[(RTCCounter%(3600*24))/60]++;


					if(sleepVibrator != 0)
					{
						if(minuteSleepCounter)
						{
							minuteSleepCounter--;
							Vibrate(3 * DISP_100ms);
						}

						if((prevRTC / 60) != newMinuteCounter)
						{
							newMinuteCounter = 0;
							minuteSleepCounter = sleepVibrator;
						}
					}
				}
			}
		}
	}
}
   

/*******************************************************************************
 * Function Name  : 
 * Description    : 
 *******************************************************************************/
void UI_Handler()
{
	if(Evnt_WakeUp == true)
	{
nrf_gpio_pin_clear(LED_WHT);

		/* Enable TWI */
		TWI_ENABLE;
		SPI_ENABLE;

		/* Turn on display */
		SSD1306_On();
		LSM303_On();

		countDown = 8*DISP_1s; 

		while(countDown != -1)
		{
			/* Update screen */
			MenuCallback();

			/* symbolic delay */
			nrf_delay_ms(10);

			/* device is charging */
			if(nrf_gpio_pin_read (VIN_SENSE))
			{
				countDown = 5*DISP_1s;
				nrf_gpio_pin_set (LED_RED);
			}
			else
				nrf_gpio_pin_clear (LED_RED);
		}

		SSD1306_EndSplash();
		MenuChangeMain();

		SSD1306_Off();
		LSM303_Off(); 
		TWI_DISABLE;
		SPI_DISABLE; 
		
		countDown = -1; 

		Evnt_WakeUp = false;
		Evnt_Browsing = false;
		return;
	} 

	if(Evnt_Browsing == true)
	{
		countDown = 4*DISP_1s;

		/* Enable TWI */
		//NRF_TWI1->ENABLE = TWI_ENABLE_ENABLE_Enabled << TWI_ENABLE_ENABLE_Pos;
		SPI_ENABLE;

		/* Turn on display */
		SSD1306_On();
		//LSM303_On();

		while(countDown != -1 && Evnt_WakeUp == false)
		{
			/* Update screen */
			SSD1306_Clear();
			BIGScreen(0);
			SSD1306_ReDraw();
			//MenuCallback();		

			/* symbolic delay */
			nrf_delay_ms(10);
		}

		//SSD1306_EndSplash();
		countDown = -1;

		SSD1306_Off();
		LSM303_Off();
		TWI_DISABLE;
		SPI_DISABLE;

		Evnt_Browsing = false;
	}
} 


/*******************************************************************************
 * Function Name  : 
 * Description    : 
 *******************************************************************************/
int main(void)
{
	ble_stack_init();

	LFCLK_Config();
	InitPeriph();
	RTC1_Config();

	// BLE services init
    device_manager_init(false);
	db_discovery_init(); 
    gap_params_init();
    advertising_init();
    services_init();
	conn_params_init(); 
	
	DispatcherInit();
	
	SSD1306_Init();
	SSD1306_ReDraw();
	SSD1306_setFont(font5x8);
	
	UpdateCorrectionCounter();
	AlarmUpdate();

	SoundBeep(2000, 500);

	twi_master_init();
	BMP180_Init();
	LSM303_Init(); 

	SSD1306_Clear(); 
	SSD1306_Brightness(20);

	SetStartScreen();
	
	LogAddInfo("SYSstrt", 0, 15);

	nrf_delay_ms(500); 

	//if(nrf_gpio_pin_read(buttonsNum[0]) != 0) //0 - menu
		//NVIC_SystemReset();  

	//uint32_t err_code;
	/*err_code = ble_advertising_start(BLE_ADV_MODE_FAST);
	 APP_ERROR_CHECK(err_code);
	 LogAddInfo("BT On", LOG_INFO, 0);*/

	extern const unsigned char SevenSegNumFont[];
	extern const unsigned char fontSTD_numSevenSegNumFontPlusPlus[];
	 
	 
//	while(1)
//	{
//		static uint8_t num = 0;
//		SSD1306_Clear();
//		SSD1306_Pixel(80, 60, WHITE);
//		SSD1306_setFont((uint8_t*) fontSTD_numSevenSegNumFontPlusPlus);
//		SSD1306_Num(0, 0, num);
//		//SSD1306_Char(0,0,num+'0');
//		SSD1306_setFont((uint8_t*) font5x8);
//		SSD1306_NumWDesc(32, 110, ":", num);

//		SSD1306_setFont((uint8_t*) font3x5);
//		SSD1306_Num(32, 100, num);
//		SSD1306_String(10, 90, "ABC0123");

//		extern const unsigned char fontSTD_swiss721_outline[];
//		SSD1306_setFont((uint8_t*) fontSTD_swiss721_outline);
//		if(num % 20 > 10)
//			SSD1306_MultChar(0, 55, '0', 2);
//		else
//			SSD1306_Char(0, 55, '0');

//			SSD1306_MultChar(20, 55, (num % 20 > 10)?'0':'1', 1);

//		SSD1306_ReDraw();
//		if(++num >= 100)
//			num = 0;

//		nrf_delay_ms(100);

//	}	
	
	UpdateSwitchModes(1);  
	
	DispatcherAddEvnt(DISP_TYPE_REPEATED, DISP_100ms, Count100ms, 0);
	DispatcherAddEvnt(DISP_TYPE_REPEATED, DISP_1s, Count1s, 0);

	while(1)
	{
		UI_Handler();

		if(Evnt_WakeUp == false)
		{
			uint32_t err_code = sd_app_evt_wait();
			APP_ERROR_CHECK(err_code);
		}
	}
}
