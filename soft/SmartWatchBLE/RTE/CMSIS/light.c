/* Includes ------------------------------------------------------------------*/
#include "light.h"
#include "hardware.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint8_t channel;
ADCDataType Light, VBat;

/* Extern variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  :
* Description    :
* Input		 :
*******************************************************************************/

/*******************************************************************************
* Function Name  :
* Description    :
*******************************************************************************/
void ADCInit(uint8_t ADCChannel)
{
    // interrupt ADC
	NRF_ADC->INTENSET = (ADC_INTENSET_END_Disabled << ADC_INTENSET_END_Pos);						/*!< Interrupt enabled. */

	// config ADC
	NRF_ADC->CONFIG	= (ADC_CONFIG_EXTREFSEL_None << ADC_CONFIG_EXTREFSEL_Pos) /* Bits 17..16 : ADC external reference pin selection. */
                    | (ADCChannel << ADC_CONFIG_PSEL_Pos)					/*!< Use analog input 0 as analog input. */
                    | (ADC_CONFIG_REFSEL_VBG << ADC_CONFIG_REFSEL_Pos)							/*!< Use internal 1.2V bandgap voltage as reference for conversion. */
                    | (ADC_CONFIG_INPSEL_AnalogInputOneThirdPrescaling << ADC_CONFIG_INPSEL_Pos) /*!< Analog input specified by PSEL with no prescaling used as input for the conversion. */
                    | (ADC_CONFIG_RES_10bit << ADC_CONFIG_RES_Pos);									/*!< 10bit ADC resolution. */

	// enable ADC
	NRF_ADC->ENABLE = ADC_ENABLE_ENABLE_Enabled;					  													/* Bit 0 : ADC enable. */

	// start ADC conversion
	NRF_ADC->TASKS_START = 1;

	// wait for conversion to end
	while (!NRF_ADC->EVENTS_END){}

	NRF_ADC->EVENTS_END	= 0;

    if(ADCChannel == ADC_BAT)
    {
        VBat.result = NRF_ADC->RESULT;
        VBat.value = VBat.result;
    }
    else if(ADCChannel == ADC_LIGHT)
    {
        Light.result = NRF_ADC->RESULT;
        Light.value = Light.result;
        nrf_gpio_pin_set(RAIL_POWER);
    }

    //Use the STOP task to save current. Workaround for PAN_028 rev1.1 anomaly 1.
    NRF_ADC->TASKS_STOP = 1;

    /* Disable ADC*/
	NRF_ADC->ENABLE = ADC_ENABLE_ENABLE_Disabled;
}


/*******************************************************************************
* Function Name  :
* Description    :
*******************************************************************************/
void MeasureVBat()
{
    ADCInit(ADC_BAT);
}


/*******************************************************************************
* Function Name  :
* Description    :
*******************************************************************************/
void MeasureLight()
{
    nrf_gpio_pin_clear(RAIL_POWER);
    nrf_delay_ms(10);

    ADCInit(ADC_LIGHT);
}
