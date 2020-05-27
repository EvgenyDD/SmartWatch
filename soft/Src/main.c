#include "nrf.h"
#include <math.h>

#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "nrf_pwm.h"
#include "twi_master.h"

#include "SSD1306.h"
#include "LSM303.h"
#include "BMP180.h"

#include "hardware.h"
#include "light.h"
#include "sound.h"
#include "timework.h"

#include "menu.h"
#include "UI.h"

char t[8] = __TIME__;

volatile uint32_t RTCCounter;
volatile uint16_t SysTickTimer;

volatile uint8_t buttonCounter=0;
volatile uint16_t buttonState=0;

volatile uint16_t xxxx=0;

extern ADCDataType Light, VBat;

volatile int ACCEL[3], MAGNET[3];

bool Evnt_Hour=false, Evnt_WakeUp=false, Evnt_Alarm=false;

char month[] = "000JanFebMarAprMayJunJulAugSepOctNovDec";
char cur_date[] = __DATE__;

bool needUpdate = false;
int j=100;

uint8_t needBeep = 0;

static void LFCLK_Config(void)
{
    NRF_CLOCK->LFCLKSRC            = (CLOCK_LFCLKSRC_SRC_Xtal << CLOCK_LFCLKSRC_SRC_Pos);
    NRF_CLOCK->EVENTS_LFCLKSTARTED = 0;
    NRF_CLOCK->TASKS_LFCLKSTART    = 1;

    while (NRF_CLOCK->EVENTS_LFCLKSTARTED == 0);

    NRF_CLOCK->EVENTS_LFCLKSTARTED = 0;
}


//Function for configuring the RTC with TICK to 100Hz and COMPARE0 to 10 sec.
static void RTC0_Config(void)
{
    NVIC_EnableIRQ(RTC0_IRQn);                                  // Enable Interrupt for the RTC in the core.
    NRF_RTC0->PRESCALER = 3276;                    // Set prescaler to a TICK of RTC_FREQUENCY.
    //NRF_RTC0->CC[0]     = COMPARE_RTCCounterTIME * RTC_FREQUENCY;  // Compare0 after approx COMPARE_RTCCounterTIME seconds.

    // Enable TICK event and TICK interrupt:
    NRF_RTC0->EVTENSET = RTC_EVTENSET_TICK_Msk;
    NRF_RTC0->INTENSET = RTC_INTENSET_TICK_Msk;

    // Enable COMPARE0 event and COMPARE0 interrupt:
    //NRF_RTC0->EVTENSET = RTC_EVTENSET_COMPARE0_Msk;
    //NRF_RTC0->INTENSET = RTC_INTENSET_COMPARE0_Msk;
}

