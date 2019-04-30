/*
  Скетч к проекту "Домашняя метеостанция"
  Страница проекта (схемы, описания): https://alexgyver.ru/meteoclock/
  Исходники на GitHub: https://github.com/AlexGyver/MeteoClock
  Нравится, как написан и закомментирован код? Поддержи автора! https://alexgyver.ru/support_alex/
  Автор: AlexGyver Technologies, 2018
  http://AlexGyver.ru/
  Доработал Роман Новиков (с)НР
*/

/*
  Время и дата устанавливаются атвоматически при загрузке прошивки (такие как на компьютере)
  График всех величин за час и за сутки (усреднённые за каждый час)
  В модуле реального времени стоит батарейка, которая продолжает отсчёт времени после выключения/сброса питания
  Как настроить время на часах. У нас есть возможность автоматически установить время на время загрузки прошивки, поэтому:
	- Ставим настройку RESET_CLOCK на 1
  - Прошиваемся
  - Сразу ставим RESET_CLOCK 0
  - И прошиваемся ещё раз
  - Всё
*/

/* Версия 1.3
  - Исправлен прогноз погоды
*/

// ------------------------- НАСТРОЙКИ --------------------
#define RESET_CLOCK 0     // сброс часов на время загрузки прошивки (для модуля с несъёмной батарейкой). Не забудь поставить 0 и прошить ещё раз!
#define SENS_TIME 10000   // время обновления показаний сенсоров на экране, миллисекунд
#define LED_MODE 0        // тип RGB светодиода: 0 - главный катод, 1 - главный анод
#define LED_BRIGHT 255    // яркость светодиода СО2 (0 - 255)
#define BLUE_YELLOW 1     // жёлтый цвет вместо синего (1 да, 0 нет) но из за особенностей подключения жёлтый не такой яркий
#define DISP_MODE 1       // в правом верхнем углу отображать: 0 - год, 1 - день недели, 2 - секунды
#define WEEK_LANG 1       // язык дня недели: 0 - английский, 1 - русский (транслит)
#define DEBUG 0           // вывод на дисплей лог инициализации датчиков при запуске. Для дисплея 1602 не работает! Но дублируется через порт!
#define PRESSURE 1        // 0 - график давления, 1 - график прогноза дождя (вместо давления). Не забудь поправить пределы гроафика
#define CO2_SENSOR 1      // включить или выключить поддержку/вывод с датчика СО2 (1 вкл, 0 выкл)
#define DISPLAY_TYPE 1    // тип дисплея: 1 - 2004 (большой), 0 - 1602 (маленький)
#define DISPLAY_ADDR 0x27 // адрес платы дисплея: 0x27 или 0x3f. Если дисплей не работает - смени адрес! На самом дисплее адрес не указан

int MAX_ONDATA = 1 + 2 + 4 + 8 + 16 + 32 + 64 + 128; // максимальные показания графиков исходя из накопленных фактических (но в пределах лимитов) данных вместо указанных пределов, 0 - использовать фиксированные пределы (с)НР
int MIN_ONDATA = 1 + 2 + 4 + 8 + 16 + 32 + 64 + 128; // минимальные показания графиков исходя из накопленных фактических (но в пределах лимитов) данных вместо указанных пределов, 0 - использовать фиксированные пределы (с)НР
/* 1 - для графика температуры часовой, 2 - для графика температуры суточной (с)НР
   4 - для графика влажности часовой, 8 - для графика влажности суточной (с)НР
   16 - для графика давления часового, 32 - для графика давления суточного (с)НР
   64 - для графика СО2 часового, 128 - для графика СО2 суточного (с)НР
   для выборочных графиков значения нужно сложить (с)НР
   например: для изменения пределов у графиков суточной температуры и суточного СО2 складываем 2 + 128 и устанавливаем значение 130 (можно ставить сумму) (с)НР
*/

// пределы отображения для графиков
#define TEMP_MIN 15
#define TEMP_MAX 35
#define HUM_MIN 0
#define HUM_MAX 100
#define PRESS_MIN -100
#define PRESS_MAX 100
#define CO2_MIN 400
#define CO2_MAX 2000

// адрес BME280 жёстко задан в файле библиотеки Adafruit_BME280.h
// стоковый адрес был 0x77, у китайского модуля адрес 0x76.
// Так что если юзаете не библиотеку из архива - не забудьте поменять

// если дисплей не заводится - поменяйте адрес (строка 54)

// пины
#define MHZ_RX 2
#define MHZ_TX 3

#define LED_COM 7
#define LED_R 9
#define LED_G 6
#define LED_B 5

#define BTN_PIN 4

