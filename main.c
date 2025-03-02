#include "main.h"
#include <stdint.h>
#include <time.h>

char strBuff[17]; // Буфер для строки (16 символов + нулевой терминатор)

NetworkStats get_network_stats(const char *interface);
NetworkSpeed calculate_network_speed(const char *interface);
void format_number_with_spaces(char *buffer, size_t buffer_size, uint64_t number);
void truncate_to_seven_chars(char *buffer);
int get_cpu_usage();
int get_ram_usage();
void print_available_interfaces();
int check_file_access(const char *filename);

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Доступные i2c шины:\n");
    LCD1602_ScanI2C();
    printf("\n");
    print_available_interfaces();
    printf("\n");
    printf("Использование программы:\n");
    printf("  sudo ./i2cStat <I2C-шина> <сетевой интерфейс>\n");
    printf("  Пример: sudo ./i2cStat /dev/i2c-1 enp2s0\n");
    return 0;
  }

  if (argc < 3) {
    fprintf(stderr, Color_ERROR("Ошибка: Не указан сетевой интерфейс.\n"));
    fprintf(stderr, "Использование: %s <i2c_device> <network_interface>\n", argv[0]);
    fprintf(stderr, "Пример: %s /dev/i2c-1 eth0\n", argv[0]);
    print_available_interfaces();
    return 1;
  }

  const char *files_to_check[] = {
    "/proc/net/dev",
    "/proc/stat",
    "/proc/meminfo",
    argv[1],
    NULL 
  };

  // Проверяем доступность всех файлов
  for (int i = 0; files_to_check[i] != NULL; i++) {
    if (check_file_access(files_to_check[i])) {
      return 1;
    }
  }

  const char *i2c_device = argv[1];
  char buffNetIn[strlen(argv[2]) + 2]; // +2: для '!' и '\0'
  sprintf(buffNetIn, "%s:", argv[2]);  
  const char *network_interface = buffNetIn;
  // const char *network_interface = argv[2];

  
  if (LCD1602_Init(i2c_device) < 0) {
    fprintf(stderr, Color_ERROR("Ошибка инициализации LCD\n"));
    return 1;
  }

  printf("Выбран LCD дисплей на шине \033[33m%s\033[0m, и сетевой интерфейс "
         "\033[33m%s\033[0m\n",
         i2c_device, network_interface);

  while (1) {
    int cpu_usage = get_cpu_usage();
    int ram_usage = get_ram_usage();

    NetworkSpeed speed = calculate_network_speed(network_interface);


    char rx_speed_str[16];
    char tx_speed_str[16];
    format_number_with_spaces(rx_speed_str, sizeof(rx_speed_str), speed.rx_speed);
    format_number_with_spaces(tx_speed_str, sizeof(tx_speed_str), speed.tx_speed);

    // Обрезаем строки скорости до 7 символов
    truncate_to_seven_chars(rx_speed_str);
    truncate_to_seven_chars(tx_speed_str);

    // Форматируем строки для дисплея (16 символов)
    snprintf(strBuff, sizeof(strBuff), "CPU:%02d%% %7sD", cpu_usage, rx_speed_str);
    LCD1602_Print(0, 0, strBuff);

    snprintf(strBuff, sizeof(strBuff), "RAM:%02d%% %7sU", ram_usage, tx_speed_str);
    LCD1602_Print(0, 1, strBuff);

    LCD1602_Write();
    sleep(1);
  }

  return 0;
}

NetworkStats get_network_stats(const char *interface) {
  FILE *file = fopen("/proc/net/dev", "r");
  if (!file) {
    perror(Color_ERROR("Ошибка открытия /proc/net/dev"));
    return (NetworkStats){0, 0};
  }

  char line[256];
  NetworkStats stats = {0, 0};

  while (fgets(line, sizeof(line), file)) {
    if (strstr(line, interface)) {
      // Формат: интерфейс: rx_bytes tx_bytes ...
      sscanf(line + strlen(interface) + 1, "%lu %*u %*u %*u %*u %*u %*u %*u %lu", (unsigned long *)&stats.rx_bytes, (unsigned long *)&stats.tx_bytes);
      break;
    }
  }

  fclose(file);
  return stats;
}

NetworkSpeed calculate_network_speed(const char *interface) {
  static uint64_t last_rx = 0, last_tx = 0;
  static time_t last_time = 0;

  NetworkStats stats = get_network_stats(interface);
  time_t now = time(NULL);

  NetworkSpeed speed = {0, 0};

  if (last_time != 0) {
    double time_diff = difftime(now, last_time);
    if (time_diff > 0) {
      speed.rx_speed =
          (stats.rx_bytes - last_rx) / 1024 / time_diff; // Килобайт/сек
      speed.tx_speed =
          (stats.tx_bytes - last_tx) / 1024 / time_diff; // Килобайт/сек
    }
  }

  last_rx = stats.rx_bytes;
  last_tx = stats.tx_bytes;
  last_time = now;

  return speed;
}

