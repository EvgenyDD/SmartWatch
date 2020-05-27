/* Includes ------------------------------------------------------------------*/
#include "twi_master.h"
#include "LSM303.h"
#include "nrf.h"
#include "nrf_delay.h"
            #include "periph.h"
#include "nrf_soc.h"			
			

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
//#define RTC_INT_ENA	sd_nvic_critical_region_exit(0);
//#define RTC_INT_DIS sd_nvic_critical_region_enter(0);
#define RTC_INT_DIS	sd_nvic_DisableIRQ(RTC1_IRQn);
#define RTC_INT_ENA sd_nvic_EnableIRQ(RTC1_IRQn);

/* Private variables ---------------------------------------------------------*/
uint8_t TWIBusy = 0;

/* Extern variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  :
* Description    :
*******************************************************************************/
bool LSM303_Init()
{		
    LSM303_Write(REG_CTRL_5, BOOT_MEM);
	nrf_delay_ms(20);
	LSM303_Write(REG_CTRL_5, BOOT_MEM);
	nrf_delay_ms(20);
	
    LSM303_Write(REG_CTRL_1, 0x4F); //100Hz low-power mode
	
                           
    LSM303_Write(REG_MR, 0x02);
    LSM303_Write(REG_CRB, 0x20);
	
	LSM303_Write(REG_CTRL_4, 0x80);// FullScale = ±2g low power mode with BDU bit enabled.
	
	LSM303_Write(REG_CTRL_2, /*FDS|*/HPCF1|HPCF2|HP_AOI1/*|HP_CLICK*/);
	
#if 1	
	
	LSM303_Write(REG_CTRL_3, I1_AOI1); //AOI1 interrupt generation is routed to INT1 pin

	
	LSM303_Write(REG_CTRL_5, LIR_INT1/*|D4D_INT1*//*|LIR_INT2*/);/*Interrupt signal on INT1 pin is latched with D4D_INT1 bit enabled.
	If there is an interrupt from AOI1, INT1 pin will go high from low and stay high.
		Reading the INT1_SRC(31h) register will clear
	the interrupt signal on INT1 pin.*/
	
	//LSM303_Write(REG_CTRL_6, I2_CLICK);

	LSM303_Write(REG_INT1_THS, 20); /* Threshold = 32LSBs * 15.625mg/LSB = 500mg. 
		This corresponds to 30
	degrees of tilt (=asin(0.5)) cone zone around the vertical gravity vector. */
	LSM303_Write(REG_REF, 0);     


	LSM303_Write(REG_INT1_DUR, 10); /* Duration = 10LSBs * (1/25Hz) = 0.4s. 1LSB = 1/ODR = 40mS. If the
	X or Y axis enters the cone zone1 or cone zone2 for longer than 0.4s duration, then the interrupt will
	be generated. Duration = 0 means that the interrupt will be generated immediately. */

	LSM303_Write(REG_INT1_CFG, /*D6|*//*ZH|ZL|YH|YL|XH|XL*/ZH|YH|XH);  /* 6D movement detection with Z axis disabled and YUPE, 
	YDOWNE, XUPE and XDOWNE bits enabled.   */
		

	/*LSM303_Write(REG_CLICK_CFG, 0x10); //single clck on z axis
	LSM303_Write(REG_CLICK_SRC, 0x10); //single clck
	LSM303_Write(REG_CLICK_THS, 0x02); //threshold*/
	//
#else

	#if 0
	/* single click detection */
		 LSM303_Write(REG_CTRL_5, 0);
		 LSM303_Write(REG_INT1_CFG,0);

	LSM303_Write(REG_CTRL_6, I2_CLICK);
	LSM303_Write(REG_CLICK_CFG, XS);
	//LSM303_Write(REG_CLICK_CFG, ZD);
	
	LSM303_Write(REG_CLICK_THS, 70);
	LSM303_Write(REG_TIME_LIM, 40);   //63 max
	
	LSM303_Write(REG_TIME_LAT, 10);   //63 max
	LSM303_Write(REG_TIME_WND, 0);   //63 max
	LSM303_Write(REG_CLICK_SRC, SRC_SCLICK);
	#endif
	

	
#endif

    return 0;
}