// библиотеки
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#if (DISPLAY_TYPE == 1)
LiquidCrystal_I2C lcd(DISPLAY_ADDR, 20, 4);
#else
LiquidCrystal_I2C lcd(DISPLAY_ADDR, 16, 2);
#endif

#include "RTClib.h"
RTC_DS3231 rtc;
DateTime now;

#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#define SEALEVELPRESSURE_HPA (1013.25)
Adafruit_BME280 bme;

#if (CO2_SENSOR == 1)
#include <MHZ19_uart.h>
MHZ19_uart mhz19;
#endif

#include <GyverTimer.h>
GTimer_ms sensorsTimer(SENS_TIME);
GTimer_ms drawSensorsTimer(SENS_TIME);
GTimer_ms clockTimer(500);
GTimer_ms hourPlotTimer((long)4 * 60 * 1000);       // 4 минуты
GTimer_ms dayPlotTimer((long)1.6 * 60 * 60 * 1000);   // 1.6 часа
GTimer_ms plotTimer(240000);
GTimer_ms predictTimer((long)10 * 60 * 1000);         // 10 минут

#include "GyverButton.h"
GButton button(BTN_PIN, LOW_PULL, NORM_OPEN);

int8_t hrs, mins, secs;
byte mode = 0;
/*
  0 часы и данные
  1 график температуры за час
  2 график температуры за сутки
  3 график влажности за час
  4 график влажности за сутки
  5 график давления за час
  6 график давления за сутки
  7 график углекислого за час
  8 график углекислого за сутки
*/
byte mode0scr = 0;
/* (с)НР
  0 - Крупно время
  1 - Крупно содержание СО2
  2 - Крупно температура
  3 - Крупно давление (не реализовано)
  4 - Крупно влажность (не реализовано)
*/
boolean bigDig = false; // true - цифры на главном экране на все 4 строки (для LCD 2004)

// переменные для вывода
float dispTemp;
byte dispHum;
int dispPres;
int dispCO2;
int dispRain;

// массивы графиков
int tempHour[15], tempDay[15];
int humHour[15], humDay[15];
int pressHour[15], pressDay[15];
int co2Hour[15], co2Day[15];
int delta;
uint32_t pressure_array[6];
uint32_t sumX, sumY, sumX2, sumXY;
float a, b;
byte time_array[6];

// символы
// график
//byte row8[8] = {0b11111,  0b11111,  0b11111,  0b11111,  0b11111,  0b11111,  0b11111,  0b11111}; // Есть под кодом 255 (с)НР
byte rowS[8] = {0b00000,  0b00000,  0b00000,  0b00000,  0b10001,  0b01010,  0b00100,  0b00000}; // стрелка вниз (с)НР
byte row7[8] = {0b00000,  0b11111,  0b11111,  0b11111,  0b11111,  0b11111,  0b11111,  0b11111};
byte row6[8] = {0b00000,  0b00000,  0b11111,  0b11111,  0b11111,  0b11111,  0b11111,  0b11111};
byte row5[8] = {0b00000,  0b00000,  0b00000,  0b11111,  0b11111,  0b11111,  0b11111,  0b11111};
byte row4[8] = {0b00000,  0b00000,  0b00000,  0b00000,  0b11111,  0b11111,  0b11111,  0b11111};
byte row3[8] = {0b00000,  0b00000,  0b00000,  0b00000,  0b00000,  0b11111,  0b11111,  0b11111};
byte row2[8] = {0b00000,  0b00000,  0b00000,  0b00000,  0b00000,  0b00000,  0b11111,  0b11111};
byte row1[8] = {0b00000,  0b00000,  0b00000,  0b00000,  0b00000,  0b00000,  0b00000,  0b11111};

// цифры двухстрочные
uint8_t LT[8] = {0b11111,  0b11111,  0b11111,  0b11111,  0b11111,  0b11111,  0b11111,  0b11111};   // для всех цифр
uint8_t UB[8] = {0b11111,  0b11111,  0b11111,  0b00000,  0b00000,  0b00000,  0b00000,  0b00000};   // для 7, 0
//uint8_t RT[8] = {0b11111,  0b11111,  0b11111,  0b11111,  0b11111,  0b11111,  0b11111,  0b11111}; // Убрал (с)НР
//uint8_t LL[8] = {0b11111,  0b11111,  0b11111,  0b11111,  0b11111,  0b11111,  0b11111,  0b11111}; // Убрал (с)НР
uint8_t LB[8] = {0b00000,  0b00000,  0b00000,  0b00000,  0b00000,  0b11111,  0b11111,  0b11111};   // для 0
//uint8_t LR[8] = {0b11111,  0b11111,  0b11111,  0b11111,  0b11111,  0b11111,  0b11111,  0b11111}; // Убрал (с)НР
uint8_t UMB[8] = {0b11111,  0b11111,  0b11111,  0b00000,  0b00000,  0b00000,  0b11111,  0b11111};  // для 2, 3, 5, 6, 8, 9
uint8_t LMB[8] = {0b11111,  0b00000,  0b00000,  0b00000,  0b00000,  0b11111,  0b11111,  0b11111};  // для 2, 3, 5, 6, 8, 9
uint8_t LM2[8] = {0b11111,  0b00000,  0b00000,  0b00000,  0b00000,  0b00000,  0b00000,  0b00000};  // для 4
uint8_t LB2[8] = {0b00000,  0b00000,  0b00000,  0b00000,  0b00000,  0b00000,  0b11111,  0b11111};  // для 4

