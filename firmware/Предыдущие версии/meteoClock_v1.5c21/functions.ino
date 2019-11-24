void checkBrightness() {
  if (LCD_BRIGHT == 11) {                         // если установлен автоматический режим для экрана (с)НР
    if (analogRead(PHOTO) < BRIGHT_THRESHOLD) {   // если темно
      analogWrite(BACKLIGHT, LCD_BRIGHT_MIN);
    } else {                                      // если светло
      analogWrite(BACKLIGHT, LCD_BRIGHT_MAX);
    }
  } else {
    analogWrite(BACKLIGHT, LCD_BRIGHT * LCD_BRIGHT * 2.5);
  }

  if (LED_BRIGHT == 11) {                         // если установлен автоматический режим для индикатора (с)НР
    if (analogRead(PHOTO) < BRIGHT_THRESHOLD) {   // если темно
#if (LED_MODE == 0)
      LED_ON = (LED_BRIGHT_MIN);
#else
      LED_ON = (255 - LED_BRIGHT_MIN);
#endif
    } else {                                      // если светло
#if (LED_MODE == 0)
      LED_ON = (LED_BRIGHT_MAX);
#else
      LED_ON = (255 - LED_BRIGHT_MAX);
#endif
    }
  }
}

/*
  mode:
  0 - Главный экран
  1-8 - Графики: СО2 (час, день), Влажность (час, день), Температура (час, день), Осадки (час, день)
  252 - выбор режима индикатора (podMode от 0 до 3: индикация СО2, влажности, температуры, осадков)
  253 - настройка яркости экрана (podMode от 0 до 11: выбор от 0% до 100% или автоматическое регулирование)
  254 - настройка яркости индикатора (podMode от 0 до 11: выбор от 0% до 100% или автоматическое регулирование)
  255 - главное меню (podMode от 0 до 13: 1 - Сохранить, 2 - Выход, 3 - Ярк.индикатора, 4 - Ярк.экрана, 5 - Режим индикатора,
                                        6-13 вкл/выкл графики: СО2 (час, день), Влажность (час, день), Температура (час, день), Осадки (час, день))
*/
void modesTick() {
  button.tick();
  boolean changeFlag = false;
  if (button.isSingle()) {    // одинарное нажатие на кнопку

    if (mode >= 240) {
      switch (mode) {
        case 252:             // Перебираем все варианты режимов LED индикатора (с)НР
          podMode++;
          if (podMode > 3) podMode = 0;
          LEDType = podMode;
          changeFlag = true;
          break;

        case 253:             // Перебираем все варианты яркости LCD экрана (с)НР
          podMode++;
          if (podMode > 11) podMode = 0;
          LCD_BRIGHT = podMode;
          checkBrightness();
                    changeFlag = true;
          break;

        case 254:             // Перебираем все варианты яркости LED индикатора (с)НР
          podMode++;
          if (podMode > 11) podMode = 0;
          LED_BRIGHT = podMode;
          changeFlag = true;
          break;

        case 255:             // Перебираем все варианты основных настроек (с)НР
          podMode++;
          if (podMode > 13) podMode = 1;
          changeFlag = true;
          break;
      }
    } else {
      do {
        mode++;
        if (mode > 8) mode = 0;
#if (CO2_SENSOR == 0 && mode == 1)
        mode = 3;
#endif
      } while (((VIS_ONDATA & (1 << (mode - 1))) == 0) && (mode > 0));   // проверка на отображение графиков (с)НР
      changeFlag = true;
    }
  }
  if (button.isDouble()) {                    // двойное нажатие (с)НР ----------------------------
    if (mode > 0 && mode < 10) {              // Меняет пределы графика на установленные/фактические максимумы (с)НР
      MAX_ONDATA = MAX_ONDATA ^ (1 << (mode - 1));
    } else if (mode == 0)  {
      mode0scr++;
#if (CO2_SENSOR == 0 && mode0scr == 1)
      mode0scr++;
#endif
      if (mode0scr > 4) mode0scr = 0;         // Переключение рехима работы главного экрана (с)НР
    } else if (mode > 240) podMode = 1;       // Переключение на меню сохранения (с)НР
    changeFlag = true;
  }

  if ((button.isTriple()) && (mode == 0)) {  // тройное нажатие - для дисплея 2004 в режиме главного экрана переход в режим сохранения натроек (с)НР
    mode = 255;
    podMode = 3;
    changeFlag = true;
  }

  if (button.isHolded()) {    // удержание кнопки (с)НР
//    if (mode >=252 && mode <= 254) {
//      mode = 255;
//      podMode = 1;
//    }
    switch (mode) {
      case 0:
        bigDig = !bigDig;
        break;
      case 252:       // реж. индикатора
        mode = 255;
        podMode = 1;
        break;
      case 253:       // ярк. экрана
        mode = 255;
        podMode = 1;
        break;
      case 254:       // ярк. индикатора
        mode = 255;
        podMode = 1;
        break;
      case 255:       // главное меню
        if (podMode == 2 || podMode == 1) mode = 0;                   // если Выход или Сохранить
        if (podMode >= 3 && podMode <= 5) mode = 255 - podMode + 2;   // если настройки яркостей, то переключаемся в настройки пункта меню
        if (podMode >= 6 && podMode <= 13) VIS_ONDATA = VIS_ONDATA ^ (1 << (podMode - 6));  // вкл/выкл отображения графиков
        if (podMode == 1) {                                           // если Сохранить
          if (EEPROM.read(2) != MAX_ONDATA) EEPROM.write(2, MAX_ONDATA);
          if (EEPROM.read(3) != VIS_ONDATA) EEPROM.write(3, VIS_ONDATA);
          if (EEPROM.read(4) != mode0scr) EEPROM.write(4, mode0scr);
          if (EEPROM.read(5) != bigDig) EEPROM.write(5, bigDig);
          if (EEPROM.read(6) != LED_BRIGHT) EEPROM.write(6, LED_BRIGHT);
          if (EEPROM.read(7) != LCD_BRIGHT) EEPROM.write(7, LCD_BRIGHT);
          if (EEPROM.read(8) != LEDType) EEPROM.write(8, LEDType);
          if (EEPROM.read(0) != 210) EEPROM.write(0, 210);
        }
        if (podMode < 6) podMode = 1;
        if (mode == 252) podMode = LEDType;     // если выбран режим LED - устанавливаем текущее значение (с)НР
        if (mode == 254) podMode = LED_BRIGHT;  // если выбрана яркость LED - устанавливаем текущее показание (с)НР
        if (mode == 253) podMode = LCD_BRIGHT;  // если выбрана яркость LCD - устанавливаем текущее показание (с)НР
        break;
      default:
        mode = 0;
    }
    changeFlag = true;
  }

  if (changeFlag) {
    if (mode >= 240) {
      lcd.clear();
      lcd.createChar(1, BM);  //Ь
      lcd.createChar(2, IY);  //Й
      lcd.createChar(3, DD);  //Д
      lcd.createChar(4, II);  //И
      lcd.createChar(5, IA);  //Я
      lcd.createChar(6, YY);  //Ы
      lcd.createChar(7, AA);  //Э
      lcd.createChar(0, ZZ);  //Ж
      lcd.setCursor(0, 0);
    }
    if (mode == 255) {          // Перебираем варианты в главном меню (с)НР
      lcd.print("HACTPO\2K\4:");              // ---Настройки
      lcd.setCursor(0, 1);
      switch (podMode) {
        case 1: lcd.print("COXPAH\4T\1");     // ---Сохранить
          break;
        case 2: lcd.print("B\6XO\3");         // --- Выход
          break;
        case 5: lcd.print("PE\10.\4H\3\4KATOPA");  // ---Реж.индик.
          break;
        case 3: lcd.print("\5PK.\4H\3\4KATOPA");  // ---Ярк.индик.
          break;
        case 4: lcd.print("\5PK.\7KPAHA");    // ---Ярк.экрана
          break;
      }
      if (podMode >= 6 && podMode <= 13) {
        lcd.createChar(8, FF);  //ф
        lcd.createChar(7, GG);  //Г
        lcd.createChar(5, LL);  //Л
        lcd.setCursor(10, 0);
        lcd.print("\7PA\10\4KOB");            // ---графиков
        lcd.setCursor(0, 1);
        if ((3 & (1 << (podMode - 6))) != 0) lcd.print("CO2 ");
        if ((12 & (1 << (podMode - 6))) != 0) lcd.print("B\5,% ");
        if ((48 & (1 << (podMode - 6))) != 0) lcd.print("t\337 ");
        if ((192 & (1 << (podMode - 6))) != 0) lcd.print("p,rain ");

        if ((85 & (1 << (podMode - 6))) != 0) {
          lcd.createChar(3, CH);  //Ч
          lcd.setCursor(8, 1);
          lcd.print("\3AC:");
        } else {
          lcd.setCursor(7, 1);
          lcd.print("\3EH\1:");
        }
        if ((VIS_ONDATA & (1 << (podMode - 6))) != 0) lcd.print("BK\5 ");
        else lcd.print("B\6K\5");
      }
    }
    if (mode == 252) {                        // --------------------- показать  "Реж.индикатора"
      LEDType = podMode;
      lcd.createChar(6, LL);  //Л
      lcd.createChar(3, DD);  //Д
      lcd.createChar(5, II);  //И
      lcd.createChar(8, ZZ);  //Ж
      lcd.setCursor(0, 0);
      lcd.print("PE\10.\4H\3\4KATOPA:");
      lcd.setCursor(0, 1);
      switch (podMode) {
        case 0:
          lcd.print("CO2   ");
          break;
        case 1:
          lcd.print("B\6A\10H.");          // влажн.
          break;
        case 2:
          lcd.print("t\337     ");
          break;
        case 3:
          lcd.print("OCA\3K\5");          // осадки
          break;
      }

    }
    if (mode == 253) {                        // --------------------- показать  "Ярк.экрана"
      lcd.print("\5PK.\7KPAHA:");// + String(LCD_BRIGHT * 10) + "%  ");
      //lcd.setCursor(11, 0);
      if (LCD_BRIGHT == 11) lcd.print("ABTO ");
      else lcd.print(String(LCD_BRIGHT * 10) + "%");
    }
    if (mode == 254) {                        // --------------------- показать  "Ярк.индикатора"
      lcd.print("\5PK.\4H\3\4K.:");// + String(LED_BRIGHT * 10) + "%  ");
      //lcd.setCursor(15, 0);
      if (LED_BRIGHT == 11) lcd.print("ABTO ");
      else lcd.print(String(LED_BRIGHT * 10) + "%");
    }

    if (mode == 0) {
      lcd.clear();
      loadClock();
      drawSensors();
      if (DISPLAY_TYPE == 1) drawData();
    } else if (mode <= 10) {
      //lcd.clear();
      loadPlot();
      redrawPlot();
    }
  }
}

