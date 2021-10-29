/*
 * sensor.h
 *
 *  Created on: 10 Sep 2021
 *      Author: Max
 */

#ifndef INC_SI7021_H_
#define INC_SI7021_H_

#include "stm32f0xx_hal.h"
/*
 * Get a reference to the I2C handle from main
 */
extern I2C_HandleTypeDef hi2c1;
/*
 * In documentation, the 7-bit slave address was 0b1000000,
 * this means that the 8-bit address is 0b10000000 or 0x80.
 */
extern const uint8_t SLAVE_ADDRESS;
/*
 * We need an array of commands, HAL_I2C_Master_Transmit
 * takes a pointer to a byte array to send data.
 * 0xFE -> Reset
 * 0xE3 -> Get Temp (Master Hold)
 * 0xE5 -> Get Humid (Master Hold)
 */
extern uint8_t uint8_commands[3];
/*
 * We need two byte arrays to store the incoming
 * data from the SI7021, one for temperature, and
 * one for humidity.
 */
extern uint8_t uint8_tempIncomingBytes[3];
extern uint8_t uint8_humidIncomingBytes[3];
/*
 * Now we need to convert these values to float
 * so we can calculate to actual temperature.
 */
extern float float_temp, float_humid;

void SI7021_Measure(float*, float*, I2C_HandleTypeDef*);
void SI7021_Init(I2C_HandleTypeDef*);

#endif /* INC_SI7021_H_ */