// цифры четырехстрочные (с)НР
uint8_t UT[8] = {0b11111,  0b11111,  0b11111,  0b11111,  0b11111,  0b00000,  0b00000,  0b00000};   // для 2, 3, 4, 5, 6, 7, 8, 9, 0
uint8_t UM[8] = {0b00000,  0b00000,  0b00000,  0b00000,  0b00000,  0b11111,  0b11111,  0b11111};   // для 2, 3, 4, 5, 6, 8, 9
uint8_t DM[8] = {0b11111,  0b11111,  0b11111,  0b00000,  0b00000,  0b00000,  0b00000,  0b00000};   // для 2, 3, 4, 5, 6, 8, 9
uint8_t DT[8] = {0b00000,  0b00000,  0b00000,  0b11111,  0b11111,  0b11111,  0b11111,  0b11111};   // для 2, 3, 4, 5, 6, 8, 9, 0
uint8_t KU[8] = {0b00000,  0b00000,  0b00000,  0b00001,  0b00010,  0b00100,  0b01000,  0b10000};   // для верхней части %
uint8_t KD[8] = {0b00001,  0b00010,  0b00100,  0b01000,  0b10000,  0b00000,  0b00000,  0b00000};   // для нижней части %

// русские буквы для дней недели (с)НР
// П
uint8_t PP[8] = {0b11111,  0b10001,  0b10001,  0b10001,  0b10001,  0b10001,  0b10001,  0b00000};
// Б
uint8_t BB[8] = {0b11111,  0b10000,  0b10000,  0b11111,  0b10001,  0b10001,  0b11111,  0b00000};
// Ч
uint8_t CH[8] = {0b10001,  0b10001,  0b10001,  0b11111,  0b00001,  0b00001,  0b00001,  0b00000};

