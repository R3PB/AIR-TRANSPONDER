#include <Oregon_TX_AIR.h>
#include <SoftwareSerial.h>
#include <TinyGPS.h>

//#define AIRx    0xAAA5
int offset_t = 3600; // Миллисекунды. Время смещения(таймслот транспондера) указывать в интервале от 0 до 4500 шагом 500.
//byte AIRx = AIR2;
// GPS модуль
//float lat, lon; // создать переменную для объекта широты и долготы  
SoftwareSerial gpsSerial(5, 4);//rx,tx  
//SoftwareSerial gpsSerial(14, 13);//rx,tx
TinyGPS gps; // создать gps объект
    bool newdata = false;
    unsigned long start;
    long lat, lon;
    //byte alt;
    float falt;
    unsigned long fix_age, time, date, Second;
    int year;
    byte month, day, hour, minute, second, hundredths;
    uint32_t mysecond, second_, milli_s, timer1;
//    uint32_t timer, timer1;
    #define PERIOD 1000 // период отсчёта миллисек. при синхронизации от 00 сек
//    static byte GPS_LED  = 16;
// END
//--------------------------
//static byte search_pin = 3;
//pinMode(search_pin, OUTPUT);
//digitalWrite(search_pin, HIGH);
//--------------------------
//Имитатор датчиков температуры и влажности Oregon Scientific

//В данном примере иммитируются пакеты следующих типов датчиков:
//THGN132N (тип 1D20h, протокол v2, 3 канала) 
//RTGN318  (тип ХСС3h, протокол v2, 5 каналов)
//THGR810  (тип F824h, протокол v3, 10 каналов)
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Вывод передатчика один. Указывается только при создании первого объекта. В данном примере передатчик подключен к D6