void redrawPlot() {
  lcd.clear();
#if (DISPLAY_TYPE == 1)       // для дисплея 2004
  switch (mode) {             // добавлена переменная для "растягивания" графика до фактических максимальных и(или) минимальных значений(с)НР
    case 1: drawPlot(0, 3, 15, 4, CO2_MIN, CO2_MAX, (int*)co2Hour, "c ", "hr", mode);
      break;
    case 2: drawPlot(0, 3, 15, 4, CO2_MIN, CO2_MAX, (int*)co2Day, "c ", "da", mode);
      break;
    case 3: drawPlot(0, 3, 15, 4, HUM_MIN, HUM_MAX, (int*)humHour, "h%", "hr", mode);
      break;
    case 4: drawPlot(0, 3, 15, 4, HUM_MIN, HUM_MAX, (int*)humDay, "h%", "da", mode);
      break;
    case 5: drawPlot(0, 3, 15, 4, TEMP_MIN, TEMP_MAX, (int*)tempHour, "t\337", "hr", mode);
      break;
    case 6: drawPlot(0, 3, 15, 4, TEMP_MIN, TEMP_MAX, (int*)tempDay, "t\337", "da", mode);
      break;
    case 7: drawPlot(0, 3, 15, 4, PRESS_MIN, PRESS_MAX, (int*)pressHour, "p ", "hr", mode);
      break;
    case 8: drawPlot(0, 3, 15, 4, PRESS_MIN, PRESS_MAX, (int*)pressDay, "p ", "da", mode);
      break;
  }
#else                         // для дисплея 1602
  switch (mode) {             
    case 1: drawPlot(0, 1, 12, 2, CO2_MIN, CO2_MAX, (int*)co2Hour, "c", "h", mode);
      break;
    case 2: drawPlot(0, 1, 12, 2, CO2_MIN, CO2_MAX, (int*)co2Day, "c", "d", mode);
      break;
    case 3: drawPlot(0, 1, 12, 2, HUM_MIN, HUM_MAX, (int*)humHour, "h", "h", mode);
      break;
    case 4: drawPlot(0, 1, 12, 2, HUM_MIN, HUM_MAX, (int*)humDay, "h", "d", mode);
      break;
    case 5: drawPlot(0, 1, 12, 2, TEMP_MIN, TEMP_MAX, (int*)tempHour, "t", "h", mode);
      break;
    case 6: drawPlot(0, 1, 12, 2, TEMP_MIN, TEMP_MAX, (int*)tempDay, "t", "d", mode);
      break;
    case 7: drawPlot(0, 1, 12, 2, PRESS_MIN, PRESS_MAX, (int*)pressHour, "p", "h", mode);
      break;
    case 8: drawPlot(0, 1, 12, 2, PRESS_MIN, PRESS_MAX, (int*)pressDay, "p", "d", mode);
      break;
  }
#endif
}