void drawDig(byte dig, byte x, byte y) {  // рисуем цифры времени (сделал цифры "квадратными" (с)НР)
  lcd.setCursor(x, y);  // чистим (с)НР
  String clr = "   ";
  if (x <= 11) clr = "    ";
  lcd.print(clr);
  lcd.setCursor(x, y + 1);
  lcd.print(clr);
  if (bigDig) {
    lcd.setCursor(x, y + 2);  // дочищаем (с)НР
    lcd.print(clr);
    lcd.setCursor(x, y + 3);
    lcd.print(clr);
    switch (dig) {   // четырехстрочные цифры
      case 0:
        lcd.setCursor(x, y); // set cursor to column 0, line 0 (first row)
        lcd.write(255);
        lcd.write(0);
        lcd.write(255);
        lcd.setCursor(x, y + 1); // set cursor to colum 0, line 1 (second row)
        lcd.write(255);
        lcd.write(32);
        lcd.write(255);
        lcd.setCursor(x, y + 2); // set cursor to colum 0, line 2
        lcd.write(255);
        lcd.write(32);
        lcd.write(255);
        lcd.setCursor(x, y + 3); // set cursor to colum 0, line 3
        lcd.write(255);
        lcd.write(3);
        lcd.write(255);
        break;
      case 1:
        lcd.setCursor(x + 1, y);
        lcd.write(255);
        lcd.setCursor(x + 1, y + 1);
        lcd.write(255);
        lcd.setCursor(x + 1, y + 2);
        lcd.write(255);
        lcd.setCursor(x + 1, y + 3);
        lcd.write(255);
        break;
      case 2:
        lcd.setCursor(x, y);
        lcd.write(0);
        lcd.write(0);
        lcd.write(255);
        lcd.setCursor(x, y + 1);
        lcd.write(1);
        lcd.write(1);
        lcd.write(255);
        lcd.setCursor(x, y + 2);
        lcd.write(255);
        lcd.write(2);
        lcd.write(2);
        lcd.setCursor(x, y + 3);
        lcd.write(255);
        lcd.write(3);
        lcd.write(3);
        break;
      case 3:
        lcd.setCursor(x, y);
        lcd.write(0);
        lcd.write(0);
        lcd.write(255);
        lcd.setCursor(x, y + 1);
        lcd.write(1);
        lcd.write(1);
        lcd.write(255);
        lcd.setCursor(x, y + 2);
        lcd.write(2);
        lcd.write(2);
        lcd.write(255);
        lcd.setCursor(x, y + 3);
        lcd.write(3);
        lcd.write(3);
        lcd.write(255);
        break;
      case 4:
        lcd.setCursor(x, y);
        lcd.write(255);
        lcd.write(32);
        lcd.write(255);
        lcd.setCursor(x, y + 1);
        lcd.write(255);
        lcd.write(1);
        lcd.write(255);
        lcd.setCursor(x, y + 2);
        lcd.write(2);
        lcd.write(2);
        lcd.write(255);
        lcd.setCursor(x + 2, y + 3);
        lcd.write(255);
        break;
      case 5:
        lcd.setCursor(x, y);
        lcd.write(255);
        lcd.write(0);
        lcd.write(0);
        lcd.setCursor(x, y + 1);
        lcd.write(255);
        lcd.write(1);
        lcd.write(1);
        lcd.setCursor(x, y + 2);
        lcd.write(2);
        lcd.write(2);
        lcd.write(255);
        lcd.setCursor(x, y + 3);
        lcd.write(3);
        lcd.write(3);
        lcd.write(255);
        break;
      case 6:
        lcd.setCursor(x, y);
        lcd.write(255);
        lcd.write(0);
        lcd.write(0);
        lcd.setCursor(x, y + 1);
        lcd.write(255);
        lcd.write(1);
        lcd.write(1);
        lcd.setCursor(x, y + 2);
        lcd.write(255);
        lcd.write(2);
        lcd.write(255);
        lcd.setCursor(x, y + 3);
        lcd.write(255);
        lcd.write(3);
        lcd.write(255);
        break;
      case 7:
        lcd.setCursor(x, y);
        lcd.write(0);
        lcd.write(0);
        lcd.write(255);
        lcd.setCursor(x + 2, y + 1);
        lcd.write(255);
        lcd.setCursor(x + 1, y + 2);
        lcd.write(255);
        lcd.setCursor(x + 1, y + 3);
        lcd.write(255);
        break;
      case 8:
        lcd.setCursor(x, y);
        lcd.write(255);
        lcd.write(0);
        lcd.write(255);
        lcd.setCursor(x, y + 1);
        lcd.write(255);
        lcd.write(1);
        lcd.write(255);
        lcd.setCursor(x, y + 2);
        lcd.write(255);
        lcd.write(2);
        lcd.write(255);
        lcd.setCursor(x, y + 3);
        lcd.write(255);
        lcd.write(3);
        lcd.write(255);
        break;
      case 9:
        lcd.setCursor(x, y);
        lcd.write(255);
        lcd.write(0);
        lcd.write(255);
        lcd.setCursor(x, y + 1);
        lcd.write(255);
        lcd.write(1);
        lcd.write(255);
        lcd.setCursor(x, y + 2);
        lcd.write(2);
        lcd.write(2);
        lcd.write(255);
        lcd.setCursor(x, y + 3);
        lcd.write(3);
        lcd.write(3);
        lcd.write(255);
        break;
      case 10:
        break;
    }
  }
  else {
    switch (dig) {
      case 0:
        lcd.setCursor(x, y); // set cursor to column 0, line 0 (first row)
        lcd.write(0);  // call each segment to create
        lcd.write(1);  // top half of the number
        lcd.write(0);
        lcd.setCursor(x, y + 1); // set cursor to colum 0, line 1 (second row)
        lcd.write(0);  // call each segment to create
        lcd.write(2);  // bottom half of the number
        lcd.write(0);
        break;
      case 1:
        lcd.setCursor(x + 1, y);
        //     lcd.write(1);   // убрал "носик" единички, вся цифра смещена левее (с)НР
        lcd.write(0);
        lcd.setCursor(x + 1, y + 1);
        lcd.write(0);
        break;
      case 2:
        lcd.setCursor(x, y);
        lcd.write(3);
        lcd.write(3);
        lcd.write(0);
        lcd.setCursor(x, y + 1);
        lcd.write(0);
        lcd.write(4);
        lcd.write(4);
        break;
      case 3:
        lcd.setCursor(x, y);
        lcd.write(3);
        lcd.write(3);
        lcd.write(0);
        lcd.setCursor(x, y + 1);
        lcd.write(4);
        lcd.write(4);
        lcd.write(0);
        break;
      case 4:
        lcd.setCursor(x, y);
        lcd.write(0);
        lcd.write(6);
        lcd.write(0);
        //      lcd.setCursor(x + 2, y + 1);
        lcd.setCursor(x + 0, y + 1);
        lcd.write(5);
        lcd.write(5);
        lcd.write(0);
        break;
      case 5:
        lcd.setCursor(x, y);
        lcd.write(0);
        lcd.write(3);
        lcd.write(3);
        lcd.setCursor(x, y + 1);
        lcd.write(4);
        lcd.write(4);
        lcd.write(0);
        break;
      case 6:
        lcd.setCursor(x, y);
        lcd.write(0);
        lcd.write(3);
        lcd.write(3);
        lcd.setCursor(x, y + 1);
        lcd.write(0);
        lcd.write(4);
        lcd.write(0);
        break;
      case 7:
        lcd.setCursor(x, y);
        lcd.write(1);
        lcd.write(1);
        lcd.write(0);
        lcd.setCursor(x + 1, y + 1);
        lcd.write(0);
        break;
      case 8:
        lcd.setCursor(x, y);
        lcd.write(0);
        lcd.write(3);
        lcd.write(0);
        lcd.setCursor(x, y + 1);
        lcd.write(0);
        lcd.write(4);
        lcd.write(0);
        break;
      case 9:
        lcd.setCursor(x, y);
        lcd.write(0);
        lcd.write(3);
        lcd.write(0);
        lcd.setCursor(x + 0, y + 1);
        lcd.write(4);
        lcd.write(4);
        lcd.write(0);
        break;
      case 10:
        //          lcd.setCursor(x, y);
        //          lcd.print("   ");
        //          lcd.setCursor(x, y + 1);
        //          lcd.print("   ");
        break;
    }
  }
}

