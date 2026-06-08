#include "tmp117.h"
#include "stm32g4xx_hal_def.h"
#include "stm32g4xx_hal_i2c.h"
#include <stdint.h>

void TMP117_Config(TMP117 *tmp) {
  tmp->config = (TMP117_MODE << 10) + (TMP117_CONVERSION_CYCLE << 7) +
                (TMP117_AVERAGE << 5) + (TMP117_TNA << 4) +
                (TMP117_POLARITY << 3) + (TMP117_DRALERT << 2) +
                (TMP117_SRST << 1);
}

HAL_StatusTypeDef TMP117_Initialize(TMP117 *tmp, I2C_HandleTypeDef *I2Chandle) {
  tmp->I2Chandle = I2Chandle;
  TMP117_Config(tmp);

  //Checking Device ID
  uint8_t rxBuf[2];
  HAL_I2C_Mem_Read(tmp->I2Chandle, TMP117_I2C_ADDRESS, 0xF, I2C_MEMADD_SIZE_8BIT, rxBuf, 2, 100);
  uint16_t id = ((rxBuf[0] << 8) + rxBuf[1]);
  if(id != 0x117) {return HAL_ERROR;}

  // Configure Register
  uint8_t txBuf[2];
  txBuf[0] = (tmp->config << 8); txBuf[1] = (tmp->config & 0xFF);
  HAL_I2C_Mem_Write(tmp->I2Chandle, TMP117_I2C_ADDRESS, 0x1, I2C_MEMADD_SIZE_8BIT, txBuf,2,100);
  
  return HAL_OK;
}

void readTMP117(TMP117 * tmp)
{
    uint8_t rxBuf[2];
    HAL_I2C_Mem_Read(tmp->I2Chandle, TMP117_I2C_ADDRESS, 0x0, I2C_MEMADD_SIZE_8BIT, rxBuf, 2, 100);
    tmp->raw_temp = ((rxBuf[0] << 8) + rxBuf[1]);
    tmp->temp = tmp->raw_temp * 0.0078125;
}