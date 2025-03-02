#ifndef MAIN_H_
#define MAIN_H_

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h> // Для AT_EACCESS
#include <sys/sysinfo.h>
#include <sys/types.h> // Для mode_t
#include <time.h>
#include <unistd.h>

#include "lcd1602_i2c.h"

// Макросы для цветного вывода
#define Color_ERROR(text) "\033[31m" text "\033[0m"
#define Color_WARNING(text) "\033[33m" text "\033[0m"
#define Color_CRITICAL(text) "\033[1;31m" text "\033[0m"
#define Color_INFO(text) "\033[34m" text "\033[0m"
#define Color_SUCCESS(text) "\033[32m" text "\033[0m"
#define Color_DEBUG(text) "\033[35m" text "\033[0m"
#define Color_HIGHLIGHT(text) "\033[1m" text "\033[0m"

#define HIGH(x) (((x) >> 4) & 0x0F)
#define LOW(x) ((x) & 0x0F)

// Структура для хранения сетевой статистики
typedef struct {
  uint64_t rx_bytes; // Принято байт
  uint64_t tx_bytes; // Передано байт
} NetworkStats;

// Структура для хранения скорости сети
typedef struct {
  uint64_t rx_speed; // Скорость приёма (Килобайт/сек)
  uint64_t tx_speed; // Скорость передачи (Килобайт/сек)
} NetworkSpeed;

#endif