void drawdots(byte x, byte y, boolean state) {  // Точки
  byte code;
  if (state) code = 165;
  else code = 32;
  if (mode0scr == 0) {    // мигание большими точками только в нулевом режиме главного экрана (с)НР
    lcd.setCursor(x, y);
    if (bigDig) lcd.setCursor(x, y + 2);
    lcd.write(code);
    lcd.setCursor(x, y + 1);
    lcd.write(code);
  }
  else {
    if (code == 165) code = 58;
    //    if (mode0scr == 1 && (!bigDig)) lcd.setCursor(17, 3);
    //    if (mode0scr == 1 && (bigDig)) lcd.setCursor(17, 3);
    //    if (mode0scr == 2 && (!bigDig)) lcd.setCursor(17, 3);
    //    if (mode0scr == 2 && (bigDig)) lcd.setCursor(17, 3);
    //    if (mode0scr == 3 && (!bigDig)) lcd.setCursor(17, 3);
    //    if (mode0scr == 3 && (bigDig)) lcd.setCursor(17, 3);
    //    if (mode0scr == 4 && (!bigDig)) lcd.setCursor(17, 3);
    //    if (mode0scr == 4 && (bigDig)) lcd.setCursor(17, 3);
    lcd.setCursor(17, 3);
    lcd.write(code);
  }
}

void drawPPM(int dispCO2, byte x, byte y) {   // Уровень СО2 крупно на главном экране (с)НР ----------------------------
  if (dispCO2 / 1000 == 0) drawDig(10, x, y);
  else drawDig(dispCO2 / 1000, x, y);
  drawDig((dispCO2 % 1000) / 100, x + 4, y);
  drawDig((dispCO2 % 100) / 10, x + 8, y);
  drawDig(dispCO2 % 10 , x + 12, y);
  lcd.setCursor(15, 0);
  lcd.print("ppm");
}


void drawPres(int dispPres, byte x, byte y) {   // Давление крупно на главном экране (с)НР ----------------------------
  drawDig((dispPres % 1000) / 100, x , y);
  drawDig((dispPres % 100) / 10, x + 4, y);
  drawDig(dispPres % 10 , x + 8, y);
  lcd.setCursor(11, 1);
  if (bigDig) lcd.setCursor(11, 3);
  lcd.print("mm");
}

void drawTemp(float dispTemp, byte x, byte y) { // Температура крупно на главном экране (с)НР ----------------------------
  if (dispTemp / 10 == 0) drawDig(10, x, y);
  else drawDig(dispTemp / 10, x, y);
  drawDig(int(dispTemp) % 10, x + 4, y);
  drawDig(int(dispTemp * 10.0) % 10, x + 9, y);

  if (bigDig) {
    lcd.setCursor(x + 7, y + 3);
    lcd.write(1);             // десятичная точка
  }
  else {
    lcd.setCursor(x + 7, y + 1);
    lcd.write(0B10100001);    // десятичная точка
  }
  lcd.setCursor(x + 13, y);
  lcd.write(223);           // градусы
  //lcd.write(0B01101111);  // градусы
}

