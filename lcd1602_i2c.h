#ifndef LCD1602_I2C_H
#define LCD1602_I2C_H

#include "main.h"

#define LCD1602_I2C_ADDR 0x27
#define LCD_BUFFER_SIZE 32

#define PCF8574_RS (1 << 0)
#define PCF8574_RW (1 << 1)
#define PCF8574_CS (1 << 2)
#define PCF8574_Backlight (1 << 3)
#define PCF8574_DB4 (1 << 4)
#define PCF8574_DB5 (1 << 5)
#define PCF8574_DB6 (1 << 6)
#define PCF8574_DB7 (1 << 7)

extern uint8_t lcd_buffer[2][16];

int LCD1602_Init(const char *i2c_device);
void LCD1602_Write();
void LCD1602_Print(uint8_t x, uint8_t y, char *str);
void LCD1602_ScanI2C();

#endif // LCD1602_I2C_H