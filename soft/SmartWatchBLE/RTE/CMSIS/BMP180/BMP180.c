/* Includes ------------------------------------------------------------------*/
#include "BMP180.h"
#include "twi_master.h"
#include "string.h"
#include <math.h>

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
 int32_t _sensorID;
BMP180_mode_t _BMP180Mode;
BMP180_calib_data _BMP180_coeffs;

#define delay(x) nrf_delay_ms(x);

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

bool BMP180_Init()
{
    /*
      BMP180_MODE_ULTRALOWPOWER          = 0,
      BMP180_MODE_STANDARD               = 1,
      BMP180_MODE_HIGHRES                = 2,
      BMP180_MODE_ULTRAHIGHRES           = 3*/
    BMP180_mode_t mode = BMP180_MODE_ULTRAHIGHRES;

     /* Mode boundary check */
    if(mode > BMP180_MODE_ULTRAHIGHRES)
    {
    mode = BMP180_MODE_ULTRAHIGHRES;
    }

    /* Make sure we have the right device */
    uint8_t id;
    BMP180_Read(BMP180_REGISTER_CHIPID, &id, 1);
    
	if(id != 0x55)
    {
        return false;
    }

    /* Set the mode indicator */
    _BMP180Mode = mode;

    /* Coefficients need to be read once */
    readCoefficients();

    return 0;
}


void readCoefficients()
{
    BMP180_Read16S(BMP180_REGISTER_CAL_AC1, &_BMP180_coeffs.ac1);
    BMP180_Read16S(BMP180_REGISTER_CAL_AC2, &_BMP180_coeffs.ac2);
    BMP180_Read16S(BMP180_REGISTER_CAL_AC3, &_BMP180_coeffs.ac3);
    BMP180_Read16(BMP180_REGISTER_CAL_AC4, &_BMP180_coeffs.ac4);
    BMP180_Read16(BMP180_REGISTER_CAL_AC5, &_BMP180_coeffs.ac5);
    BMP180_Read16(BMP180_REGISTER_CAL_AC6, &_BMP180_coeffs.ac6);
    BMP180_Read16S(BMP180_REGISTER_CAL_B1, &_BMP180_coeffs.b1);
    BMP180_Read16S(BMP180_REGISTER_CAL_B2, &_BMP180_coeffs.b2);
    BMP180_Read16S(BMP180_REGISTER_CAL_MB, &_BMP180_coeffs.mb);
    BMP180_Read16S(BMP180_REGISTER_CAL_MC, &_BMP180_coeffs.mc);
    BMP180_Read16S(BMP180_REGISTER_CAL_MD, &_BMP180_coeffs.md);
}


bool BMP180_Read(BMP180_REG_t reg, uint8_t *dest, uint8_t amount)
{
    bool state;

    state  = twi_master_transfer(BMP180_ADDRESS, &reg, 1, TWI_DONT_ISSUE_STOP);
    state &= twi_master_transfer(BMP180_ADDRESS|1, dest, amount, TWI_ISSUE_STOP);

    return state;
}

void BMP180_Read16(BMP180_REG_t reg, uint16_t *dest)
{
    uint8_t buf[2];
    BMP180_Read(reg, buf, 2);
    *dest = (buf[0]<<8)|buf[1];
}

void BMP180_Read16S(BMP180_REG_t reg, int16_t *dest)
{
    uint16_t buf;
    BMP180_Read16(reg, &buf);
    *dest = (int16_t)buf;
}

bool BMP180_Write(uint8_t reg_addr, uint8_t value)
{
    uint8_t data[2];

    data[0] = reg_addr;
    data[1] = value;

    return twi_master_transfer(BMP180_ADDRESS, &data[0], 2, TWI_ISSUE_STOP);
}


uint16_t BMP180_GetPressure()
{
    float pressure=0;

    getTemperature(&pressure);

    return pressure;
}


static void readRawTemperature(int32_t *temperature)
{
    uint16_t t;
    BMP180_Write(BMP180_REGISTER_CONTROL, BMP180_CMD_READTEMP);
    delay(5);
    BMP180_Read16(BMP180_REGISTER_TEMPDATA, &t);
    *temperature = t;
}


static void readRawPressure(int32_t *pressure)
{
    uint8_t  p8;
    uint16_t p16;
    int32_t  p32;

    BMP180_Write(BMP180_REGISTER_CONTROL, BMP180_CMD_READPRESSURE + (_BMP180Mode << 6));
    switch(_BMP180Mode)
    {
      case BMP180_MODE_ULTRALOWPOWER:
        delay(5);
        break;
      case BMP180_MODE_STANDARD:
        delay(8);
        break;
      case BMP180_MODE_HIGHRES:
        delay(14);
        break;
      case BMP180_MODE_ULTRAHIGHRES:
      default:
        delay(26);
        break;
    }

    BMP180_Read16(BMP180_REGISTER_PRESSUREDATA, &p16);
    p32 = (uint32_t)p16 << 8;
    BMP180_Read((BMP180_REG_t)(BMP180_REGISTER_PRESSUREDATA+2), &p8, 1);
    p32 += p8;
    p32 >>= (8 - _BMP180Mode);

    *pressure = p32;
}


