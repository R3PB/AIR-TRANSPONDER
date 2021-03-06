# AIR-TRANSPONDER
**Simple aviation transponder system. Простая система передачи и приёма параметров полёта**

**Система:** транспондер - базовая станция.

**Предназначение:** Передача параметров полёта воздушного судна на базовую станцию по радиоканалу, без использования операторов связи. Отображение на офлайн карте, в режиме реального времени, меток с указанием параметров полёта

**Возможности:** Использование до 10 транспондеров с одной базовой станцией.
Передаются транспондером и принимаются базовой станцией, в режиме реального времени, параметров: 
* Место,
* Высота,
* Скорость,
* Скороподъёмность,
* Курс,
* Номер борта.

Колличество одновременно работающих транспондеров 10.

Производится запись параметров в базу данных.

Отображение на офлайн карте, в режиме реального времени, меток с указанием параметров полёта, а так же:
* Удаление от точки старта.
* Название ближайшего населённого пункта и расстояние до него.

Отображение метеоданных на точке старта:
* Направление ветра
* Скорость ветра
* Порывы Ветра
* Температура

## Требования.
  Базовая станция:
  * Декодер данных. ESP8266(либо Arduino UNO с ETH шильдом)
  * Сервер. Ноутбук с ОС LINUX.
  * Ресивер. В качестве приёмника используется носимая портативная радиостанция 144/433MHz
  * Антенна стационарная внешняя с круговой диаграммой 
  * WIFI роутер(для удобства подключения).
  <img class="float-right" src="Transmitter_AIR/img/IMG_20200711_192412(1).jpg" alt="AIR_TRANSPONDER BASE, server, decoder" width="" />
  
  Транспондер:
  * GPS модуль GPS NEO-6M-0-001,
  * Кодер данных - Arduino Nano,
  * Трансмиттер. В качестве передатчика используется носимая портативная радиостанция 144/433MHz.
  <img class="float-right" src="Transmitter_AIR/img/WhatsApp Image 2021-02-12 at 22.47.49.jpeg" alt="AIR_TRANSPONDER FT-11r, coder" width="350" />


Модуляция FSK (FM).
Подключение кодера/декодера к приёмнику/передатчику простое, с помощью аудиокабеля. Используются штатные разъемы радиостанций.
Подключение к серверу базовой станции: компьютер/смартфон ethernet кабель либо wifi. 
Данные отображаются на экране в виде интеракивной оффлайн карты месности, где производятся полёты. Активные метки на карте отображают положение(и прочие параметры полёта) воздушных средств находящихся в воздухе в реальном времени. Время смещения(задержки) 4 секунды. Дальность действия транспондера определяется качеством трансмиттера и приёмника. 
## Лётные испытания
Для тестирования и отладки системы, транспондер устанавливался на планёр Blanik L13. 
<img class="float-right" src="Transmitter_AIR/img/IMG_20200711_194358(1).jpg" alt="AIR_TRANSPONDER BLANIK L13" /> Базовая станция разворачивалась на СКП - точке старта, в полевых условиях. Летные испытания показали что уверенный приём, передаваемых параметров полёта, осуществляется на расстоянии 8 км и высотой полёта не вышее 1200 метров, при мощности излучения 100 милливат. На большие расстояния тесты пока не проводились.
<img src="https://github.com/R3PB/AIR-TRANSPONDER/blob/main/Transmitter_AIR/img/IMG_20200711_192203(1).jpg" alt="AIR_TRANSPONDER отображение на карте" />
.
## Благодарность
Отдельное специальное спасибо INVANDY. Начало было положенно благодаря его проекту https://github.com/invandy/Oregon_NR

## ОБНОВЛЕНИЕ
17.03.21 
* Улучшена функция timeslot. Теперь при рассинхронизации с GPS, транспондер не уйдёт в свободное плавание по каналам. Будет прекращена передача не обновлённых данных.
* Изменена индикация GPS led. При потересигнала GPS led начинает часто мигать. При восстановлении горит постоянно.
* Исправлен баг с зависшим индикатором TX при отсутсвующих обновлённых данных от GPS.