void InitPeriph()
{
    /* Get System Time and Date from compiler current date */
    RTC_TimeDateTypeDef temp;
    temp.date = 10*((cur_date[4]!=32)?(cur_date[4]-0x30):0) + cur_date[5]-0x30;

    for(uint8_t i=1; i<13; i++)
        if(cur_date[0]==month[i*3] && cur_date[1]==month[i*3+1] && cur_date[2]==month[i*3+2])
        {
            temp.month = i;
            break;
        }
    temp.year   = 1000*(cur_date[7]-0x30) + 100*(cur_date[8]-0x30) +
                  10*(cur_date[9]-0x30) + (cur_date[10]-0x30);
    temp.hour   = (t[0]-0x30)*10+(t[1]-0x30);
    temp.minute = (t[3]-0x30)*10+t[4]-0x30;
    temp.second = (t[6]-0x30)*10+t[7]-0x30;

    RTCCounter = RTC_TimeDateToCnt(&temp) + 11; //11 is avg time of compiling&loading FW into chip

    /* GPIO Settings */
    nrf_gpio_cfg_sense_input(BTN_0, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);
    nrf_gpio_cfg_sense_input(BTN_1, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);
    nrf_gpio_cfg_sense_input(BTN_2, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);
    nrf_gpio_cfg_sense_input(BTN_3, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);
    nrf_gpio_cfg_sense_input(BTN_4, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);

    nrf_gpio_cfg_output(LED_RED);
    nrf_gpio_cfg_output(LED_WHT);
    nrf_gpio_cfg_output(LED_YLW);
    nrf_gpio_cfg_output(LED_BLU);

    nrf_gpio_cfg_input(VIN_SENSE,  GPIO_PIN_CNF_PULL_Disabled);
    nrf_gpio_cfg_input(CHRG_SENSE, GPIO_PIN_CNF_PULL_Pullup);
    nrf_gpio_cfg_input(CHRG_STBY,  GPIO_PIN_CNF_PULL_Pullup);
    nrf_gpio_cfg_input(LSM303_INT1, GPIO_PIN_CNF_PULL_Disabled);
    nrf_gpio_cfg_input(LSM303_INT2, GPIO_PIN_CNF_PULL_Disabled);
    nrf_gpio_cfg_input(LSM303_DRDY, GPIO_PIN_CNF_PULL_Disabled);

    nrf_gpio_cfg_output(RAIL_POWER);
    nrf_gpio_pin_set(RAIL_POWER);
    nrf_gpio_cfg_output(VIBRO);

    nrf_delay_ms(50);

    /* Disable Reset (by SWDIO pin) if pressing BTN_0 at startup */
    NRF_POWER->RESET = (nrf_gpio_pin_read(BTN_1) == 1)?(1UL):(1UL);//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    /*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

    //NRF_GPIO->PIN_CNF[20] |= (GPIO_PIN_CNF_SENSE_Low << GPIO_PIN_CNF_SENSE_Pos);

    NRF_GPIOTE->INTENSET = GPIOTE_INTENSET_PORT_Msk;
    NVIC_EnableIRQ(GPIOTE_IRQn);


    //nrf_gpio_cfg_input(4, NRF_GPIO_PIN_PULLUP);
    // Start 32 MHz crystal oscillator
	NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
	//NRF_CLOCK->TASKS_HFCLKSTART = 1;

	// Wait for the external oscillator to start up
	//while (NRF_CLOCK->EVENTS_HFCLKSTARTED == 0);

    NRF_POWER->TASKS_CONSTLAT = 0;
    NRF_POWER->TASKS_LOWPWR = 1;

    /* Watchdog */
    NRF_WDT->CONFIG = (WDT_CONFIG_HALT_Pause << WDT_CONFIG_HALT_Pos) | ( WDT_CONFIG_SLEEP_Run << WDT_CONFIG_SLEEP_Pos);
	NRF_WDT->CRV = 3*32768;   //ca 3 sek. timout
	NRF_WDT->RREN |= WDT_RREN_RR0_Msk;  //Enable reload register 0
	NRF_WDT->TASKS_START = 1;
}


extern MenuItem Game;

void ButtonProcess()
{
    #define DEBOUNCE        3
    #define SHORT_PRESS     4
    #define LONG_PRESS      9

    if(++buttonCounter == DEBOUNCE)
    {
        if(!nrf_gpio_pin_read(BTN_0)) BitSet(buttonState, 0);
        if(!nrf_gpio_pin_read(BTN_1)) BitSet(buttonState, 1);
        if(!nrf_gpio_pin_read(BTN_2)) BitSet(buttonState, 2);
        if(!nrf_gpio_pin_read(BTN_3)) BitSet(buttonState, 3);
        if(!nrf_gpio_pin_read(BTN_4)) BitSet(buttonState, 4);
        return;
    }
    if(buttonCounter >= SHORT_PRESS && buttonCounter < LONG_PRESS)
    {
        uint16_t temp = 0;
        if(!nrf_gpio_pin_read(BTN_0)) BitSet(temp, 0);
        if(!nrf_gpio_pin_read(BTN_1)) BitSet(temp, 1);
        if(!nrf_gpio_pin_read(BTN_2)) BitSet(temp, 2);
        if(!nrf_gpio_pin_read(BTN_3)) BitSet(temp, 3);
        if(!nrf_gpio_pin_read(BTN_4)) BitSet(temp, 4);

        if(temp == 0)
        {

            xxxx = buttonState;
            needBeep = 1;

            UIShortButton(buttonState);

            buttonState = 0;
            buttonCounter = 0;
        }

        return;
    }

    if(buttonCounter == LONG_PRESS)
    {
        buttonState = 0;
        if(!nrf_gpio_pin_read(BTN_0)) BitSet(buttonState, 5);
        if(!nrf_gpio_pin_read(BTN_1)) BitSet(buttonState, 6);
        if(!nrf_gpio_pin_read(BTN_2)) BitSet(buttonState, 7);
        if(!nrf_gpio_pin_read(BTN_3)) BitSet(buttonState, 8);
        if(!nrf_gpio_pin_read(BTN_4)) BitSet(buttonState, 9);

        if(buttonState != 0)
        {
            if(buttonState == 0x340) //all 4 right-side buttons are long pressed together
                NVIC_SystemReset();

            if(buttonState == 0x220)
                MenuChange(&Game);

            xxxx = buttonState;
            needBeep = 2;
            UILongButton(buttonState);
        }

        buttonState = 0;
        buttonCounter = 0;
    }
}

extern AlarmType Alarmes[10];

