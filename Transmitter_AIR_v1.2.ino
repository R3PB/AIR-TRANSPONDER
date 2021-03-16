#include <Oregon_TX_AIR.h>
#include <SoftwareSerial.h>
#include <TinyGPS.h>

//#define AIR1

// GPS модуль 
SoftwareSerial gpsSerial(5, 4);//rx,tx  
TinyGPS gps; // создать gps объект
    bool newdata = false;
    bool tx_flag = false;
    unsigned long start;
    long lat, lon;
    float falt;
    unsigned long fix_age, time, date, Second;
    int year;
    byte month, day, hour, minute, second, hundredths;
    uint32_t mysecond, second_, _second_, milli_s, timer1, timer2;
    #define PERIOD 1000 // период отсчёта миллисек. при синхронизации от 00 сек
// END

// ** Пример от Invandy** - Имитатор датчиков температуры и влажности Oregon Scientific
// В данном примере иммитируются пакеты следующих типов датчиков:
// THGN132N (тип 1D20h, протокол v2, 3 канала) 
// RTGN318  (тип ХСС3h, протокол v2, 5 каналов)
// THGR810  (тип F824h, протокол v3, 10 каналов)
// Вывод передатчика один. Указывается только при создании первого объекта. В данном примере передатчик подключен к D6
// Oregon_TM transmitter(6), transmitter2, transmitter3, transmitter4;
// END ////////////////////////////////////////////////
Oregon_TM transmitter, transmitter2, transmitter3, transmitter4;

