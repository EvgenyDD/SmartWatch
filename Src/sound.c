/* Includes ------------------------------------------------------------------*/
#include "sound.h"
#include "nrf.h"
#include "nrf_pwm.h"
#include "hardware.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
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
/*
void set_frequency_and_duty_cycle(uint32_t frequency, uint32_t duty_cycle_percent)
{
    //nrf_pwm_set_max_value((16000000 + (frequency / 2)) / frequency);
    nrf_pwm_set_value(0, (16000000 / frequency) * duty_cycle_percent / 100);
}
*/

void SoundBeep(uint16_t tone, uint16_t length)
{
    // Start the external 16 MHz clock for a more accurate PWM frequency
    //NRF_CLOCK->TASKS_HFCLKSTART = 1;

    nrf_pwm_config_t PWMConfig = PWM_DEFAULT_CONFIG;

    PWMConfig.mode             = PWM_MODE_LED_1000;
    PWMConfig.num_channels     = 1;
    PWMConfig.gpio_num[0]      = BUZZER;

    nrf_pwm_init(&PWMConfig);

    #define TIM_CHANNEL 0
    nrf_pwm_set_value(TIM_CHANNEL, 500);

    #define TIM_VAL(hz, presc) (16000000/((hz)*2*(1<<(presc))))

    PWM_TIMER->PRESCALER = 3;
    PWM_TIMER->CC[3] = TIM_VAL(tone, 3);

nrf_delay_ms(length);

    nrf_pwm_set_value(0, 0);
    PWM_TIMER->TASKS_STOP = 1;

    //NRF_CLOCK->TASKS_HFCLKSTOP = 1;
}
