/* Includes ------------------------------------------------------------------*/
#include "twi_master.h"
#include "LSM303.h"
#include "nrf.h"


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
*******************************************************************************/
bool LSM303_Init()
{
    LSM303_Write(0x20, 0x27);
    LSM303_Write(0x20, 0x4F);

    LSM303_Write(0x02, 0x00);
    LSM303_Write(0x01, 0x20);
#if 0
LSM303_Write(0x22, 0x40); //AOI1 interrupt generation is routed to INT1 pin

LSM303_Write(0x23, 0x80);// FS = ±2g low power mode with BDU bit enabled.

LSM303_Write(0x24, 0x0C);/*Interrupt signal on INT1 pin is latched with D4D_INT1 bit enabled.
If there is an interrupt from AOI1, INT1 pin will go high from low and stay high. Reading the INT1_SRC(31h) register will clear
the interrupt signal on INT1 pin.*/

LSM303_Write(0x32, 0x20); /* Threshold = 32LSBs * 15.625mg/LSB = 500mg. This corresponds to 30
degrees of tilt (=asin(0.5)) cone zone around the vertical gravity vector. */


LSM303_Write(0x33, 0x02); /* Duration = 10LSBs * (1/25Hz) = 0.4s. 1LSB = 1/ODR = 40mS. If the
X or Y axis enters the cone zone1 or cone zone2 for longer than 0.4s duration, then the interrupt will
be generated. Duration = 0 means that the interrupt will be generated immediately. */

LSM303_Write(0x30, 0x7F);  // 6D movement detection with Z axis disabled and YUPE, YDOWNE, XUPE and XDOWNE bits enabled.

/*LSM303_Write(0x38, 0x10); //single clck on z axis
LSM303_Write(0x39, 0x10); //single clck
LSM303_Write(0x3A, 0x02); //threshold*/
#endif
//LSM303_Write(0x25, 0xE0);


    return 0;
}

/*******************************************************************************
* Function Name  :
* Description    :
*******************************************************************************/
bool LSM303_On()
{
    return LSM303_Init();
}


bool LSM303_Off()
{
    //LSM303_Write(0x20, 0);
    LSM303_Write(0x02, (1<<1)); //put magnetometer in deep sleep
    //LSM303_Write(0x01, 0);

    return 0;
}



/*******************************************************************************
* Function Name  :
* Description    :
* Input		     :
*******************************************************************************/
bool LSM303_GetAccelData(int mass[])
{
    uint8_t temp[6] = {0}, status;
    status = LSM303_Read(0x28, &temp[0], 6);

    mass[XAxis] = (int16_t)(temp[0] | (temp[1] << 8)) >> 4;
    mass[YAxis] = (int16_t)(temp[2] | (temp[3] << 8)) >> 4;
    mass[ZAxis] = (int16_t)(temp[4] | (temp[5] << 8)) >> 4;
    return status;
}


/*******************************************************************************
* Function Name  :
* Description    :
* Input		     :
*******************************************************************************/
bool LSM303_GetMagnetData(int mass[])
{
    uint8_t temp[6] = {0}, status;
    status = LSM303_Read(0x03, &temp[0], 6);

    mass[XAxis] = (int16_t)(temp[1] | (temp[0] << 8));
    mass[YAxis] = (int16_t)(temp[3] | (temp[2] << 8));
    mass[ZAxis] = (int16_t)(temp[5] | (temp[4] << 8));
    return status;
}


/*******************************************************************************
* Function Name  :
* Description    :
* Input		     :
*******************************************************************************/
bool LSM303_Write(uint8_t reg_addr, uint8_t value)
{
    uint8_t data[2];

    data[0] = reg_addr;
    data[1] = value;

    return twi_master_transfer((reg_addr < 0x20)?LSM303_ADDR_M:LSM303_ADDR_A,
                               &data[0], 2, TWI_ISSUE_STOP);
}


/*******************************************************************************
* Function Name  : LSM303_Read
* Description    :
* Input		     :
*******************************************************************************/
bool LSM303_Read(uint8_t reg_addr, uint8_t * destination, uint8_t number_of_bytes)
{
    bool transferOk;
    if(reg_addr < 0x20)
    {
        transferOk  = twi_master_transfer(LSM303_ADDR_M, &reg_addr, 1, TWI_DONT_ISSUE_STOP);
        transferOk &= twi_master_transfer(LSM303_ADDR_M|1, destination, number_of_bytes, TWI_ISSUE_STOP);
    }
    else
    {
        if(number_of_bytes > 1) BitSet(reg_addr, 7);
        transferOk  = twi_master_transfer(LSM303_ADDR_A, &reg_addr, 1, TWI_DONT_ISSUE_STOP);
        transferOk &= twi_master_transfer(LSM303_ADDR_A|1, destination, number_of_bytes, TWI_ISSUE_STOP);
    }
    return transferOk;
}
