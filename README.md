# i2cStat
**i2cStat** — это инструмент для мониторинга системных ресурсов (загрузка CPU, использование RAM, скорость сети) с выводом информации на LCD-дисплей 1602, подключенный через интерфейс I2C. Проект написан на языке C и предназначен для работы на Linux-системах.

**Пример Дисплея:**
<pre>
╭────────────────╮
│CPU:45%    1234D│
│RAM:67%    5678U│
╰────────────────╯

После запуска программы на LCD-дисплее будет отображаться:
  Первая строка: Загрузка CPU и скорость приема данных (Килобайт/сек).
  Вторая строка: Использование RAM и скорость передачи данных (Килобайт/сек).
Данные обновляются каждую секунду.
</pre>

## Установка и использование

### Требования

- Linux-система (тестировалось на x86 архитектуре).
- LCD-дисплей 1602 с поддержкой I2C.
- Установленные библиотеки для работы с I2C (`i2c-tools`).
- Компилятор GCC.

### Установка

1. **Установите пакет i2c-tools. После установки загрузите i2c-dev модуль ядра.**
   ```bash
   sudo pacman -S i2c-tools
   sudo modprobe i2c-dev

  ```bash
  git clone https://github.com/isupovandrey/i2cStat.git
  cd i2cStat
  sudo ./i2cStat
```

## Обратная связь

Если у вас есть вопросы, предложения или вы хотите оставить отзыв о проекте, пожалуйста:
- Создайте [Issue](https://github.com/isupovandrey/i2cStat/issues) для обсуждения проблем или идей.
- Перейдите в [Discussions](https://github.com/isupovandrey/i2cStat/discussions), чтобы задать вопрос или пообщаться с сообществом.

## Благодарности
Проект был разработан с использованием ИИ DeepSeek.



