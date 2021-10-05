/*
 * pressure.h
 *
 *  Created on: Oct 1, 2021
 *      Author: Max
 */

#ifndef INC_PRESSURE_H_
#define INC_PRESSURE_H_

#include "stm32f0xx_hal.h"

void delay_ms(uint32_t period_ms);
int8_t i2c_reg_write(uint8_t i2c_addr, uint8_t reg_addr, uint8_t *reg_data, uint16_t length);
int8_t i2c_reg_read(uint8_t i2c_addr, uint8_t reg_addr, uint8_t *reg_data, uint16_t length);
int8_t spi_reg_write(uint8_t cs, uint8_t reg_addr, uint8_t *reg_data, uint16_t length);
int8_t spi_reg_read(uint8_t cs, uint8_t reg_addr, uint8_t *reg_data, uint16_t length);
void print_rslt(const char api_name[], int8_t rslt);

#endif /* INC_PRESSURE_H_ */
