/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef SOUND_H
#define SOUND_H

/* Includes ------------------------------------------------------------------*/
#include "nrf.h"
#include "nrf_pwm.h"


/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
enum Notes{C, Cd, D, Dd, E, F, Fd, G, Gd, A, B, H};

/* Exported macro ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void SoundBeep(uint16_t tone, uint16_t length);
void SoundNote(uint16_t note, uint16_t octave, uint16_t length);
//void set_frequency_and_duty_cycle(uint32_t frequency, uint32_t duty_cycle_percent);

#endif //SOUND_H