void drawHum(int dispHum, byte x, byte y) {   // Влажность крупно на главном экране (с)НР ----------------------------
  if (dispHum / 100 == 0) drawDig(10, x, y);
  else drawDig(dispHum / 100, x, y);
  if ((dispHum % 100) / 10 == 0) drawDig(0, x + 4, y);
  else drawDig(dispHum / 10, x + 4, y);
  drawDig(int(dispHum) % 10, x + 8, y);
  if (bigDig) {
    lcd.setCursor(x + 12, y + 1);
    lcd.write(165);
    lcd.write(4);
    lcd.setCursor(x + 12, y + 2);
    lcd.write(5);
    lcd.write(165);
  }
  else {
    lcd.setCursor(x + 12, y + 1);
    lcd.print("%");
  }
}

void drawClock(byte hours, byte minutes, byte x, byte y, boolean dotState) {
  //if (hours > 23 || minutes > 59) return;
  if (hours / 10 == 0) drawDig(10, x, y);
  else drawDig(hours / 10, x, y);
  drawDig(hours % 10, x + 4, y);
  // тут должны быть точки. Отдельной функцией
  drawDig(minutes / 10, x + 8, y);
  drawDig(minutes % 10, x + 12, y);
}

#if (WEEK_LANG == 0)
static const char *dayNames[]  = {
  "Sund",
  "Mond",
  "Tues",
  "Wedn",
  "Thur",
  "Frid",
  "Satu",
};
#else
static const char *dayNames[]  = {  // доработал дни недели на двухсимвольные русские (ПН, ВТ, СР....) (с)НР
  "BC",
  " H",
  "BT",
  "CP",
  " T",
  " T",
  "C ",
};
#endif

void drawData() {
  int Y = 0;
  if (DISPLAY_TYPE == 1 && mode0scr == 1) Y = 2;
  if (!bigDig) {              // если 4-х строчные цифры, то дату не пишем - некуда (с)НР
    lcd.setCursor(15, 0 + Y);
    if (now.day() < 10) lcd.print(0);
    lcd.print(now.day());
    lcd.print(".");
    if (now.month() < 10) lcd.print(0);
    lcd.print(now.month());
  }

  if (DISP_MODE == 0) {
    lcd.setCursor(16, 1);
    lcd.print(now.year());
  } else if (DISP_MODE == 1) {
    if (WEEK_LANG == 0) {     // для русского двухсимвольного дня недели устанавливаем курсор на предпоследнюю позицию (с)НР
      lcd.setCursor(16, 1);
    } else {
      lcd.setCursor(18, 1);
    }
    int dayofweek = now.dayOfTheWeek();
    lcd.print(dayNames[dayofweek]);
    if (WEEK_LANG == 1) {
      if  (dayofweek == 1 || dayofweek == 5) { // Рисуем букву "П" для понедельника и пятницы. (с)НР
        lcd.setCursor(18, 1);
        lcd.write(7);
      }  else if (dayofweek == 4)  {         // Рисуем букву "Ч" для четверга (с)НР
        lcd.setCursor(18, 1);
        lcd.write(7);
      }  else if (dayofweek == 6)  {         // Рисуем букву "Б" для субботы (с)НР
        lcd.setCursor(19, 1);
        lcd.write(7);
      }
    }
  }
}