void RTC0_IRQHandler()
{
    if ((NRF_RTC0->EVENTS_TICK != 0) && ((NRF_RTC0->INTENSET & RTC_INTENSET_TICK_Msk) != 0))
    {
        NRF_RTC0->EVENTS_TICK = 0;

        SysTickTimer++; //        <=================

        if(buttonCounter) ButtonProcess();

        static uint8_t preCnt = 0;
        if(++preCnt >= 10)
        {
            preCnt = 0;
            RTCCounter++;

            /* Check Alarm every minute */
            if(RTCCounter%60 == 0)
            {
                RTC_TimeDateTypeDef temp;
                RTC_CntToTimeDate(RTCCounter, &temp);

                for(uint8_t i=0; i<10; i++)
                    if(Alarmes[i].active && Alarmes[i].hour == temp.hour && Alarmes[i].minute==temp.minute)
                        Evnt_Alarm = true;
            }


//
            if(j==255) NRF_WDT->RR[0] = 0x6E524635; //Reload watchdog register 0

            if(Evnt_WakeUp == true) needUpdate = true;
              //  MenuCallback();
                //UpdateDisplay();

            /* Every hour*/
            if(RTCCounter%3600 == 0)
            {
                RTC_TimeDateTypeDef tmp;
                RTC_CntToTimeDate(RTCCounter, &tmp);
                if(tmp.hour > 7)
                    Evnt_Hour = true;
            }
        }
    }
}


void GPIOTE_IRQHandler(void)
{
    // This handler will be run after wakeup from system ON (GPIO wakeup)
    if(NRF_GPIOTE->EVENTS_PORT)
    {
        NRF_GPIOTE->EVENTS_PORT = 0;

        if(!nrf_gpio_pin_read(BTN_0)) BitSet(buttonState, 0);
        if(!nrf_gpio_pin_read(BTN_1)) BitSet(buttonState, 1);
        if(!nrf_gpio_pin_read(BTN_2)) BitSet(buttonState, 2);
        if(!nrf_gpio_pin_read(BTN_3)) BitSet(buttonState, 3);
        if(!nrf_gpio_pin_read(BTN_4)) BitSet(buttonState, 4);


        Evnt_Alarm = false;
        //UIShortButton(state);

        //xxxx = state;

        //buttonCounter = 1;
        if(j==255) Evnt_WakeUp = true;
        else
        {
            buttonCounter = 1;
            j=40;
        }
    }
}


void Sleep()
{
    SSD1306_Off();
    LSM303_Off();
    NRF_TWI1->ENABLE = TWI_ENABLE_ENABLE_Disabled << TWI_ENABLE_ENABLE_Pos;
//NRF_WDT->TASKS_START = 0;

    while(Evnt_WakeUp == false &&
          Evnt_Hour   == false &&
          Evnt_Alarm  == false)
              __WFE();
    //NRF_WDT->TASKS_START = 1;
}

void EvntHandler()
{
    /* Buzz every hour - in daytime */
    if(Evnt_Hour == true)
    {
        nrf_gpio_pin_set(VIBRO);
        nrf_delay_ms(200);
        nrf_gpio_pin_clear(VIBRO);

        Evnt_Hour = false;
        Sleep();
    }

    Evnt_WakeUp = false;
}


void WakeUp()
{
    NRF_TWI1->ENABLE = TWI_ENABLE_ENABLE_Enabled << TWI_ENABLE_ENABLE_Pos;

    SSD1306_On();
    LSM303_On();

}