void readSensors() {
  bme.takeForcedMeasurement();
  dispTemp = bme.readTemperature();
  dispHum = bme.readHumidity();
  dispPres = (float)bme.readPressure() * 0.00750062;
#if (CO2_SENSOR == 1)
  dispCO2 = mhz19.getPPM();
#else
  dispCO2 = 0;
#endif
}

void drawSensors() {
#if (DISPLAY_TYPE == 1)
  // дисплей 2004 ----------------------------------

  if (mode0scr != 2) {                        // Температура (с)НР ----------------------------
    lcd.setCursor(0, 2);
    if (bigDig) {
      if (mode0scr == 1) lcd.setCursor(15, 2);
      if (mode0scr != 1) lcd.setCursor(15, 0);
    }
    lcd.print(String(dispTemp, 1));
    lcd.write(223);
  } else {
    drawTemp(dispTemp, 0, 0);
  }

  if (mode0scr != 4) {                        // Влажность (с)НР ----------------------------
    if (!bigDig) {                            // для четырехстрочных цифр мелко влажность не выводим (с)НР
      lcd.setCursor(5, 2);
      lcd.print(" " + String(dispHum) + "%  ");
    }
  } else {
    drawHum(dispHum, 0, 0);
  }

#if (CO2_SENSOR == 1)
  if (mode0scr != 1) {                        // СО2 (с)НР ----------------------------

    if (bigDig) {
      lcd.setCursor(15, 2);
      lcd.print(String(dispCO2) + "p");
    } else {
      lcd.setCursor(11, 2);
      lcd.print(String(dispCO2) + "ppm ");
    }
  } else {
    drawPPM(dispCO2, 0, 0);
  }
#endif

  if (mode0scr != 3) {                      // давление (с)НР ---------------------------
    lcd.setCursor(0, 3);
    if (bigDig && mode0scr == 0) lcd.setCursor(15, 3);
    if (bigDig && (mode0scr == 1 || mode0scr == 2)) lcd.setCursor(15, 0);
    if (!(bigDig && (mode0scr == 1 || mode0scr == 4))) lcd.print(String(dispPres) + "mm");
  } else {
    drawPres(dispPres, 0, 0);
  }

  if (!bigDig) {                            // дождь (с)НР -----------------------------
    lcd.setCursor(5, 3);
    lcd.print(" rain     ");
    lcd.setCursor(11, 3);
    if (dispRain < 0) lcd.setCursor(10, 3);
    lcd.print(String(dispRain) + "%");
    //  lcd.setCursor(14, 3);
    //  lcd.print(bme.readAltitude(SEALEVELPRESSURE_HPA));  // высота над уровнем моря (с)НР
  }

  if (mode0scr != 0) {                      // время (с)НР ----------------------------
    lcd.setCursor(15, 3);
    if (hrs / 10 == 0) lcd.print(" ");
    lcd.print(hrs);
    lcd.print(":");
    if (mins / 10 == 0) lcd.print("0");
    lcd.print(mins);
  } else {
    drawClock(hrs, mins, 0, 0); //, 1);
  }
#else
 
  // дисплей 1602 ----------------------------------
  if (!bigDig) {              // если только мелкими цифрами (с)НР
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
  } else {                    // для крупных цифр (с)НР
    switch (mode0scr) {
      case 0:
        drawClock(hrs, mins, 0, 0);
        break;
      case 1:
#if (CO2_SENSOR == 1)
        drawPPM(dispCO2, 0, 0);
#endif
        break;
      case 2:
        drawTemp(dispTemp, 2, 0);
        break;
      case 3:
        drawPres(dispPres, 2, 0);
        break;
      case 4:
        drawHum(dispHum, 0, 0);
        break;
    }
  }
#endif
}

