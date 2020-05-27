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
const uint16_t NoteMass[12*5] =
{
	131, 139, 148, 156, 165, 175, 185, 196, 207, 220, 233, 247,
	262, 277, 294, 311, 330, 349, 370, 392, 415, 440, 466, 494,
	523, 554, 587, 622, 659, 698, 740, 784, 830, 880, 932, 988,
	1046,1109,1174,1244,1318,1397,1480,1568,1661,1720,1865,1975,
	2093,2217,2349,2489,2637,2794,2960,3136,3332,3440,3729,3951
};

/* Extern variables ----------------------------------------------------------*/
extern uint32_t soundCountDown;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
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
void SoundNote(uint16_t note, uint16_t octave, uint16_t length)
{
	if(octave * 12 + note > 12*5) 
		octave = note = 1;
	
	SoundBeep(NoteMass[octave * 12 + note], length);
}

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

	soundCountDown = tone*length/500;
	
    PWM_TIMER->PRESCALER = 3;
    PWM_TIMER->CC[3] = TIM_VAL(tone, 3);
	PWM_TIMER->EVENTS_COMPARE[3] = 1;
	PWM_TIMER->INTENSET = TIMER_INTENCLR_COMPARE3_Msk;
	return;
	
nrf_delay_ms(length);

    nrf_pwm_set_value(0, 0);
    PWM_TIMER->TASKS_STOP = 1;

    //NRF_CLOCK->TASKS_HFCLKSTOP = 1;
}
