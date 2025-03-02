#include "lcd1602_i2c.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int lcd_fd;
uint8_t lcd_buffer[2][16];
uint8_t i2c_SendBuff[204];

static void LCD1602_SendNibble(uint8_t nibble, uint8_t rs) {
  uint8_t data = (nibble << 4) | (rs << PCF8574_RS) | PCF8574_Backlight;
  uint8_t packet[3] = {data, (data | PCF8574_CS), data};
  write(lcd_fd, packet, sizeof(packet));
}

static void LCD1602_SendByte(uint8_t byte, uint8_t rs) {
  LCD1602_SendNibble((byte >> 4) & 0x0F, rs);
  LCD1602_SendNibble(byte & 0x0F, rs);
}

int LCD1602_Init(const char *i2c_device) {
  if ((lcd_fd = open(i2c_device, O_RDWR)) < 0) {
    perror(Color_ERROR("Ошибка открытия I2C-шины"));
    return -1;
  }

  if (ioctl(lcd_fd, I2C_SLAVE, LCD1602_I2C_ADDR) < 0) {
    perror("Дисплей не найден");
    close(lcd_fd);
    return -1;
  }

  LCD1602_SendNibble(0x03, 0);
  usleep(4500);
  LCD1602_SendNibble(0x03, 0);
  usleep(150);
  LCD1602_SendNibble(0x03, 0);
  usleep(150);
  LCD1602_SendNibble(0x02, 0);
  usleep(150);

  LCD1602_SendByte(0x28, 0); // 4-битный режим, 2 строки, шрифт 5x8
  LCD1602_SendByte(0x0C, 0); // Включение дисплея, курсор и мигание выключены
  LCD1602_SendByte(0x06, 0); // Режим ввода: курсор двигается вправо

  for (uint8_t i = 0; i < 16; i++)
    lcd_buffer[0][i] = 0x20;
  for (uint8_t i = 0; i < 16; i++)
    lcd_buffer[1][i] = 0x20;

  return 0;
}

void LCD1602_Write() {
  uint8_t buf_pos = 0;
  uint8_t char_idx = 0;

  // Макрос для добавления команды в буфер
  #define AddCmd(cmd) { \
    i2c_SendBuff[(buf_pos * 6) + 0] = HIGH(cmd) << 4 | PCF8574_Backlight; \
    i2c_SendBuff[(buf_pos * 6) + 1] = HIGH(cmd) << 4 | PCF8574_CS | PCF8574_Backlight; \
    i2c_SendBuff[(buf_pos * 6) + 2] = HIGH(cmd) << 4 | PCF8574_Backlight; \
    i2c_SendBuff[(buf_pos * 6) + 3] = LOW(cmd) << 4 | PCF8574_Backlight; \
    i2c_SendBuff[(buf_pos * 6) + 4] = LOW(cmd) << 4 | PCF8574_CS | PCF8574_Backlight; \
    i2c_SendBuff[(buf_pos * 6) + 5] = LOW(cmd) << 4 | PCF8574_Backlight; \
  }

  // Макрос для добавления данных в буфер
  #define AddData(dat) { \
    i2c_SendBuff[(buf_pos * 6) + 0] = HIGH(dat) << 4 | PCF8574_Backlight | PCF8574_RS; \
    i2c_SendBuff[(buf_pos * 6) + 1] = HIGH(dat) << 4 | PCF8574_CS | PCF8574_Backlight | PCF8574_RS; \
    i2c_SendBuff[(buf_pos * 6) + 2] = HIGH(dat) << 4 | PCF8574_Backlight | PCF8574_RS; \
    i2c_SendBuff[(buf_pos * 6) + 3] = LOW(dat) << 4 | PCF8574_Backlight | PCF8574_RS; \
    i2c_SendBuff[(buf_pos * 6) + 4] = LOW(dat) << 4 | PCF8574_CS | PCF8574_Backlight | PCF8574_RS; \
    i2c_SendBuff[(buf_pos * 6) + 5] = LOW(dat) << 4 | PCF8574_Backlight | PCF8574_RS; \
  }

  // Установка курсора на начало первой строки
  AddCmd(0x80);
  buf_pos++;

  // Запись данных первой строки
  for (uint8_t i = 0; i < 16; i++) {
    AddData(lcd_buffer[0][char_idx]);
    buf_pos++;
    char_idx++;
  }

  // Установка курсора на начало второй строки
  AddCmd(0xC0);
  buf_pos++;

  // Запись данных второй строки
  char_idx = 0;
  for (uint8_t i = 0; i < 16; i++) {
    AddData(lcd_buffer[1][char_idx]);
    buf_pos++;
    char_idx++;
  }

  // Отправка данных на дисплей
  if (write(lcd_fd, i2c_SendBuff, sizeof(i2c_SendBuff)) < 0) {
    perror(Color_ERROR("Ошибка записи на дисплей"));
    exit(1);
  }
}

