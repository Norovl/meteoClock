void modesTick() {
  button.tick();
  boolean changeFlag = false;
  if (button.isSingle()) {
    mode++;
    if (mode > 8) mode = 0;
#if (CO2_SENSOR == 0 && mode == 1)
    mode = 3;
//#else
//    if (mode > 6) mode = 0;
#endif
    changeFlag = true;
  }
  if (button.isDouble()) {                  // двойное нажатие (с)НР ----------------------------
    if (mode > 0) {                         // Меняет пределы графика на установленные/фактические максимумы (с)НР
      int bt = 1 << (mode - 1);
      if ((MAX_ONDATA & bt) > 0) {
        MAX_ONDATA = MAX_ONDATA - bt;
      }
      else {
        MAX_ONDATA = MAX_ONDATA + bt;
      }
      if ((MIN_ONDATA & bt) > 0) {
        MIN_ONDATA = MIN_ONDATA - bt;
      }
      else {
        MIN_ONDATA = MIN_ONDATA + bt;
      }
    }
    else {
      mode0scr++;
#if (CO2_SENSOR == 0 && mode0scr == 1)
      mode0scr++;
#endif
      if (mode0scr > 4) mode0scr = 0;         // Переключение рехима работы главного экрана (с)НР
    } changeFlag = true;
  }
  //  if ((button.isTriple()) && (mode == 0) && (DISPLAY_TYPE == 1)) {  // тройное нажатие - для дисплея 2004 в режиме главного экрана меряет размер крцпных цифр (с)НР
  //    bigDig = !bigDig;
  //    changeFlag = true;
  //  }

  // Serial.print(mode);Serial.print(" ");Serial.println(MAX_ONDATA); // Отладка (с)НР

  if (button.isHolded()) {
    if (mode == 0) bigDig = !bigDig;
    mode = 0;
    changeFlag = true;
  }

  if (changeFlag) {
    if (mode == 0) {
      lcd.clear();
      loadClock();
      // drawClock(hrs, mins, 0, 0, 1);  // перемещен в drawSensors() (с)НР
      drawSensors();
      if (DISPLAY_TYPE == 1) drawData();
    } else {
      lcd.clear();
      loadPlot();
      redrawPlot();
    }
  }
}

void redrawPlot() {
  lcd.clear();
  switch (mode) {             // добавлена переменная для "растягивания" графика до фактических максимальных и(или) минимальных значений(с)НР
    case 1: drawPlot(0, 3, 15, 4, CO2_MIN, CO2_MAX, (int*)co2Hour, "c hr", mode);
      break;
    case 2: drawPlot(0, 3, 15, 4, CO2_MIN, CO2_MAX, (int*)co2Day, "c day", mode);
      break;    
    case 3: drawPlot(0, 3, 15, 4, HUM_MIN, HUM_MAX, (int*)humHour, "h%hr", mode);
      break;
    case 4: drawPlot(0, 3, 15, 4, HUM_MIN, HUM_MAX, (int*)humDay, "h%day", mode);
      break;
    case 5: drawPlot(0, 3, 15, 4, TEMP_MIN, TEMP_MAX, (int*)tempHour, "t" + String(char(223)) + "hr", mode);
      break;
    case 6: drawPlot(0, 3, 15, 4, TEMP_MIN, TEMP_MAX, (int*)tempDay, "t" + String(char(223)) + "day", mode);
      break;
    case 7: drawPlot(0, 3, 15, 4, PRESS_MIN, PRESS_MAX, (int*)pressHour, "p hr", mode);
      break;
    case 8: drawPlot(0, 3, 15, 4, PRESS_MIN, PRESS_MAX, (int*)pressDay, "p day", mode);
      break;
  }
}

void readSensors() {
  bme.takeForcedMeasurement();
  dispTemp = bme.readTemperature();
  dispHum = bme.readHumidity();
  dispPres = (float)bme.readPressure() * 0.00750062;
#if (CO2_SENSOR == 1)
  dispCO2 = mhz19.getPPM();

  if (dispCO2 < 800) setLED(2);
  else if (dispCO2 < 1200) setLED(3);
  else if (dispCO2 >= 1200) setLED(1);
#endif
}

