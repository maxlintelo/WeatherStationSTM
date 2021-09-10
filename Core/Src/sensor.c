#include "sensor.h"

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

void initSensor(I2C_HandleTypeDef* hi2c) {
	// Send 0xFE to 0x80 (Reset)
	HAL_I2C_Master_Transmit(hi2c, SLAVE_ADDRESS, &uint8_commands[0], 1, 100);
	// Wait for transmission
	HAL_Delay(40);
}
