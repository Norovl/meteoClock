# meteoClock

Это лубоко переработанная прошивки AlexGyver под метеостанцию.
(Оригинал здесь: ![Исходная версия](https://github.com/AlexGyver/MeteoClock))

##Краткие характеристики:

-   Двойным нажатием на кнопку переключаются режимы крупного отображения
    значений следующих показателей: время – содержание СО2 – температура
    – давление - влажность. Для экрана 1602 работает только в режиме 
    крупных цифр (см.ниже);

![г1](https://github.com/Norovl/meteoClock/blob/master/media/136Time.jpg)
![г2](https://github.com/Norovl/meteoClock/blob/master/media/136co2.jpg)
![г3](https://github.com/Norovl/meteoClock/blob/master/media/136temp.jpg)
![г4](https://github.com/Norovl/meteoClock/blob/master/media/136mmhg.jpg)
![г5](https://github.com/Norovl/meteoClock/blob/master/media/136h.jpg)

-   удержанием кнопки переключается режим большие цифры/очень большие
    цифры (для экрана 2004), или просто переключается на большие цифры
    (для экрана 1602)

![г6](https://github.com/Norovl/meteoClock/blob/master/media/136BigTime.jpg)
![г7](https://github.com/Norovl/meteoClock/blob/master/media/136BigTemp.jpg)
![г8](https://github.com/Norovl/meteoClock/blob/master/media/15c21_1602_time.jpg)
![г9](https://github.com/Norovl/meteoClock/blob/master/media/15c21_1602_temp.jpg)

Гарафики динамики изменения показаний за час и за день вызываются одиночным 
нажатием на кнопку.

-   на графических экранах двойным нажатием на кнопку переключаются макс/мин
    пределы между указанными значениями в прошивке и фактическим максимумом и
    минимумом за период (т.е. более наглядное графическое представление
    показателей). При этом с правой стороны графика появляются стрелочки
    (вовнутрь и наружу соответственно) в качестве индикации режима;

![9](https://github.com/Norovl/meteoClock/blob/master/media/136Grafco2max.jpg)
![10](https://github.com/Norovl/meteoClock/blob/master/media/136Grafco2.jpg)
![1602 график температуры](https://github.com/Norovl/meteoClock/blob/master/media/15c21_1602_graft.jpg)
![1602 график прогноза](https://github.com/Norovl/meteoClock/blob/master/media/15c21_1602_grafrain.jpg)

-   создано меню для тонкой настройки и сохранения выбранных параментров.
    Вызывается тройным нажатием на кнопку. Переход по меню - одинарное
    нажатие, выбор параметра - удержание кнопки.
	
	Структура меню настроек следующая:

		яркость индикатора
			- 0%
			- 10%
			- 20%
			...
			- 100%
			- АВТО
![Настройка яркости индикатора](https://github.com/Norovl/meteoClock/blob/master/media/158LEDBright.jpg)
![1602 настройка ярк.индикатора](https://github.com/Norovl/meteoClock/blob/master/media/15c21_1602_setbrind.jpg)

		яркость экрана
			- 0%
			- 10%
			- 20%
			...
			- 100%
			- АВТО
![Настройка яркости дисплея](https://github.com/Norovl/meteoClock/blob/master/media/158LCDBright.jpg)
![Установка яркости дисплея](https://github.com/Norovl/meteoClock/blob/master/media/158LCDBright10.jpg)
![1602 настройка ярк.экрана](https://github.com/Norovl/meteoClock/blob/master/media/15c21_1602_setbrLCD.jpg)
		
		режим индикатора (привязка к показаниям датчика)
			- СО2
			- влажность
			- температура
![2004 настройка реж.индикатора](https://github.com/Norovl/meteoClock/blob/master/media/15c21_2004_setregind.jpg)
![2004 настройка реж.индикатора2](https://github.com/Norovl/meteoClock/blob/master/media/15c21_2004_setregindhum.jpg)
		
		настройки графиков СО2 час (вкл/выкл)
![Включение отображения графика уровня СО2 за час](https://github.com/Norovl/meteoClock/blob/master/media/158GrafOn.jpg)
![1602 настройка график СО2 вкл/выкл](https://github.com/Norovl/meteoClock/blob/master/media/15c21_1602_onoffgrafco2.jpg)

		настройки графиков СО2 день (вкл/выкл)
		настройки графиков влажность час (вкл/выкл)
		настройки графиков влажность день (вкл/выкл)
![Отключение отображения влажности за сутки](https://github.com/Norovl/meteoClock/blob/master/media/158GrafOff.jpg)
![1602 настройка график влажности вкл/выкл](https://github.com/Norovl/meteoClock/blob/master/media/15c21_1602_onoffgrafhum.jpg)

		настройки графиков температура час (вкл/выкл)
		настройки графиков температура день (вкл/выкл)
		настройки графиков p,rain час (вкл/выкл)
		настройки графиков p,rain день (вкл/выкл)		
		
		сохранить
![Сохранение настроек](https://github.com/Norovl/meteoClock/blob/master/media/158Save.jpg)
	
		выход

В заголовке скетча можно установить пределы значений датчиков, при которых
индикатор будет менять свет. Так же задается предел для мигания индикатора.

Сохранение производится в энергонезависимую память, поэтому при следующем
включении все настройки восстановятся.

-   есть возможность работы от аккумулятора 18650 с индикацией остатка
    заряда. Так же отображается индикатор работы от сети. Необходима 
    доработка схемы – см. рисунок ниже.

![Работа от батареи](https://github.com/Norovl/meteoClock/blob/master/media/158battery.jpg)
![Работа от внешнего источника (сети)](https://github.com/Norovl/meteoClock/blob/master/media/158DC.jpg)

![Доработка схемы для работы от батареи](https://github.com/Norovl/meteoClock/blob/master/media/scheme1.jpg)



История изменений:

## v1.3b6

-   большие цифры на главном экране сделал более «квадратными», что в свою
    очередь позволило высвободить пару символов, которые использовал для
    написания двухбуквенного обозначения дня недели по-русски (например: ПН, ЧТ,
    ПТ, СБ);

![1](https://github.com/Norovl/meteoClock/blob/master/media/136Time.jpg)

-   из-за сокращения длины написания дня недели по-русски появилась возможность
    добавить рядом секунды;

-   выровнял большую цифру «1» по центру, что на мой взгляд выглядит более
    наглядно;

-   на главном экране двойным нажатием на кнопку в цикле переключаются режимы
    крупного отображения значений следующих показателей: время – содержание СО2
    – температура – давление;

![2](https://github.com/Norovl/meteoClock/blob/master/media/136co2.jpg)
![3](https://github.com/Norovl/meteoClock/blob/master/media/136temp.jpg)
![4](https://github.com/Norovl/meteoClock/blob/master/media/136mmhg.jpg)
![5](https://github.com/Norovl/meteoClock/blob/master/media/136h.jpg)
![6](https://github.com/Norovl/meteoClock/blob/master/media/136Time.jpg)

-   на главном экране удержанием кнопки переключается режим большие цифры/очень
    большие цифры – работает только для LCD 2004. В режиме четырехстрочных цифр
    некоторые данные не выводятся, т.к. просто нет места (прогноз дождя,
    влажность)

![7](https://github.com/Norovl/meteoClock/blob/master/media/136BigTime.jpg)
![8](https://github.com/Norovl/meteoClock/blob/master/media/136BigTemp.jpg)

-   на графических экранах при минимальных значениях были пропуски в графике.
    Добавил минимально отображаемое значения, чтобы график выглядел цельным.

-   на графических экранах двойным нажатием на кнопку переключаются макс/мин
    пределы между указанными значениями в прошивке и фактическим максимумом и
    минимумом за период (т.е. более наглядное графическое изменение
    показателей). При этом с правой стороны графика появляются стрелочки
    (вовнутрь и наружу соответственно) в качестве индикации режима;

![9](https://github.com/Norovl/meteoClock/blob/master/media/136Grafco2max.jpg)
![10](https://github.com/Norovl/meteoClock/blob/master/media/136Grafco2.jpg)

-   мелкие изменения, не влияющие на функциональность (например, на графике
    температуры за обозначением «t» поставил значок градуса и пр…)

## v1.3b7

Новая версия прошивки с возможностью сохранения вида настроек: текущие крупные
показания (время, содержание СО2, температура, давление, влажность), размер
крупных цифр (2-х строчные или 4-х строчные), пределы графиков (указанные в
прошивке или исходя из значений за период).

Меню сохранения вызывается тройным нажатием на кнопку, затем одинарным нажатием
выбирается ДА или НЕТ и длинным нажатием применяется. Все настройки сохраняются
в энергонезависимой памяти ардуинки, поэтому потеря питания теперь не страшна.

![11](https://github.com/Norovl/meteoClock/blob/master/media/137save.jpg)

поправлен баг наложения элементов часов (времени) на температуру, если она была
выбрана основной.

## v1.5b8

### v1.5b81

Изменения в меню (вызывается тройным нажатием кнопки в главном окне):

-   Добавлен пункт меню «Выход», чтобы можно было выйти не сохраняя значения в
    энергонезависимую память (однако выбранные параметры будут использоваться 
    до перезагрузки устройства)

-   Убрано подтверждение сохранения, теперь сохраняется сразу при удержании
    кнопки на меню «Сохранить»

![Сохранение настроек](https://github.com/Norovl/meteoClock/blob/master/media/158Save.jpg)

-   Добавлена настройка выбора яркости индикатора (0% - 100%, АВТО) и экрана 
    (0% - 100%, АВТО, но для регулировки яркости экрана необходима доработка
    схемы под версию 1.5 Алекса).

![Настройка яркости индикатора](https://github.com/Norovl/meteoClock/blob/master/media/158LEDBright.jpg)
![Настройка яркости дисплея](https://github.com/Norovl/meteoClock/blob/master/media/158LCDBright.jpg)
![Установка яркости дисплея](https://github.com/Norovl/meteoClock/blob/master/media/158LCDBright10.jpg)

-   Добавлена возможность настройки отображения и скрытия графиков при их
    последовательном переборе.

![Отключение отображения влажности за сутки](https://github.com/Norovl/meteoClock/blob/master/media/158GrafOff.jpg)
![Включение отображения графика уровня СО2 за час](https://github.com/Norovl/meteoClock/blob/master/media/158GrafOn.jpg)

-   Добавлена возможность работы от аккумулятора 18650 с индикацией остатка
    заряда. Так же есть индикатор работы от сети. Необходима доработка схемы –
    см. рисунок ниже.

![Работа от батареи](https://github.com/Norovl/meteoClock/blob/master/media/158battery.jpg)
![Работа от внешнего источника (сети)](https://github.com/Norovl/meteoClock/blob/master/media/158DC.jpg)

![Доработка схемы для работы от батареи](https://github.com/Norovl/meteoClock/blob/master/media/scheme1.jpg)

### v1.5b82

-   Теперь из любого пункта меню настроек можно выйти в пункт "Сохранить" 
    двойным нажатием на кнопку;

-   в заголовке скетча (blinkLED) можно установить порог значений СО2, при
    превышении которого будет мигать индикатор;

-   поправлена ошибка в режиме debug;

## v1.5c21

-   Добавлен пункт меню "РЕЖ.ИНДИКАТОРА", позволяющий выбрать привязку 
    индикатора к датчикам СО2, температуры и влажности (прогноз осадков 
    пока не привязан). Пределы значений датчиков для изменения цвета
    индикатора находятся в заголовке скетча.

![2004 настройка реж.индикатора](https://github.com/Norovl/meteoClock/blob/master/media/15c21_2004_setregind.jpg)
![2004 настройка реж.индикатора2](https://github.com/Norovl/meteoClock/blob/master/media/15c21_2004_setregindhum.jpg)

-   оптимизирован код для экономии памяти.

-   убрано мерцание при перерисовке крупных цифр.

-   в режиме отладки (debug = 1) информация о датчиках отображается в 
    течение 10 секунд, затем происходит переход в рабочий режим.

Расширена поддержка экранов 1602: 

![1602 главный экран](https://github.com/Norovl/meteoClock/blob/master/media/15c21_1602_main.jpg)

-   добавлен режим крупных цифр с отображением времени, СО2, температуры,
    давления, влажности. Переключиться в этот режим и обратно можно
    удержанием кнопки на главном экране. Переключение датчиков производится
    двойным нажатием на кнопку.

![1602 крупное время](https://github.com/Norovl/meteoClock/blob/master/media/15c21_1602_time.jpg)
![1602 крупная температура](https://github.com/Norovl/meteoClock/blob/master/media/15c21_1602_temp.jpg)

-   добавлена возможность отображения графиков как и на большом экране.
    Переключение по графикам производится одинарным нажатием на кнопку.

![1602 график температуры](https://github.com/Norovl/meteoClock/blob/master/media/15c21_1602_graft.jpg)
![1602 график прогноза](https://github.com/Norovl/meteoClock/blob/master/media/15c21_1602_grafrain.jpg)

-   теперь меню доступно и на малом экране. Вызывается тройным нажатием
    на кнопку.

![1602 настройка ярк.индикатора](https://github.com/Norovl/meteoClock/blob/master/media/15c21_1602_setbrind.jpg)
![1602 настройка ярк.экрана](https://github.com/Norovl/meteoClock/blob/master/media/15c21_1602_setbrLCD.jpg)
![1602 настройка график СО2 вкл/выкл](https://github.com/Norovl/meteoClock/blob/master/media/15c21_1602_onoffgrafco2.jpg)
![1602 настройка график влажности вкл/выкл](https://github.com/Norovl/meteoClock/blob/master/media/15c21_1602_onoffgrafhum.jpg)