void drawSensors() {
#if (DISPLAY_TYPE == 1)
  // дисплей 2004

  if (mode0scr != 2) {                      // Температура (с)НР ----------------------------
    lcd.setCursor(0, 2);
    if (bigDig) {
      if (mode0scr == 1) lcd.setCursor(15, 2);
      if (mode0scr != 1) lcd.setCursor(15, 0);
    }
    lcd.print(String(dispTemp, 1));
    lcd.write(223);
  }
  else
  {
    drawTemp(dispTemp, 0, 0);
  }

  if (mode0scr != 4) {                      // Влажность (с)НР ----------------------------
    if (!bigDig) {      // для четырехстрочных цифр мелко влажность не выводим (с)НР
      lcd.setCursor(5, 2);
      lcd.print(" " + String(dispHum) + "%  ");
    }
  }
  else {
    drawHum(dispHum, 0, 0);
  }

#if (CO2_SENSOR == 1)
  if (mode0scr != 1) {                      // СО2 (с)НР ----------------------------

    if (bigDig) {
      lcd.setCursor(15, 2);
      lcd.print(String(dispCO2) + "p");
    }
    else {
      lcd.setCursor(11, 2);
      lcd.print(String(dispCO2) + "ppm ");
      //if (dispCO2 < 1000) lcd.print(" ");
    }
  }
  else {
    drawPPM(dispCO2, 0, 0);
  }
#endif

  if (mode0scr != 3) {                      // Давление (с)НР ----------------------------
    lcd.setCursor(0, 3);
    if (bigDig && mode0scr == 0) lcd.setCursor(15, 3);
    if (bigDig && (mode0scr == 1 || mode0scr == 2)) lcd.setCursor(15, 0);
    if (!(bigDig && (mode0scr == 1 || mode0scr == 4))) lcd.print(String(dispPres) + "mm");
  }
  else {
    drawPres(dispPres, 0, 0);
  }

  if (!bigDig) {
    lcd.setCursor(5, 3);
    lcd.print(" rain     ");
    //lcd.print(F("       "));
    lcd.setCursor(11, 3);
    if (dispRain < 0) lcd.setCursor(10, 3);
    lcd.print(String(dispRain) + "%");
    //  lcd.setCursor(14, 3);
    //  lcd.print(bme.readAltitude(SEALEVELPRESSURE_HPA));  // высота над уровнем моря (с)НР
  }

  if (mode0scr != 0) {                // время мелкими символами (с)НР ----------------------------
    //    if (mode0scr == 1 && (!bigDig)) lcd.setCursor(15, 3);
    //    if (mode0scr == 1 && (bigDig)) lcd.setCursor(15, 3);
    //    if (mode0scr == 2 && (!bigDig)) lcd.setCursor(15, 3);
    //    if (mode0scr == 2 && (bigDig)) lcd.setCursor(15, 3);
    //    if (mode0scr == 3 && (!bigDig)) lcd.setCursor(15, 3);
    //    if (mode0scr == 3 && (bigDig)) lcd.setCursor(15, 3);
    //    if (mode0scr == 4 && (!bigDig)) lcd.setCursor(15, 3);
    //    if (mode0scr == 4 && (bigDig)) lcd.setCursor(15, 3);
    lcd.setCursor(15, 3);
    if (hrs / 10 == 0) lcd.print(" ");
    lcd.print(hrs);
    lcd.print(":");
    if (mins / 10 == 0) lcd.print("0");
    lcd.print(mins);
  }
  else {
    drawClock(hrs, mins, 0, 0, 1);
  }
#else
  // дисплей 1602
  lcd.setCursor(0, 0);
  lcd.print(String(dispTemp, 1));
  lcd.write(223);
  lcd.setCursor(6, 0);
  lcd.print(String(dispHum) + "% ");

#if (CO2_SENSOR == 1)
  lcd.print(String(dispCO2) + "ppm");
  if (dispCO2 < 1000) lcd.print(" ");
#endif

  lcd.setCursor(0, 1);
  lcd.print(String(dispPres) + " mm  rain ");
  lcd.print(String(dispRain) + "% ");
#endif
}

