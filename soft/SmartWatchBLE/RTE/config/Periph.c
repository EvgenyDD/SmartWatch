/* Includes ------------------------------------------------------------------*/
#include "periph.h"
#include "hardware.h"

#include "nrf_soc.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint8_t monthMass[] = "000JanFebMarAprMayJunJulAugSepOctNovDec";

/* Extern variables ----------------------------------------------------------*/
extern volatile uint32_t RTCCounter;
extern const uint8_t COMPILE_TIME[];
extern const uint8_t COMPILE_DATE[];
  
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
 * Function Name  : InitPeriph
 * Description    : 
 *******************************************************************************/
void InitPeriph() 
{
	/* Get System Time and Date from compiler current date */
	RTC_TimeDateTypeDef temp;
	temp.date = 10 * ((COMPILE_DATE[4] != 32) ? (COMPILE_DATE[4] - '0') : 0) + COMPILE_DATE[5] - '0';

	for(uint8_t i = 1; i < 13; i++)
		if(COMPILE_DATE[0] == monthMass[i * 3] && COMPILE_DATE[1] == monthMass[i * 3 + 1] && COMPILE_DATE[2] == monthMass[i * 3 + 2])
		{ 
			temp.month = i;  
			break;
		}

	temp.year = 1000 * (COMPILE_DATE[7] - '0') + 100 * (COMPILE_DATE[8] - '0') + 10 * (COMPILE_DATE[9] - '0') + (COMPILE_DATE[10] - '0');
	temp.hour = (COMPILE_TIME[0] - '0') * 10 + (COMPILE_TIME[1] - '0');
	temp.minute = (COMPILE_TIME[3] - '0') * 10 + COMPILE_TIME[4] - '0';
	temp.second = (COMPILE_TIME[6] - '0') * 10 + COMPILE_TIME[7] - '0';

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

	nrf_gpio_cfg_input(VIN_SENSE, (nrf_gpio_pin_pull_t) GPIO_PIN_CNF_PULL_Disabled);
	nrf_gpio_cfg_input(CHRG_SENSE, (nrf_gpio_pin_pull_t) GPIO_PIN_CNF_PULL_Pullup);
	nrf_gpio_cfg_input(CHRG_STBY, (nrf_gpio_pin_pull_t) GPIO_PIN_CNF_PULL_Pullup);
	nrf_gpio_cfg_input(LSM303_INT1, (nrf_gpio_pin_pull_t) GPIO_PIN_CNF_PULL_Disabled);
	nrf_gpio_cfg_input(LSM303_INT2, (nrf_gpio_pin_pull_t) GPIO_PIN_CNF_PULL_Disabled);
	nrf_gpio_cfg_input(LSM303_DRDY, (nrf_gpio_pin_pull_t) GPIO_PIN_CNF_PULL_Disabled);

	nrf_gpio_cfg_output(RAIL_POWER);
	nrf_gpio_pin_set(RAIL_POWER);
	nrf_gpio_cfg_output(VIBRO);

	nrf_delay_ms(50);

	//NRF_GPIO->PIN_CNF[20] |= (GPIO_PIN_CNF_SENSE_Low << GPIO_PIN_CNF_SENSE_Pos);
	NRF_GPIOTE->INTENSET = GPIOTE_INTENSET_PORT_Msk;
	//NVIC_EnableIRQ(GPIOTE_IRQn);  
	NVIC_SetPriority(GPIOTE_IRQn, 1);
	uint32_t err_code = sd_nvic_EnableIRQ(GPIOTE_IRQn);

	//nrf_gpio_cfg_input(4, NRF_GPIO_PIN_PULLUP);
	// Start 32 MHz crystal oscillator
	NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
	//NRF_CLOCK->TASKS_HFCLKSTART = 1;

	// Wait for the external oscillator to start up
	//while (NRF_CLOCK->EVENTS_HFCLKSTARTED == 0);

	NRF_POWER->TASKS_CONSTLAT = 0;
	NRF_POWER->TASKS_LOWPWR = 1;

	/* Disable Reset (by SWDIO pin) if pressing BTN_0 at startup */
	//NRF_POWER->RESET = (nrf_gpio_pin_read(BTN_1) == 1)?(1UL):(1UL);//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	NRF_POWER->RESET = 1; //reset enabled
	//NRF_POWER->RESET = 0; //reset disabled

	/* Watchdog */
	NRF_WDT->CONFIG = (WDT_CONFIG_HALT_Pause << WDT_CONFIG_HALT_Pos) | (WDT_CONFIG_SLEEP_Run << WDT_CONFIG_SLEEP_Pos);
	NRF_WDT->CRV = 3 * 32768; //DOG: 3 sek. timout
	NRF_WDT->RREN |= WDT_RREN_RR0_Msk; //Enable reload register 0
	NRF_WDT->TASKS_START = 1;
}

//Function for configuring the RTC with TICK to 100Hz and COMPARE0 to 10 sec.
void RTC1_Config(void)
{
	NVIC_EnableIRQ (RTC1_IRQn); // Enable Interrupt for the RTC in the core.
	NRF_RTC1->PRESCALER = 3276; //+1                    // Set prescaler to a TICK of RTC_FREQUENCY.
	//NRF_RTC0->CC[0]     = COMPARE_RTCCounterTIME * RTC_FREQUENCY;  // Compare0 after approx COMPARE_RTCCounterTIME seconds.

	// Enable TICK event and TICK interrupt:
	NRF_RTC1->EVTENSET = RTC_EVTENSET_TICK_Msk;
	NRF_RTC1->INTENSET = RTC_INTENSET_TICK_Msk;

	// Enable COMPARE0 event and COMPARE0 interrupt:
	//NRF_RTC0->EVTENSET = RTC_EVTENSET_COMPARE0_Msk;
	//NRF_RTC0->INTENSET = RTC_INTENSET_COMPARE0_Msk;

	NVIC_SetPriority(RTC1_IRQn, 3);

	NRF_RTC1->TASKS_START = 1;
}

void LFCLK_Config()
{
	NRF_CLOCK->LFCLKSRC = (CLOCK_LFCLKSRC_SRC_Xtal << CLOCK_LFCLKSRC_SRC_Pos);
	NRF_CLOCK->EVENTS_LFCLKSTARTED = 0;
	NRF_CLOCK->TASKS_LFCLKSTART = 1;

	while(NRF_CLOCK->EVENTS_LFCLKSTARTED == 0)
		;

	NRF_CLOCK->EVENTS_LFCLKSTARTED = 0;
}