void LCD1602_Print(uint8_t x, uint8_t y, char *str) {
  if (y > 1 || x > 15)
    return; // Защита от выхода за пределы
  while (*str && x < 16) {
    lcd_buffer[y][x++] = *str++;
  }
}

// Ручная реализация чтения байта
int i2c_smbus_read_byte(int fd) {
  uint8_t buffer;
  if (read(fd, &buffer, 1) < 0) {
    return -1; // Ошибка чтения
  }
  return buffer;
}

void LCD1602_ScanI2C() {
  DIR *dir;
  struct dirent *ent;
  char i2c_devices[1024] = "";
  int first = 1;
  int lcd_found = 0; // Флаг для отслеживания, найден ли дисплей

  // Открываем директорию /sys/class/i2c-dev/
  if ((dir = opendir("/sys/class/i2c-dev/")) == NULL) {
    perror(Color_ERROR("Ошибка открытия директории /sys/class/i2c-dev/"));
    return;
  }

  // Собираем список всех подключенных шин I2C
  while ((ent = readdir(dir)) != NULL) {
    if (strncmp(ent->d_name, "i2c-", 4) == 0) {
      char i2c_device[64];
      snprintf(i2c_device, sizeof(i2c_device), "/dev/%.50s", ent->d_name);

      // Выводим каждую шину в формате "  - /dev/i2c-X"
      printf("  - %s\n", i2c_device);

      if (!first) {
        strcat(i2c_devices, " ");
      }
      strcat(i2c_devices, "'");
      strcat(i2c_devices, i2c_device);
      strcat(i2c_devices, "'");
      first = 0;
    }
  }

  closedir(dir);

  // Выводим сообщение о поиске дисплеев
  printf("LCD дисплей доступен на интерфейсах\n");

  // Повторно открываем директорию для поиска устройств
  if ((dir = opendir("/sys/class/i2c-dev/")) == NULL) {
    perror(Color_ERROR("Ошибка открытия директории /sys/class/i2c-dev/"));
    return;
  }

  // Перебираем все файлы в директории
  while ((ent = readdir(dir)) != NULL) {
    if (strncmp(ent->d_name, "i2c-", 4) == 0) {
      char i2c_device[64];
      snprintf(i2c_device, sizeof(i2c_device), "/dev/%.50s", ent->d_name);

      int fd = open(i2c_device, O_RDWR);
      if (fd < 0) {
        continue; // Если шина недоступна, пропускаем
      }

      // Попытка установить адрес устройства
      if (ioctl(fd, I2C_SLAVE, LCD1602_I2C_ADDR) < 0) {
        close(fd);
        continue; // Если не удалось установить адрес, пропускаем
      }

      // Попытка прочитать байт с устройства
      int result = i2c_smbus_read_byte(fd);
      if (result >= 0) {
        // Выводим шину с желтым цветом, если дисплей найден
        printf(Color_WARNING("  - %s\n"), i2c_device);
        lcd_found = 1; // Устанавливаем флаг, что дисплей найден
      }

      close(fd);
    }
  }

  closedir(dir);

  // Если дисплей не найден, выводим сообщение красным цветом
  if (!lcd_found) {
    printf(Color_ERROR("  - Lcd дисплеи не найдены (файлы /dev/i2c-* должны быть доступны для чтения пользователю)\n"));
  }
}