void plotSensorsTick() {
  // 4 минутный таймер
  if (hourPlotTimer.isReady()) {
    for (byte i = 0; i < 14; i++) {
      tempHour[i] = tempHour[i + 1];
      humHour[i] = humHour[i + 1];
      pressHour[i] = pressHour[i + 1];
      co2Hour[i] = co2Hour[i + 1];
    }
    tempHour[14] = dispTemp;
    humHour[14] = dispHum;
    co2Hour[14] = dispCO2;

    if (PRESSURE) pressHour[14] = dispRain;
    else pressHour[14] = dispPres;
  }

  // 1.5 часовой таймер
  if (dayPlotTimer.isReady()) {
    long averTemp = 0, averHum = 0, averPress = 0, averCO2 = 0;

    for (byte i = 0; i < 15; i++) {
      averTemp += tempHour[i];
      averHum += humHour[i];
      averPress += pressHour[i];
      averCO2 += co2Hour[i];
    }
    averTemp /= 15;
    averHum /= 15;
    averPress /= 15;
    averCO2 /= 15;

    for (byte i = 0; i < 14; i++) {
      tempDay[i] = tempDay[i + 1];
      humDay[i] = humDay[i + 1];
      pressDay[i] = pressDay[i + 1];
      co2Day[i] = co2Day[i + 1];
    }
    tempDay[14] = averTemp;
    humDay[14] = averHum;
    pressDay[14] = averPress;
    co2Day[14] = averCO2;
  }

  // 10 минутный таймер
  if (predictTimer.isReady()) {
    // тут делаем линейную аппроксимацию для предсказания погоды
    long averPress = 0;
    for (byte i = 0; i < 10; i++) {
      bme.takeForcedMeasurement();
      averPress += bme.readPressure();
      delay(1);
    }
    averPress /= 10;

    for (byte i = 0; i < 5; i++) {                   // счётчик от 0 до 5 (да, до 5. Так как 4 меньше 5)
      pressure_array[i] = pressure_array[i + 1];     // сдвинуть массив давлений КРОМЕ ПОСЛЕДНЕЙ ЯЧЕЙКИ на шаг назад
    }
    pressure_array[5] = averPress;                    // последний элемент массива теперь - новое давление
    sumX = 0;
    sumY = 0;
    sumX2 = 0;
    sumXY = 0;
    for (int i = 0; i < 6; i++) {                    // для всех элементов массива
      sumX += time_array[i];
      sumY += (long)pressure_array[i];
      sumX2 += time_array[i] * time_array[i];
      sumXY += (long)time_array[i] * pressure_array[i];
    }
    a = 0;
    a = (long)6 * sumXY;             // расчёт коэффициента наклона приямой
    a = a - (long)sumX * sumY;
    a = (float)a / (6 * sumX2 - sumX * sumX);
    delta = a * 6;      // расчёт изменения давления
    dispRain = map(delta, -250, 250, 100, -100);  // пересчитать в проценты
    //Serial.println(String(pressure_array[5]) + " " + String(delta) + " " + String(dispRain));   // дебаг
  }
}

boolean dotFlag;
void clockTick() {
  dotFlag = !dotFlag;
  if (dotFlag) {          // каждую секунду пересчёт времени
    secs++;
    if (secs > 59) {      // каждую минуту
      secs = 0;
      mins++;
      if (mins <= 59 && mode == 0) {
        if (mode0scr == 0) {
          drawClock(hrs, mins, 0, 0, 1);
        }
      }
    }
    if (mins > 59) {      // каждый час
      now = rtc.now();
      secs = now.second();
      mins = now.minute();
      hrs = now.hour();
      if (mode == 0) drawClock(hrs, mins, 0, 0, 1);
      if (hrs > 23) {
        hrs = 0;
      }
      if (mode == 0 && DISPLAY_TYPE) drawData();
    }
    if (DISP_MODE == 2 && mode == 0 || WEEK_LANG == 1 && DISP_MODE == 1 && mode == 0) {   // Если режим секунд или дни недели по-русски, то показывать секунды (с)НР
      lcd.setCursor(14 + DISP_MODE, 1);
      if (secs < 10) lcd.print(" ");
      lcd.print(secs);
    }
  }
  if (mode == 0) drawdots(7, 0, dotFlag);
  if (dispCO2 >= 1500) {    // изменил 1200 на 1500, чтобы какое-то время горел красный, а при превышении уже начал мигать (с)НР
    if (dotFlag) setLED(1);
    else setLED(0);
  }
}
