#include "bq76920.h"
#include "stm32g4xx_hal.h"
#include "stm32g4xx_hal_gpio.h"
#include "stm32g4xx_hal_i2c.h"
#include <stdint.h>

uint8_t cellArray[4] = {0x0C, 0x0E, 0x10, 0x14};
void BQ76920_Initialize(BQ76920 *bq, I2C_HandleTypeDef *hi2c,
                        GPIO_TypeDef *batIntPort, uint16_t batIntPin,
                        GPIO_TypeDef *batBootPort, uint16_t batBootPin) {
  bq->I2Chandle = hi2c;
  bq->batIntPort = batIntPort;
  bq->batIntPin = batIntPin;
  bq->batBootPort = batBootPort;
  bq->batBootPin = batBootPin;
  for (int i = 0; i < 4; i++) {
    bq->rawCellVoltages[i] = 0;
    bq->CellVoltages[i] = 0.0f;
  }
  uint8_t txBuf;
  txBuf = CELL_BALANCE_ENABLE;

  HAL_GPIO_WritePin(bq->batBootPort, bq->batBootPin, GPIO_PIN_SET);
  HAL_Delay(10);
  HAL_GPIO_WritePin(bq->batBootPort, bq->batBootPin, GPIO_PIN_RESET);
  HAL_Delay(250);

  // Enables Cell Balancing with all Cells
  // HAL_I2C_Mem_Write(bq->I2Chandle, BQ76920_I2C_ADDRESS, 0x01,
  //                   I2C_MEMADD_SIZE_8BIT, &txBuf, 1, 100);
  
  txBuf = 0b00010000;
  HAL_I2C_Mem_Write(bq->I2Chandle, BQ76920_I2C_ADDRESS, 0x04,
                    I2C_MEMADD_SIZE_8BIT, &txBuf, 1, 100);
  
}

void readCells(BQ76920 *bq) {
  for (int i = 0; i < 4; i++) {
    uint8_t rxBuf[2];
    HAL_I2C_Mem_Read(bq->I2Chandle, BQ76920_I2C_ADDRESS, cellArray[i],
                     I2C_MEMADD_SIZE_8BIT, rxBuf, 2, 100);
    bq->rawCellVoltages[i] = ((rxBuf[0] & 0x3F) << 8) + rxBuf[1];
    bq->CellVoltages[i] = bq->rawCellVoltages[i] * 0.000380f;
  }
}

void setMosfet(uint8_t state) {
  uint8_t txBuf;
  if (state == 0) {

  } else {
  }
}