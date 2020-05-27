/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef STRING_H
#define STRING_H

/* Includes ------------------------------------------------------------------*/
#include "nrf.h"
#include <stdint.h>

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
//int strlen(char *);
int strlenNum(char *pText, int begin);
void itoa_(int, char s[]);
void dtoa_(uint32_t n, char s[]);
void ftoa_(float, char str[], char precision);
void reverse(char s[]);
void strcat_(char first[], char second[]);
uint16_t strcatNum(char first[], char second[], int begin, int end);

float log10_(int v);
float pow_(float x, float y);
float log10_(int v);
float sqrt_(float x);
void ftoa_(float num, char str[], char precision);

void ClearBuffer(uint8_t *s, uint16_t end, uint8_t value);


#endif //STRING_H