//Gets the compensated pressure level in Pa
void getPressure(float *pressure)
{
	int32_t  ut = 0, up = 0;
	float compp = 0;
	int32_t  x1, x2, b5, b6, x3, b3, p;
	uint32_t b4, b7;


	/* Get the raw pressure and temperature values */
	readRawTemperature(&ut);
	readRawPressure(&up);

	/* Temperature compensation */
	b5 = computeB5(ut);

	/* Pressure compensation */
	b6 = b5 - 4000;
	x1 = (_BMP180_coeffs.b2 * ((b6 * b6) >> 12)) >> 11;
	x2 = (_BMP180_coeffs.ac2 * b6) >> 11;
	x3 = x1 + x2;
	b3 = (((((int32_t) _BMP180_coeffs.ac1) * 4 + x3) << _BMP180Mode) + 2) >> 2;
	x1 = (_BMP180_coeffs.ac3 * b6) >> 13;
	x2 = (_BMP180_coeffs.b1 * ((b6 * b6) >> 12)) >> 16;
	x3 = ((x1 + x2) + 2) >> 2;
	b4 = (_BMP180_coeffs.ac4 * (uint32_t) (x3 + 32768)) >> 15;
	b7 = ((uint32_t) (up - b3) * (50000 >> _BMP180Mode));

	if (b7 < 0x80000000)
	{
	p = (b7 << 1) / b4;
	}
	else
	{
	p = (b7 / b4) << 1;
	}

	x1 = (p >> 8) * (p >> 8);
	x1 = (x1 * 3038) >> 16;
	x2 = (-7357 * p) >> 16;
	compp = p + ((x1 + x2 + 3791) >> 4);


	/* Assign compensated pressure value */
	*pressure = compp;
}

//Reads the temperatures in degrees Celsius
void getTemperature(float *temp)
{
	int32_t UT, B5;     // following ds convention
	float t;

	readRawTemperature(&UT);

	B5 = computeB5(UT);
	t = (B5+8) >> 4;
	t /= 10;

	*temp = t;
}

int32_t computeB5(int32_t ut)
{
	int32_t X1 = (ut - (int32_t)_BMP180_coeffs.ac6) * ((int32_t)_BMP180_coeffs.ac5) >> 15;
	int32_t X2 = ((int32_t)_BMP180_coeffs.mc << 11) / (X1+(int32_t)_BMP180_coeffs.md);
	return X1 + X2;
}











/**************************************************************************/
/*!
    Calculates the altitude (in meters) from the specified atmospheric
    pressure (in hPa), and sea-level pressure (in hPa).

    @param  seaLevel      Sea-level pressure in hPa
    @param  atmospheric   Atmospheric pressure in hPa
*/
/**************************************************************************/
float pressureToAltitude(float seaLevel, float atmospheric)
{
  // Equation taken from BMP180 datasheet (page 16):
  //  http://www.adafruit.com/datasheets/BST-BMP180-DS000-09.pdf

  // Note that using the equation from wikipedia can give bad results
  // at high altitude.  See this thread for more information:
  //  http://forums.adafruit.com/viewtopic.php?f=22&t=58064

  return 44330.0 * (1.0 - pow(atmospheric / seaLevel, 0.1903));
}

/**************************************************************************/
/*!
    Calculates the altitude (in meters) from the specified atmospheric
    pressure (in hPa), and sea-level pressure (in hPa).  Note that this
    function just calls the overload of pressureToAltitude which takes
    seaLevel and atmospheric pressure--temperature is ignored.  The original
    implementation of this function was based on calculations from Wikipedia
    which are not accurate at higher altitudes.  To keep compatibility with
    old code this function remains with the same interface, but it calls the
    more accurate calculation.

    @param  seaLevel      Sea-level pressure in hPa
    @param  atmospheric   Atmospheric pressure in hPa
    @param  temp          Temperature in degrees Celsius
*/
/**************************************************************************/
float pressureToAltitude2(float seaLevel, float atmospheric, float temp)
{
  return pressureToAltitude(seaLevel, atmospheric);
}

/**************************************************************************/
/*!
    Calculates the pressure at sea level (in hPa) from the specified altitude
    (in meters), and atmospheric pressure (in hPa).

    @param  altitude      Altitude in meters
    @param  atmospheric   Atmospheric pressure in hPa
*/
/**************************************************************************/
float seaLevelForAltitude(float altitude, float atmospheric)
{
  // Equation taken from BMP180 datasheet (page 17):
  //  http://www.adafruit.com/datasheets/BST-BMP180-DS000-09.pdf

  // Note that using the equation from wikipedia can give bad results
  // at high altitude.  See this thread for more information:
  //  http://forums.adafruit.com/viewtopic.php?f=22&t=58064

  return atmospheric / pow(1.0 - (altitude/44330.0), 5.255);
}

/**************************************************************************/
/*!
    Calculates the pressure at sea level (in hPa) from the specified altitude
    (in meters), and atmospheric pressure (in hPa).  Note that this
    function just calls the overload of seaLevelForAltitude which takes
    altitude and atmospheric pressure--temperature is ignored.  The original
    implementation of this function was based on calculations from Wikipedia
    which are not accurate at higher altitudes.  To keep compatibility with
    old code this function remains with the same interface, but it calls the
    more accurate calculation.

    @param  altitude      Altitude in meters
    @param  atmospheric   Atmospheric pressure in hPa
    @param  temp          Temperature in degrees Celsius
*/
/**************************************************************************/
float seaLevelForAltitude2(float altitude, float atmospheric, float temp)
{
  return seaLevelForAltitude(altitude, atmospheric);
}
