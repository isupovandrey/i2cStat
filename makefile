# Компилятор и флаги
CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS = -lrt

# Исходные файлы
SRCS = main.c lcd1602_i2c.c

# Объектные файлы
OBJS = $(patsubst %.c, OBJ/%.o, $(SRCS))

# Имя исполняемого файла
TARGET = i2cStat

# Папка для объектных файлов
OBJ_DIR = OBJ

# Правило по умолчанию
all: $(TARGET)
	sudo ./$(TARGET) /dev/i2c-1 enp2s0

# Сборка исполняемого файла
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET) $(LDFLAGS)

# Правило для компиляции исходных файлов в объектные
OBJ/%.o: %.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Создание папки OBJ, если она не существует
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Очистка
clean:
	rm -f $(OBJS) $(TARGET)
	rm -rf $(OBJ_DIR)

# Пересборка
rebuild: clean all

# Псевдонимы
.PHONY: all clean rebuild