// Массив режимов работы ЗЕЛЁНОГО светодиода(ИНДИКАТОР ПРИЁМА GPS)
byte modes[] = {
   0B00000000, //Светодиод выключен
   0B11111111, //Горит постоянно
   0B00001111, //Мигание по 0.5 сек
   0B00000001, //Короткая вспышка раз в секунду
   0B00000101, //Две короткие вспышки раз в секунду
   0B00010101, //Три короткие вспышки раз в секунду
   0B01010101  //Частые короткие вспышки (4 раза в секунду)
};
  uint32_t ms, ms1 = 0, ms2 = 0;
  uint8_t  blink_loop = 0;
  uint8_t  blink_mode;
  uint8_t  modes_count; 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void setup()
{
  Serial.begin(115200);     
  Serial.println("Waiting data of GPS..."); 
  gpsSerial.begin(9600);            // Подключаем GPS приёмник на указанной скорости
  pinMode(GPS_LED, OUTPUT);         // ИНДИКАЦИЯ GPS ПРИЁМА - ЗЕЛЁНЫЙ СВЕТОДИОД 
  digitalWrite(GPS_LED, HIGH); 
  
  // Пример от Invandy /////////////////////////////////////////
  // Первый передатчик /////////////////////////////////////////
  transmitter.setType(THGN132);
  transmitter.setChannel(3);        // Номер канала для THGN132 - 1...3
  transmitter.setBatteryFlag(1);    // Флаг разряженной батарейки
  transmitter.setTemperature(24.2); // -50C...+70C
  transmitter.setHumidity(13);      // 2...98%
  transmitter.setComfort(24.2, 13); // Расчёт передаваемого индекса комфорта

  //Второй передатчик
  transmitter2.setType(THGR810);    
  transmitter2.setChannel(1);       // Номер канала для THGR810 - 1...10 (Возможно приниматься базой будут тольок первые 5 каналов. Проверить мне не на чем...)
  transmitter2.setBatteryFlag(1); 
  transmitter2.setTemperature(+49.9); 
  transmitter2.setHumidity(98); 
  transmitter2.setComfort(+49.9, 98); 

  //Третий передатчик
  transmitter3.setType(RTGN318);
  transmitter3.setChannel(5);       // Номер канала для RTGN318 - 1...5. 
  transmitter3.setBatteryFlag(0); 
  transmitter3.setTemperature(-31); 
  transmitter3.setHumidity(50); 
  transmitter3.setComfort(-31,50); 
  // END //////////////////////////////
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void loop()
{
  newdata = readgps();               // Чтение данных GPS приёмника
  sync_with_time();                  // Синхронизация таймера с GPS
      if (newdata)
      {
        start = millis();
        gps.get_position(&lat, &lon);
        gps.get_datetime(&date, &time, &fix_age);
        falt = gps.f_altitude();     // +/- altitude in meters
        green_led(modes[1]);         // Включаем индикацию приёма GPS. LED постоянно горит - GPS приём есть.
        Serial.print("Lat: "); Serial.print(lat);
        Serial.print(" Long: "); Serial.print(lon);
        Serial.print(" Altitude: "); Serial.print(falt);
        Serial.print(" Date: "); Serial.print(date);
        Serial.print(" Time: "); Serial.println(time);
        Serial.print(" Real_Time секунды: ");Serial.println(second);
      }
    //передача осуществляется по таймеру, который определяется типом датчика и номером канала
    transmitter4.setType(AIRx);
    transmitter4.setChannel(1);       // tiimeslot. формируется номер транспондера(Сейчас этот параметр формирует номер канала - timeslot. 27.07.20 - флаг, пока, не принимает участия в формировании номера тайм-слота)
    transmitter4.setLatitude(lat);
    transmitter4.setLongitude(lon);
    transmitter4.setAltitude(falt);
    
    //if (transmitter.transmit()) PrintSentData(transmitter.SendBuffer);
    //if (transmitter2.transmit())PrintSentData(transmitter2.SendBuffer);
    //if (transmitter3.transmit()) PrintSentData(transmitter3.SendBuffer);
    if ( tx_flag == true && transmitter4.transmit(second_)) PrintSentData(transmitter4.SendBuffer);  //параметры (second, offset_t) - текущее время(только секунды), и время смещения(таймслот)
}
// ИНДИКАЦИЯ GPS ЗЕЛЁНЫЙ  СВЕТОДИОД #########################
    void green_led(uint8_t blink_mode){           // Пеердаём в воид параметр - режим индикации
      ms = millis();
      ms1 = ms; 
      if( ( ms - ms2 ) > 150 ){                   // Параметр (150) определяет частоту мигания. Чем меньше значение - тем чаще.
         ms2 = ms;
         digitalWrite(GPS_LED, HIGH);             // Высокое значение на выводе - гасим LED
         if(  blink_mode & 1<<(blink_loop&0x07) ) // Режим светодиода ищем по битовой маске
         digitalWrite(GPS_LED, LOW);
         blink_loop++;
      }
    }
   
// GPS void ###################################################
// проверка наличия данных
    bool readgps()
    {
      while (gpsSerial.available()){
        int b = gpsSerial.read();
        //в библиотеке TinyGPS имеется ошибка: не обрабатываются данные с \r и \n
        if('\r' != b){
        if (gps.encode(b))
        return true;
        }
      }
    return false;
    }
    
// СИНХРОНИЗАЦИЯ С НУЛЕВОЙ СЕКУНДОЙ - void ######################
void sync_with_time()
{
  gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths);
  // second = second % 5; - эта процедура превращает секунды 00 - 59, полученные от tinyGPS в десятичном виде, в 5-ти секундные циклы 0,1,2,3,4,0,1,2,3,4,0, и.т.д. 
  // путем деления с остатком который и заносится в переменную. Важное замечание! переменную second необходимо представить типом byte!
  // Это самый короткий путь чтобы отбросить старший разряд, тоесть отбросить десятки секунд и оставить только цикл от 0 до 4
  _second_ = second % 5 ;             // Секунды полученные от GPS 0 ... 59, переводим в 0,1,2,3,4,0,1,2,3,4,0.... и т.д..  
  if (newdata){                       // Если с GPS есть новая отметка времени, то:
  tx_flag = true;
  timer1 = millis();                  // в этот момент(на переходе 0-ой отметки между секундами) СБРАСЫВАЕМ/устанавливаем timer1 = значению millis() 
  }else{                              // Впротивном случае выполняем:
//  tx_flag = false;
  timer2 = millis() - timer1;         // отсчёт millis() сразу после перехода через 0 (0,1,2,3,4,5,6,7,8,9,10,11, .... 999)
  second_ = _second_ * 1000 + timer2; // Сращиваем секунды в диаппазоне - 0,1,2,3,4,0 и миллисекунды в диаппазоне от 0 до 999(если произошёл сброс timer1), или до  бесконечности если сброса не было!
  if (second_ >= 5000){               // Если произошла рассинхронизация таймера, значит нет данных от GPS. Тогда:
//    tx_flag = false;
    second_ = 0;                      // Сбрасываем таймер в 0. Тем самым запрещаем циклическую передачу повторяющихся данных
    green_led(modes[6]);              // Частое мигание зелёным LED. 4 раза в секунду - "нет приёма GPS"
  }
//  if (second_ >= 32000){              // Если произошла рассинхронизация таймера, значит нет данных от GPS. Тогда:
//    second_ = 10000;                      // Сбрасываем таймер в 0. Тем самым запрещаем циклическую передачу повторяющихся данных
//    tx_flag = false;
//    green_led(modes[0]);              // Отсутствие индикации зелёного LED."нет приёма GPS более чем полминуты"
//  }
//  }
  }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PrintSentData(byte* buf)
{
  Serial.print(millis() / 1000);
  Serial.print("s \t\t");
  for (byte i = 0; i < 10; ++i) 
  {
    byte trmbuf = *buf;
    Serial.print(trmbuf >> 4, HEX);
    if (i < (10 - 1) )
    Serial.print(trmbuf & 0x0F, HEX);
    buf++;
  }  
  Serial.println();
}