int main( void )
{
    InitPeriph();

    /* RTC 0 */
    LFCLK_Config();
    RTC0_Config();
    NRF_RTC0->TASKS_START = 1;

    SSD1306_Init();
    SSD1306_ReDraw();
    SSD1306_setFont(font5x8);

    SoundBeep(2000, 500);

    twi_master_init();

    LSM303_Init();
    BMP180_Init();

    SSD1306_Clear();
    //nrf_gpio_pin_clear(RAIL_POWER);
    SSD1306_Brightness(20);

    SetStartScreen();

    while( 1 )
    {
        NRF_WDT->RR[0] = 0x6E524635; //Reload watchdog register 0

        /* Update screen */
        MenuCallback(0);

        /* symbolic delay */
        nrf_delay_ms(10);

        /* device is charging */
        if(nrf_gpio_pin_read(VIN_SENSE))
        {
            j=0;
            nrf_gpio_pin_set(LED_RED);
        }
        else
            nrf_gpio_pin_clear(LED_RED);



        /* READ LSM303 */
        //LSM303_GetAccelData(ACCEL);
    //LSM303_GetMagnetData(MAGNET);

    /* DRAW LSM303 DATA */
    /*SSD1306_NumWDesc(0,0,"X:", ACCEL[XAxis]);
    SSD1306_NumWDesc(0,9,"Y:", ACCEL[YAxis]);
    SSD1306_NumWDesc(0,18,"Z:", ACCEL[ZAxis]);*/
    /*SSD1306_NumWDesc(40,0,"X:", MAGNET[XAxis]);
    SSD1306_NumWDesc(40,9,"Y:", MAGNET[YAxis]);
    SSD1306_NumWDesc(40,18,"Z:", MAGNET[ZAxis]);*/

           //     float xxx, yyy;
        //getTemperature(&xxx);
        //getPressure(&yyy);

        /*SSD1306_FloatWDesc(80,0,".", xxx, 2);
        SSD1306_FloatWDesc(80,9,":", yyy, 2);
        SSD1306_FloatWDesc(80,18,"*", pressureToAltitude(102320,yyy), 2);*/




       /* nrf_gpio_pin_set(LED_RED);
        nrf_delay_ms(DELAY);
        nrf_gpio_pin_set(LED_WHT);
        nrf_delay_ms(DELAY);
        nrf_gpio_pin_set(LED_YLW);
        nrf_delay_ms(DELAY);*/
       /* nrf_gpio_pin_set(LED_BLU);
        nrf_delay_ms(DELAY);*/


      /*  nrf_gpio_pin_clear(LED_RED);
        nrf_delay_ms(DELAY);
        nrf_gpio_pin_clear(LED_WHT);
        nrf_delay_ms(DELAY);
        nrf_gpio_pin_clear(LED_YLW);
        nrf_delay_ms(DELAY);*/
        /*nrf_gpio_pin_clear(LED_BLU);
        nrf_delay_ms(DELAY);*/

        /*if(nrf_gpio_pin_read(BTN_0)==0)
            {SSD1306_Brightness(0xff);nrf_gpio_pin_set(VIBRO);
            nrf_delay_ms(500);
            nrf_gpio_pin_clear(VIBRO);}
            else
                SSD1306_Brightness(0);*/


/*(nrf_gpio_pin_read(VIN_SENSE))?     SSD1306_RectFill(0, 100, 5, 105): SSD1306_Rect(0, 100, 5, 105);
(nrf_gpio_pin_read(CHRG_SENSE))?    SSD1306_RectFill(0, 90, 5, 95) :  SSD1306_Rect(0, 90, 5, 95);
(nrf_gpio_pin_read(CHRG_STBY))?     SSD1306_RectFill(0, 80, 5, 85) :  SSD1306_Rect(0, 80, 5, 85);
(nrf_gpio_pin_read(LSM303_INT1))?   SSD1306_RectFill(0, 70, 5, 75) :  SSD1306_Rect(0, 70, 5, 75);
(nrf_gpio_pin_read(LSM303_INT2))?   SSD1306_RectFill(0, 60, 5, 65) :  SSD1306_Rect(0, 60, 5, 65);
(nrf_gpio_pin_read(LSM303_DRDY))?   SSD1306_RectFill(0, 50, 5, 55) :  SSD1306_Rect(0, 50, 5, 55);*/



    if(needBeep==1) {SoundBeep(3000, 30); needBeep = 0;}
    if(needBeep==2) {SoundBeep(4000, 50); needBeep = 0;}

    if(Evnt_Alarm)
    {
        SoundBeep(4000, 500);
        nrf_delay_ms(500);
        j=50;
    }


/*
        (nrf_gpio_pin_read(LSM303_INT1))?   SSD1306_RectFill(0, 30, 5, 35) :  SSD1306_Rect(0, 30, 5, 35);
        (nrf_gpio_pin_read(LSM303_INT2))?   SSD1306_RectFill(0, 20, 5, 25) :  SSD1306_Rect(0, 20, 5, 25);
uint8_t ggg;
        if(!nrf_gpio_pin_read(BTN_0))
        {
            LSM303_Read(0x31, &ggg, 1);

        }

        SSD1306_NumWDesc(0,9,":", ggg);*/
//

//if(!nrf_gpio_pin_read(BTN_0)){
//LSM303_Read(0x39, &ggg, 1);
   //      SSD1306_NumWDesc(0,9,":", ggg);
//LSM303_Read(0x31, &ggg, 1);
  //       SSD1306_NumWDesc(0,0,":", ggg);
//LSM303_Read(0x3A, &ggg, 1);
    //     SSD1306_NumWDesc(28,0,":", ggg);

    //     SSD1306_NumWDesc(28,6,"-", blua);
//}
       // SSD1306_ReDraw();

        if(--j==0)
        {
            j=255;
            SSD1306_EndSplash();
            MenuChangeMain();
            Sleep();

            EvntHandler();
            WakeUp();
            j=40;
        }

    }
}