void drawPlot(byte pos, byte row, byte width, byte height, int min_val, int max_val, int *plot_array, String label, int stretch) {
  int max_value = -32000;
  int min_value = 32000;

  for (byte i = 0; i < 15; i++) {
    if (plot_array[i] > max_value) max_value = plot_array[i];
    if (plot_array[i] < min_value) min_value = plot_array[i];
  }

  // меняем пределы графиков на предельные/фактические значения (в пределах установленных лимитов), одновременно рисуем указатель пределов (стрелочки вверх-вниз) (с)НР
  lcd.setCursor(15, 0);
  if ((MAX_ONDATA & (1 << (stretch - 1))) > 0) {    // побитовое сравнение 1 - растягиваем, 0 - не растягиваем (по указанным пределам) (с)НР
    if (max_val >= max_value) max_val = max_value;
    lcd.write(0b01011110);
  }  else {
    lcd.write(0);
  }

  lcd.setCursor(15, 3);
  if ((MIN_ONDATA & (1 << (stretch - 1))) > 0) {    // побитовое сравнение 1 - растягиваем, 0 - не растягиваем (по указанным пределам) (с)НР
    if (min_val <= min_value) min_val = min_value;
    lcd.write(0);
  } else {
    lcd.write(0b01011110);
  }
  lcd.setCursor(15, 1); lcd.write(0b01111100);
  lcd.setCursor(15, 2); lcd.write(0b01111100);

  if (min_val >= max_val) max_val = min_val + 1;
  //Serial.println(max_val);Serial.println(min_val);  // отладка (с)НР

  lcd.setCursor(16, 0); lcd.print(max_value);
  lcd.setCursor(16, 1); lcd.print(label);
  lcd.setCursor(16, 2); lcd.print(plot_array[14]);
  lcd.setCursor(16, 3); lcd.print(min_value);

  for (byte i = 0; i < width; i++) {                  // каждый столбец параметров
    int fill_val = plot_array[i];
    fill_val = constrain(fill_val, min_val, max_val);
    byte infill, fract;
    // найти количество целых блоков с учётом минимума и максимума для отображения на графике
    if (plot_array[i] > min_val)
      infill = floor((float)(plot_array[i] - min_val) / (max_val - min_val) * height * 10);
    else infill = 0;
    fract = (float)(infill % 10) * 8 / 10;            // найти количество оставшихся полосок
    infill = infill / 10;

    for (byte n = 0; n < height; n++) {     // для всех строк графика
      if (n < infill && infill > 0) {       // пока мы ниже уровня
        lcd.setCursor(i, (row - n));        // заполняем полными ячейками
        lcd.write(255);
      }
      if (n >= infill) {                    // если достигли уровня
        lcd.setCursor(i, (row - n));
        if (n == 0 && fract == 0) fract++;      // если нижний перел графика имеет минимальное значение, то рисуем одну полоску, чтобы не было пропусков (с)НР
        if (fract > 0) lcd.write(fract);        // заполняем дробные ячейки
        else lcd.write(16);                     // если дробные == 0, заливаем пустой
        for (byte k = n + 1; k < height; k++) { // всё что сверху заливаем пустыми
          lcd.setCursor(i, (row - k));
          lcd.write(16);
        }
        break;
      }
    }
  }
}

void loadClock() {
  if (bigDig && (DISPLAY_TYPE == 1)) {              // для четырехстрочных цифр (с)НР ----------------------------
    lcd.createChar(0, UT);
    lcd.createChar(1, UM);
    lcd.createChar(2, DM);
    lcd.createChar(3, DT);
    lcd.createChar(4, KU);
    lcd.createChar(5, KD);
  }
  else {                                            // для двустрочных цифр (с)НР ----------------------------
    lcd.createChar(0, LT);
    lcd.createChar(1, UB);
    //  lcd.createChar(2, RT);
    //  lcd.createChar(3, LL);
    lcd.createChar(2, LB);
    //  lcd.createChar(5, LR);
    lcd.createChar(3, UMB);
    lcd.createChar(4, LMB);
    lcd.createChar(5, LM2);
    lcd.createChar(6, LB2);

  }

  if (now.dayOfTheWeek() == 4)  {          // Для четверга в ячейку запоминаем "Ч", для субботы "Б", иначе "П" (с)НР
    lcd.createChar(7, CH);  // Ч (с)НР
  } else if (now.dayOfTheWeek() == 6) {
    lcd.createChar(7, BB);  // Б (с)НР
  } else {
    lcd.createChar(7, PP);  // П (с)НР
  }
}

void loadPlot() {
  //  lcd.createChar(0, row8);  // такой символ есть под кодом 255 (с)НР
  lcd.createChar(0, rowS); // Стрелка вниз для индикатора пределов (с)НР
  lcd.createChar(1, row1);
  lcd.createChar(2, row2);
  lcd.createChar(3, row3);
  lcd.createChar(4, row4);
  lcd.createChar(5, row5);
  lcd.createChar(6, row6);
  lcd.createChar(7, row7);
}

#if (LED_MODE == 0)
#define LED_ON (LED_BRIGHT)
#else
#define LED_ON (255 - LED_BRIGHT)
#endif

void setLED(byte color) {
  // сначала всё выключаем
  if (!LED_MODE) {
    analogWrite(LED_R, 0);
    analogWrite(LED_G, 0);
    analogWrite(LED_B, 0);
  } else {
    analogWrite(LED_R, 255);
    analogWrite(LED_G, 255);
    analogWrite(LED_B, 255);
  }
  switch (color) {    // 0 выкл, 1 красный, 2 зелёный, 3 синий (или жёлтый)
    case 0:
      break;
    case 1: analogWrite(LED_R, LED_ON);
      break;
    case 2: analogWrite(LED_G, LED_ON);
      break;
    case 3:
      if (!BLUE_YELLOW) analogWrite(LED_B, LED_ON);
      else {
        analogWrite(LED_R, LED_ON - 50 * 0);  // чутка уменьшаем красный (убираем умеьшение красного (с)НР)
        analogWrite(LED_G, LED_ON / 2);       // Снижаем зеленый, чтобы был больше похож на желтый (с)НР
      }
      break;
  }
}

