#pragma once
#include "bq76920.h"
#include "stm32g4xx_hal.h"
#include <stdint.h>

/*
    Register Map (8 Bit Registers)
    
    Configuration / Status Registers
    Register 0x00 (System Status) -> Holds flags for fault events such as OV, UV, short circuit, ect.
    Register 0x01 (Cell Balancing Enable) -> Holds enables for certian cells to be balanced with each other.
    Register 0x04 (System Control 1) -> Control register that handles ADC enable, thermistor enable, & shutdown.
    Register 0x05 (System Control 2) -> Control register that disables UV, OV, OCD, SCD delays, CC conversions, and charge / discharge mosfets.
    
    Voltage Sense Registers
    Register 0x0C (Cell 1 Voltage (13:8)) -> High 6 bits for the 14-bit cell 1 voltage measurement.
    Register 0x0C (Cell 1 Voltage (7:0)) -> Low 8 bits for the 14-bit cell 1 voltage measurement.
    Register 0x0C (Cell 2 Voltage (13:8)) -> High 6 bits for the 14-bit cell 2 voltage measurement.
    Register 0x0C (Cell 2 Voltage (7:0)) -> Low 8 bits for the 14-bit cell 2 voltage measurement.
    Register 0x0C (Cell 3 Voltage (13:8)) -> High 6 bits for the 14-bit cell 3 voltage measurement.
    Register 0x0C (Cell 3 Voltage (7:0)) -> Low 8 bits for the 14-bit cell 3 voltage measurement.
    Register 0x0C (Cell 4 Voltage (13:8)) -> High 6 bits for the 14-bit cell 4 voltage measurement.
    Register 0x0C (Cell 4 Voltage (7:0)) -> Low 8 bits for the 14-bit cell 4 voltage measurement.
*/
#define BQ76920_I2C_ADDRESS (0x18 << 1)
#define CELL_BALANCE_ENABLE 0b00001111
typedef struct 
{
    I2C_HandleTypeDef * I2Chandle;
    GPIO_TypeDef * batIntPort;
    uint16_t batIntPin;
    GPIO_TypeDef * batBootPort;
    uint16_t batBootPin;
    uint16_t rawCellVoltages[4];
    float CellVoltages[4];
} BQ76920;

void BQ76920_Initialize(BQ76920 * bq, I2C_HandleTypeDef * hi2c, GPIO_TypeDef * batIntPor, uint16_t batIntPin,  GPIO_TypeDef * batBootPort,
                        uint16_t batBootPin);
void readCells(BQ76920 * bq);
void setMosfet(uint8_t state);