void format_number_with_spaces(char *buffer, size_t buffer_size,
                               uint64_t number) {
  if (number < 1000) {
    snprintf(buffer, buffer_size, "%lu",
             number); // Просто число, если меньше 1000
  } else if (number < 1000000) {
    snprintf(buffer, buffer_size, "%lu %03lu", number / 1000,
             number % 1000); // Тысячи
  } else {
    snprintf(buffer, buffer_size, "%lu %03lu %03lu", number / 1000000,
             (number % 1000000) / 1000, number % 1000); // Миллионы
  }
}

void truncate_to_seven_chars(char *buffer) {
  if (strlen(buffer) > 7) {
    buffer[7] = '\0'; // Обрезаем строку до 7 символов
  }
}

int get_cpu_usage() {
  FILE *file = fopen("/proc/stat", "r");
  if (!file)
    return -1;

  static unsigned long long last_total = 0, last_idle = 0;
  unsigned long long total = 0, idle = 0;

  char line[256];
  fgets(line, sizeof(line), file);
  fclose(file);

  unsigned long long user, nice, system, idle_time, iowait, irq, softirq, steal;
  sscanf(line, "cpu %llu %llu %llu %llu %llu %llu %llu %llu", &user, &nice,
         &system, &idle_time, &iowait, &irq, &softirq, &steal);

  total = user + nice + system + idle_time + iowait + irq + softirq + steal;
  idle = idle_time;

  unsigned long long total_diff = total - last_total;
  unsigned long long idle_diff = idle - last_idle;

  last_total = total;
  last_idle = idle;

  if (total_diff == 0)
    return 0;

  int usage = (int)(100.0f * (1.0f - (float)idle_diff / (float)total_diff));
  return (usage > 99) ? 99 : (usage < 0) ? 0 : usage;
}

int get_ram_usage() {
  FILE *file = fopen("/proc/meminfo", "r");
  if (!file)
    return -1;

  unsigned long long totalRAM = 0, freeRAM = 0, buffers = 0, cached = 0,
                     sReclaimable = 0, shmem = 0;

  char line[128];
  while (fgets(line, sizeof(line), file)) {
    if (strstr(line, "MemTotal:"))sscanf(line, "MemTotal: %llu kB", &totalRAM);
    else if (strstr(line, "MemFree:"))sscanf(line, "MemFree: %llu kB", &freeRAM);
    else if (strstr(line, "Buffers:"))sscanf(line, "Buffers: %llu kB", &buffers);
    else if (strstr(line, "Cached:"))sscanf(line, "Cached: %llu kB", &cached);
    else if (strstr(line, "SReclaimable:"))sscanf(line, "SReclaimable: %llu kB", &sReclaimable);
    else if (strstr(line, "Shmem:"))sscanf(line, "Shmem: %llu kB", &shmem);
  }
  fclose(file);

  if (totalRAM == 0)
    return 0;

  unsigned long long usedRAM = totalRAM - freeRAM - buffers - cached - sReclaimable + shmem;
  int usage = (int)(100.0f * ((float)usedRAM / (float)totalRAM));

  if (usage > 99)usage = 99;
  if (usage < 0)usage = 0;

  return usage;
}

// Функция для вывода доступных сетевых интерфейсов
void print_available_interfaces() {
  FILE *file = fopen("/proc/net/dev", "r");
  if (!file) {
    perror(Color_ERROR("Ошибка открытия /proc/net/dev"));
    return;
  }

  char line[256];
  printf("Доступные сетевые интерфейсы:\n");

  // Пропускаем первые две строки (заголовки)
  fgets(line, sizeof(line), file);
  fgets(line, sizeof(line), file);

  // Читаем и выводим интерфейсы
  while (fgets(line, sizeof(line), file)) {
    char interface[32];
    if (sscanf(line, "%31[^:]:", interface) == 1) {
      printf("  - \033[33m%s\033[0m\n", interface);
    }
  }

  fclose(file);
}

int check_file_access(const char *filename) {
  if (access(filename, R_OK | W_OK) == -1) { // Проверяем, доступен ли файл для чтения и записи
    fprintf(stderr, Color_ERROR("Ошибка: Файл '%s' недоступен для чтения.\n"), filename);
    fprintf(stderr, Color_HIGHLIGHT("Запустите программу с правами суперпользователя (sudo).\n"));
    return -1; // Возвращаем ошибку
  }
  return 0; // Файл доступен
}