/*******************************************************************************
* Function Name  :
* Description    :
*******************************************************************************/
void LSM303_Init_SleepTracker(void)
{
	LSM303_Write(REG_CTRL_5, BOOT_MEM);
	nrf_delay_ms(20);
	
    LSM303_Write(REG_CTRL_1, 0x4F); //100Hz low-power mode
	
	LSM303_Write(REG_CTRL_2, /*FDS|*/HPCF1|HPCF2|HP_AOI1/*|HP_CLICK*/);
	LSM303_Write(REG_CTRL_3, I1_AOI1); //AOI1 interrupt generation is routed to INT1 pin
	LSM303_Write(REG_CTRL_5, LIR_INT1/*|D4D_INT1*//*|LIR_INT2*/);
	
	LSM303_Write(REG_INT1_THS, 20);
	LSM303_Write(REG_INT1_DUR, 10);
	LSM303_Write(REG_INT1_CFG, ZH|YH|XH);	
	
	LSM303_Write(REG_INT2_CFG, 0);	
}


 /*******************************************************************************
* Function Name  :
* Description    :
*******************************************************************************/
void LSM303_Init_ShadowGesture(void)
{
	LSM303_Write(REG_CTRL_5, BOOT_MEM);
	nrf_delay_ms(20);
	
    LSM303_Write(REG_CTRL_1, 0x4F); //100Hz low-power mode
	
	LSM303_Write(REG_CTRL_2, /*FDS|*/HPCF1|HPCF2|HP_AOI2/*|HP_CLICK*/);
	LSM303_Write(REG_CTRL_3, I1_AOI1); //AOI1 interrupt generation is routed to INT1 pin
	LSM303_Write(REG_CTRL_5, /*LIR_INT1|*/D4D_INT1/*|D4D_INT1*//*|LIR_INT2*/);
	LSM303_Write(REG_CTRL_6, I2_AOI2);

	
	LSM303_Write(REG_INT1_THS, 45);
	LSM303_Write(REG_INT1_DUR, 10);
	LSM303_Write(REG_INT1_CFG, AOI|D6|XL|XH);
	
	LSM303_Init_Pedometer();	
}


 /*******************************************************************************
* Function Name  :
* Description    :
*******************************************************************************/
void LSM303_Init_Pedometer()
{
	//+ reg2 + reg6
	LSM303_Write(REG_INT2_THS, 15);
	LSM303_Write(REG_INT2_DUR, 2);
	LSM303_Write(REG_INT2_CFG, /*XL|*/XH);	
}


/*******************************************************************************
* Function Name  :
* Description    :
*******************************************************************************/
void LSM303_On()
{
    //return LSM303_Init();
}



/*******************************************************************************
* Function Name  :
* Description    :
*******************************************************************************/
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
*******************************************************************************/
bool LSM303_GetMagnetData(int mass[])
{
	LSM303_Write(REG_MR, 0x00);
	nrf_delay_ms(10);
    uint8_t temp[6] = {0}, status;
    status = LSM303_Read(0x03, &temp[0], 6);
	LSM303_Write(REG_MR, 0x02);

    mass[XAxis] = (int16_t)(temp[1] | (temp[0] << 8));
    mass[YAxis] = (int16_t)(temp[3] | (temp[2] << 8));
    mass[ZAxis] = (int16_t)(temp[5] | (temp[4] << 8));
    return status;
}


/*******************************************************************************
* Function Name  :
* Description    :
*******************************************************************************/
bool LSM303_Write(uint8_t reg_addr, uint8_t value)
{
    uint8_t data[2];
	bool ret;

    data[0] = reg_addr;
    data[1] = value;

	RTC_INT_DIS;
    ret = twi_master_transfer((reg_addr < 0x20)?LSM303_ADDR_M:LSM303_ADDR_A,
                               &data[0], 2, TWI_ISSUE_STOP);
	RTC_INT_ENA;
	
	return ret;
}


/*******************************************************************************
* Function Name  : LSM303_Read
* Description    :
* Input		     :
*******************************************************************************/
bool LSM303_Read(uint8_t reg_addr, uint8_t * destination, uint8_t number_of_bytes)
{
    bool transferOk;
	
	//RTC_INT_DIS;
	if(TWIBusy) return 1;
	TWIBusy = 1;
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
	TWIBusy = 0;
	//RTC_INT_ENA;
	
    return transferOk;
}


/*******************************************************************************
* Function Name  :
* Description    :
*******************************************************************************/
uint8_t LSM303_ReadReg(uint8_t reg_addr)
{
    uint8_t * destination;
	uint8_t number_of_bytes=1;
	
	//RTC_INT_DIS;
	if(TWIBusy) return 1;
	TWIBusy = 1;
    if(reg_addr < 0x20)
    {
        twi_master_transfer(LSM303_ADDR_M, &reg_addr, 1, TWI_DONT_ISSUE_STOP);
        twi_master_transfer(LSM303_ADDR_M|1, destination, number_of_bytes, TWI_ISSUE_STOP);
    }
    else
    {
        if(number_of_bytes > 1) BitSet(reg_addr, 7);
        twi_master_transfer(LSM303_ADDR_A, &reg_addr, 1, TWI_DONT_ISSUE_STOP);
        twi_master_transfer(LSM303_ADDR_A|1, destination, number_of_bytes, TWI_ISSUE_STOP);
    }
	TWIBusy = 0;
	//RTC_INT_ENA;
	
	
    return *destination;
}
