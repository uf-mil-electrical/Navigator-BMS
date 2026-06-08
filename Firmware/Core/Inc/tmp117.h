#ifndef TMP117_H_
#define TMP117_H_

#include "main.h"
#include "stm32g4xx_hal_def.h"
#include "stm32g4xx_hal_i2c.h"
#include <stdint.h>

#define TMP117_I2C_ADDRESS (0b01001000 << 1)
#define TMP117_I2C_HANDLE hi2c1

#define TMP117_MODE 0b00
#define TMP117_CONVERSION_CYCLE 0b000
#define TMP117_AVERAGE 0b00
#define TMP117_TNA 0b0
#define TMP117_POLARITY 0b0
#define TMP117_DRALERT 0b0
#define TMP117_SRST 0b0

typedef struct 
{
    I2C_HandleTypeDef * I2Chandle;
    uint16_t config;
    int16_t raw_temp;
    float temp;

    HAL_StatusTypeDef hal;
}TMP117;
void TMP117_Config(TMP117 * tmp);
HAL_StatusTypeDef TMP117_Initialize(TMP117 * tmp, I2C_HandleTypeDef * I2Chandle);
void readTMP117(TMP117 * tmp);

#endif