void setup() {
  Serial.begin(9600);

  pinMode(LED_COM, OUTPUT);
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);
  setLED(0);

  digitalWrite(LED_COM, LED_MODE);

  lcd.init();
  lcd.backlight();
  lcd.clear();

#if (DEBUG == 1 && DISPLAY_TYPE == 1)
  boolean status = true;

  setLED(1);

#if (CO2_SENSOR == 1)
  lcd.setCursor(0, 0);
  lcd.print(F("MHZ-19... "));
  Serial.print(F("MHZ-19... "));
  mhz19.begin(MHZ_TX, MHZ_RX);
  mhz19.setAutoCalibration(false);
  mhz19.getStatus();    // первый запрос, в любом случае возвращает -1
  delay(500);
  if (mhz19.getStatus() == 0) {
    lcd.print(F("OK"));
    Serial.println(F("OK"));
  } else {
    lcd.print(F("ERROR"));
    Serial.println(F("ERROR"));
    status = false;
  }
#endif

  setLED(2);
  lcd.setCursor(0, 1);
  lcd.print(F("RTC... "));
  Serial.print(F("RTC... "));
  delay(50);
  if (rtc.begin()) {
    lcd.print(F("OK"));
    Serial.println(F("OK"));
  } else {
    lcd.print(F("ERROR"));
    Serial.println(F("ERROR"));
    status = false;
  }

  setLED(3);
  lcd.setCursor(0, 2);
  lcd.print(F("BME280... "));
  Serial.print(F("BME280... "));
  delay(50);
  if (bme.begin(&Wire)) {
    lcd.print(F("OK"));
    Serial.println(F("OK"));
  } else {
    lcd.print(F("ERROR"));
    Serial.println(F("ERROR"));
    status = false;
  }

  setLED(0);
  lcd.setCursor(0, 3);
  if (status) {
    lcd.print(F("All good"));
    Serial.println(F("All good"));
    delay(1000);
    lcd.clear();
  } else {
    lcd.print(F("Check wires!"));
    Serial.println(F("Check wires!"));
    delay(1000);
    while (1);
  }
#else

#if (CO2_SENSOR == 1)
  mhz19.begin(MHZ_TX, MHZ_RX);
  mhz19.setAutoCalibration(false);
#endif
  rtc.begin();
  bme.begin(&Wire);
#endif

  bme.setSampling(Adafruit_BME280::MODE_FORCED,
                  Adafruit_BME280::SAMPLING_X1, // temperature
                  Adafruit_BME280::SAMPLING_X1, // pressure
                  Adafruit_BME280::SAMPLING_X1, // humidity
                  Adafruit_BME280::FILTER_OFF   );

  if (RESET_CLOCK || rtc.lostPower())
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  now = rtc.now();
  secs = now.second();
  mins = now.minute();
  hrs = now.hour();

  bme.takeForcedMeasurement();
  uint32_t Pressure = bme.readPressure();
  for (byte i = 0; i < 6; i++) {   // счётчик от 0 до 5
    pressure_array[i] = Pressure;  // забить весь массив текущим давлением
    time_array[i] = i;             // забить массив времени числами 0 - 5
  }

  if (DISPLAY_TYPE == 1) {
    loadClock();
    drawClock(hrs, mins, 0, 0, 1);
    drawData();
  }
  readSensors();
  drawSensors();
}

void loop() {
  if (sensorsTimer.isReady()) readSensors();    // читаем показания датчиков с периодом SENS_TIME
  //Serial.println(dispTemp);
#if (DISPLAY_TYPE == 1)
  if (clockTimer.isReady()) clockTick();        // два раза в секунду пересчитываем время и мигаем точками
  plotSensorsTick();                            // тут внутри несколько таймеров для пересчёта графиков (за час, за день и прогноз)
  modesTick();                                  // тут ловим нажатия на кнопку и переключаем режимы
  if (mode == 0) {                                  // в режиме "главного экрана"
    if (drawSensorsTimer.isReady()) drawSensors();  // обновляем показания датчиков на дисплее с периодом SENS_TIME
  } else {                                          // в любом из графиков
    if (plotTimer.isReady()) redrawPlot();          // перерисовываем график
  }
#else
  if (drawSensorsTimer.isReady()) drawSensors();
#endif
}