//Oregon_TM transmitter(6), transmitter2, transmitter3, transmitter4;
Oregon_TM transmitter, transmitter2, transmitter3, transmitter4;
//tr_pin = 16;

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
  uint8_t  blink_mode = 0;
  uint8_t  modes_count = 0; 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void setup()
{
// --------------------------
//static byte search_pin = 14;
//pinMode(search_pin, OUTPUT);
//digitalWrite(search_pin, LOW);
// --------------------------
  Serial.begin(115200);     
  Serial.println("Waiting data of GPS..."); 
  gpsSerial.begin(9600); // подключаем gps датчик
  
  //Первый передатчик
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
  
/// ИНДИКАЦИЯ GPS ПРИЁМА - ЗЕЛЁНЫЙ СВЕТОДИОД /////
  pinMode(GPS_LED, OUTPUT);
  digitalWrite(GPS_LED, HIGH); 
  modes_count = 1;
  blink_mode = modes[modes_count];

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void loop()
{
 newdata = readgps();
// gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths);
sync_with_time();
//  gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths);
//  second = second % 5 ;
//  minute = minute % 5 ;
//  //#define PERIOD 1000
//  if (second % 1  == 0){
//  milli_s = millis() - timer1;
//  second_ = second * 1000 + milli_s;
//  if((millis() - timer1) >= PERIOD ){
//    if(milli_s >= PERIOD){
//      timer1 = millis();
//      }
//    }
//  }
////#################################
//byte f_time = second + 10;
////#################################
//{
    //GPS mr.Tech
    if (millis() - start > 900) //установка задержки в одну секунду между обновлением данных
    {
      //gpsSerial.read();
//      newdata = readgps();
      if (newdata)
      {
        start = millis();
        gps.get_position(&lat, &lon);
        gps.get_datetime(&date, &time, &fix_age);
//        gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths);
//        int mysecond = int(second); 
//        second = second % 5;   //  %10 - эта процедура превращает секунды 00 - 59, полученные от tinyGPS в десятичном виде, в секунды от 0 до 9 (%5 => 0 ... 4), 
                                  // путем деления с остатком который и заносится в переменную. Важное замечание! переменную second необходимо представить типом byte!
                                  // Это самый короткий путь чтобы отбросить старший разряд, тоесть отбросить десятки секунд и оставить только цикл от 0 до 9
        falt = gps.f_altitude(); // +/- altitude in meters
        green_led();
        Serial.print("Lat: "); Serial.print(lat);
        Serial.print(" Long: "); Serial.print(lon);
        Serial.print(" Altitude: "); Serial.print(falt);
        Serial.print(" Date: "); Serial.print(date);
        Serial.print(" Time: "); Serial.println(time);
        Serial.print(" Real_Time секунды: ");Serial.println(second);
        Serial.print(" Offset_Time: ");Serial.println(offset_t);
//        Serial.print(" Real_Time минуты: "); Serial.println(minute);
        Serial.print(" MILLI_S: "); Serial.println(milli_s);
        Serial.print(" SECOND_: "); Serial.println(second_);
      }
    }

  /////////////////////
    //передача осуществляется по таймеру, который определяется типом датчика и номером канала
    transmitter4.setType(AIRx);
    transmitter4.setChannel(1); // tiimeslot. формируется номер транспондера(Сейчас этот параметр формирует номер канала - timeslot. 27.07.20 - флаг, пока, не принимает участия в формировании номера тайм-слота)
    transmitter4.setLatitude(lat);
    transmitter4.setLongitude(lon);
    transmitter4.setAltitude(falt);
    
//    if (transmitter.transmit()) PrintSentData(transmitter.SendBuffer);
//    if (transmitter2.transmit())PrintSentData(transmitter2.SendBuffer);
//    if (transmitter3.transmit()) PrintSentData(transmitter3.SendBuffer);
    if (transmitter4.transmit(second_, offset_t)) PrintSentData(transmitter4.SendBuffer);  //параметры (second, offset_t) - текущее время(только секунды), и время смещения(таймслот)
//        Serial.print(" MILLI_S: "); Serial.println(milli_s);
//green_led();
}
//////////////////// ИНДИКАЦИЯ GPS ЗЕЛЁНЫЙ  СВЕТОДИОД ////////////////////
    void green_led(){
//      blink_mode = modes[3];
    ms = millis();
    // Событие срабатывающее каждые 125 мс   
//    if( ( ms - ms1 ) > 125|| ms < ms1 ){
       ms1 = ms;
    // Режим светодиода ищем по битовой маске       
       if(  blink_mode & 1<<(blink_loop&0x07) ) digitalWrite(GPS_LED, LOW); 
       else  digitalWrite(GPS_LED, HIGH);
       blink_loop++;    
//    }
     blink_mode = modes[2];
    // Этот код служит для демонстрации переключения режимов    
    // Один раз в 5 секунд меняем эффект   
//    if( ( ms - ms2 ) > 1000|| ms < ms2 ){
//       ms2 = ms;
//       blink_mode = modes[3];
////       blink_mode = modes[modes_count++];
////       if( modes_count >= 7 )modes_count = 1;
//    }
   }
   
// mr.Tech GPS void #########################
// проверка наличия данных
    bool readgps()
    {
    while (gpsSerial.available())
    {
    int b = gpsSerial.read();
    //в библиотеке TinyGPS имеется ошибка: не обрабатываются данные с \r и \n
    if('\r' != b)
    {
    if (gps.encode(b))
     return true;
    }
    }
    return false;
    }

// END
// синхроизация с нулевой секундой
void sync_with_time()
{
  gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths);
  second = second % 5 ;
  //minute = minute % 5 ;
  //#define PERIOD 1000
  if (second){
  milli_s = millis() - timer1;
  second_ = second * 1000 + milli_s;
  if((millis() - timer1) >= PERIOD ){
      timer1 = millis();
    }
  }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PrintSentData(byte* buf)
{
      Serial.print(millis() / 1000);
      Serial.print("s \t\t");
      for (byte i = 0; i < 10; ++i) //original
      //for (byte i = 0; i < 11; ++i)  //mr.Tech 
      {
        byte trmbuf = *buf;
        Serial.print(trmbuf >> 4, HEX);
        if (i < (10 - 1) )//original
        //if (i < (11 - 1) )//mr.Tech
        Serial.print(trmbuf & 0x0F, HEX);
        buf++;
      }  
      Serial.println();
}
