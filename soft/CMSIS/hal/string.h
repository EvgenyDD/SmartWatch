/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef STRING_H
#define STRING_H

/* Includes ------------------------------------------------------------------*/
#include "nrf.h"


/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
int strlen(char *);
void itoa_(int n, char s[]);
void ftoa_(float num, char str[], char precision);
void reverse(char s[]);
void strcat_(char first[], char second[]);
void strcatWRev(char first[], char second[]);
float log10_(int v);
float pow_(float x, float y);


#endif //STRING_H
