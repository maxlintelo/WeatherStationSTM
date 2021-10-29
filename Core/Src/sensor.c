#include "sensor.h"

//variables
I2C_HandleTypeDef *i2c_h;
int32_t t_fine;
int16_t t2, t3, p2, p3, p4, p5, p6, p7, p8, p9;
uint16_t t1, p1;
uint8_t _temperature_res, _pressure_oversampling,  _mode;

#define BMP280_I2CADDR	0xEC
#define	BMP280_CONTROL			0xF4
#define TEST_Pin GPIO_PIN_9

//data
#define	BMP280_PRESSUREDATA		0xF7
#define	BMP280_TEMPDATA			0xFA

//pressure
#define BMP280_ULTRALOWPOWER	1
#define BMP280_LOWPOWER			2
#define BMP280_STANDARD			3
#define BMP280_HIGHRES			4
#define BMP280_ULTRAHIGHRES		5

//BMP280 mode
#define BMP280_SLEEPMODE		0
#define BMP280_FORCEDMODE		1
#define BMP280_NORMALMODE		3


/*
 * In documentation, the 7-bit slave address was 0b1000000,
 * this means that the 8-bit address is 0b10000000 or 0x80.
 */
const uint8_t SLAVE_ADDRESS = 0x80;
/*
 * We need an array of commands, HAL_I2C_Master_Transmit
 * takes a pointer to a byte array to send data.
 * 0xFE -> Reset
 * 0xE3 -> Get Temp (Master Hold)
 * 0xE5 -> Get Humid (Master Hold)
 */
uint8_t uint8_commands[3]= { 0xFE, 0xE3, 0xE5 };
/*
 * We need two byte arrays to store the incoming
 * data from the SI7021, one for temperature, and
 * one for humidity.
 */
uint8_t uint8_tempIncomingBytes[3];
uint8_t uint8_humidIncomingBytes[3];
/*
 * Now we need to convert these values to float
 * so we can calculate to actual temperature.
 */
float float_temp, float_humid;

/**
  * @brief I2C1 Initialization Function
  * @param pTemp Pointer to final temperature float
  * @param pHumid Pointer to final humidity float
  * @param hi2c Pointer to I2C handle you want to use
  * @retval None
  */
void updateSensor(float* pTemp, float* pHumid, I2C_HandleTypeDef* hi2c) {
	// Send 0xE3 to 0x80 (Measure Temperature CMD)
	HAL_I2C_Master_Transmit(hi2c, SLAVE_ADDRESS, &uint8_commands[1], 1, 100);
	// Save the 3 next incoming bytes from 0x80
	HAL_I2C_Master_Receive(hi2c, SLAVE_ADDRESS, uint8_tempIncomingBytes, 3, 1000);
	// Combine byte[0] and byte[1] to make a float
	float_temp = (float)((uint8_tempIncomingBytes[0]<<8) | uint8_tempIncomingBytes[1]);
	// Calculation as shown in documentation
	*pTemp = (-46.85 + (175.72*(float_temp/65536)));
	// Delay
	HAL_Delay(50);

	// Send 0xE5 to 0x80 (Measure Humidity CMD)
	HAL_I2C_Master_Transmit(hi2c, SLAVE_ADDRESS, &uint8_commands[2] ,1, 100);
	// Save the 3 next incoming bytes from 0x80
	HAL_I2C_Master_Receive(hi2c, SLAVE_ADDRESS, uint8_humidIncomingBytes, 3, 1000);
	// Combine byte[0] and byte[1] to make a float
	float_humid = (float)((uint8_humidIncomingBytes[0]<<8) | uint8_humidIncomingBytes[1]);
	// Calculation as shown in documentation
	*pHumid = (-6+(125*(float_humid/65536)));
	// Delay
	HAL_Delay(50);
}


void BMP280_Write8(uint8_t address, uint8_t data){
	HAL_I2C_Mem_Write(i2c_h, BMP280_I2CADDR, address, 1, &data, 1, 10);
}


void initSensor(I2C_HandleTypeDef* hi2c){
	// Send 0xFE to 0x80 (Reset)
	HAL_I2C_Master_Transmit(hi2c, SLAVE_ADDRESS, &uint8_commands[0], 1, 100);
	// Wait for transmission
	HAL_Delay(40);
}

uint32_t BMP280_Read24(uint8_t addr){
	uint8_t tmp[3];
	HAL_I2C_Mem_Read(i2c_h, BMP280_I2CADDR, addr, 1, tmp, 3, 10);
	return ((tmp[0] << 16) | tmp[1] << 8 | tmp[2]);
}


uint8_t BMP280_Read8(uint8_t addr){
	uint8_t tmp = 0;
	HAL_I2C_Mem_Read(i2c_h, BMP280_I2CADDR, addr, 1, &tmp, 1, 10);
	return tmp;
}


float BMP280_ReadTemperature(void){
  int32_t var1, var2;

  if(_mode == BMP280_FORCEDMODE)
  {
	  uint8_t mode;
	  uint8_t ctrl = BMP280_Read8(BMP280_CONTROL);
	  ctrl &= ~(0x03);
	  ctrl |= BMP280_FORCEDMODE;
	  BMP280_Write8(BMP280_CONTROL, ctrl);

	  mode = BMP280_Read8(BMP280_CONTROL); 	// Read written mode
	  mode &= 0x03;							// Do not work without it...

	  if(mode == BMP280_FORCEDMODE)
	  {
		  while(1) // Wait for end of conversion
		  {
			  mode = BMP280_Read8(BMP280_CONTROL);
			  mode &= 0x03;
			  if(mode == BMP280_SLEEPMODE)
				  break;
		  }

		  int32_t adc_T = BMP280_Read24(BMP280_TEMPDATA);
		  adc_T >>= 4;

		  var1  = ((((adc_T>>3) - ((int32_t)t1 <<1))) *
				  ((int32_t)t2)) >> 11;

		  var2  = (((((adc_T>>4) - ((int32_t)t1)) *
				  ((adc_T>>4) - ((int32_t)t1))) >> 12) *
				  ((int32_t)t3)) >> 14;

		  t_fine = var1 + var2;

		  float T  = (t_fine * 5 + 128) >> 8;
		  return T/100;
	  }
  }

  return -99;
}


uint8_t BMP280_ReadTemperatureAndPressure(float *temperature, int32_t *pressure){
	int64_t var1, var2, p;

	// Must be done first to get the t_fine variable set up
	BMP280_ReadTemperature();

	if(*temperature == -99)
		return -1;

	int32_t adc_P = BMP280_Read24(BMP280_PRESSUREDATA);
	adc_P >>= 4;

	var1 = ((int64_t)t_fine) - 128000;
	var2 = var1 * var1 * (int64_t)p6;
	var2 = var2 + ((var1*(int64_t)p5)<<17);
	var2 = var2 + (((int64_t)p4)<<35);
	var1 = ((var1 * var1 * (int64_t)p3)>>8) +
			((var1 * (int64_t)p2)<<12);
	var1 = (((((int64_t)1)<<47)+var1))*((int64_t)p1)>>33;

	if (var1 == 0) {
		return 0;  // avoid exception caused by division by zero
	}
	p = 1048576 - adc_P;
	p = (((p<<31) - var2)*3125) / var1;
	var1 = (((int64_t)p9) * (p>>13) * (p>>13)) >> 25;
	var2 = (((int64_t)p8) * p) >> 19;

	p = ((p + var1 + var2) >> 8) + (((int64_t)p7)<<4);
	*pressure = (int32_t)p/256;

	return 0;
}