void plotSensorsTick() {
  // 4 или 5 минутный таймер
  if (testTimer(hourPlotTimerD, hourPlotTimer)) {
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

  // 1.5 или 2 часовой таймер
  if (testTimer(dayPlotTimerD, dayPlotTimer)) {
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
  if (testTimer(predictTimerD, predictTimer)) {
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
    pressure_array[5] = averPress;                   // последний элемент массива теперь - новое давление
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
    a = (long)6 * sumXY;                            // расчёт коэффициента наклона приямой
    a = a - (long)sumX * sumY;
    a = (float)a / (6 * sumX2 - sumX * sumX);
    delta = a * 6;      // расчёт изменения давления
    dispRain = map(delta, -250, 250, 100, -100);    // пересчитать в проценты
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
        drawSensors();       // (с)НР
      }
    }
    if (mins > 59) {      // каждый час
      now = rtc.now();
      secs = now.second();
      mins = now.minute();
      hrs = now.hour();
      if (mode == 0) drawSensors();
      if (hrs > 23) hrs = 0;
      if (mode == 0 && DISPLAY_TYPE) drawData();
    }
    if ((DISP_MODE == 2 && mode == 0 || WEEK_LANG == 1 && DISP_MODE == 1 && mode == 0) && DISPLAY_TYPE == 1) {   // Если режим секунд или дни недели по-русски, то показывать секунды (с)НР
      lcd.setCursor(14 + DISP_MODE, 1);
      if (secs < 10) lcd.print(" ");
      lcd.print(secs);
    }
  }
  if (mode == 0) drawdots(7, 0, dotFlag); // мигание точками

  if ((dispCO2 >= blinkLEDCO2 && LEDType == 0 || dispHum <= blinkLEDHum && LEDType == 1 || dispTemp >= blinkLEDTemp && LEDType == 2) && !dotFlag) setLED1(0);     // изменил значение для мигания индикатора на определяемое по каждому из сенсоров (с)НР
  else setLED();
}

boolean testTimer(unsigned long & dataTimer, unsigned long setTimer) {   // Проверка таймеров (с)НР
  if (millis() - dataTimer >= setTimer) {
    dataTimer = millis();
    return true;
  } else {
    return false;
  }
}
