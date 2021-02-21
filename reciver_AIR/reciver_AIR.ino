#include <Oregon_NR_AIR.h>
//#include <Oregon_TR.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>

//#define UNO

#ifdef UNO
//Oregon_NR oregon(2, 0, 13); // Для Arduino UNO/Nano - приёмник на выводе D2, Прерывание 0, Светодиод приёма на вывод 13
Oregon_NR oregon(2, 0); // Если светодиод не нужен
#endif

#define WEMOS_D1

#ifdef WEMOS_D1
Oregon_NR oregon(13, 13, 2, true); // для Wemos D1 - датчик на выводе D7 (GPIO13), Светодиод на D2 подтянут к +пит.
#endif

// Индикация ----------------------------- //
#define BLUE_LED 2      //Индикация подключения к WiFi
//#define GREEN_LED 14    //Индикатор успешной доставки пакета а народмон
//#define RED_LED 255     //Индикатор ошибки доставки пакета на народмон
// -------------------------------------- //

// Интервалы соединения с сервером ------ //
#define SEND_INTERVAL 120000 //350000 - 5 min   //Как часто отсылать данные на сервер
#define CONNECT_TIMEOUT 10000         //Время ожидания  соединения
#define DISCONNECT_TIMEOUT 10000      //Время ожидания отсоединения
// -------------------------------------- //

//Параметры входа в WiFi ---------------- //
const char* ssid = "ASUS";           
const char* password = "123456789";
//--------------------------------------- //

//Параметоы соединения с hitech -------- //
//IPAddress nardomon_server(192.168.1.202);
char nardomon_server[] = "hitech"; 
int port=3000;
WiFiClient client; //Клиент hitech
//-------------------------------------- //

// инициализация переменных для хранения данных с транспондеров
// Транспондер ID ------------------------ //
//long transponder_id = AIR1;
long transponder_id;
//const char* transponder_name = transponder_id;
// --------------------------------------- //
float latitude_, longitude_;
byte curs_, vario_;
word altitude_, speed_;
// ------------------------------------- //
void setup() {
   Serial.begin(115200);
//  вкючение прослушивания радиоканала  
//  oregon.start(); 
//  oregon.receiver_dump = false;       //true - Включает "осциллограф" - отображение данных, полученных с приёмника
//  
//  //Если не распознаются последние байты принятого пакета можно попробовать подстроить частоту захвата данных
//  oregon.timing_correction = 0;       // коррекция частоты завхвата данных. Обычно достаоточно подобрать значение от -5 до 5
//  oregon.decode_method = 3;           // или использовать метод 3 для декодирования



  // Запуск Wifi --------------------------------------------
  //wifi_connect();
  WiFi.mode(WIFI_OFF);                //Prevents reconnection issue (taking too long to connect)
  delay(1000);
  WiFi.mode(WIFI_STA);                //Отключаем hotspot и переводим ESP в режим wifi клиент. В хотспот работает  как каждый с каждым - иногда выручает :)
  WiFi.begin(ssid, password);         //Соединяемся с роутером
  Serial.println("");
  Serial.print("Connecting");
  // ждём соединения
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  //If connection successful show IP address in serial monitor
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  //IP address assigned to your ESP
  // --------------------------------------------------------

  //--------вкючение прослушивания радиоканала -----------------------------------------------------------------------  
  oregon.start(); 
  oregon.receiver_dump = false;       // true - Включает "осциллограф" - отображение данных, полученных с приёмника
                                      // Если не распознаются последние байты принятого пакета можно попробовать подстроить частоту захвата данных
  oregon.timing_correction = 0;       // Коррекция частоты завхвата данных. Обычно достаоточно подобрать значение от -5 до 5
  oregon.decode_method = 3;           // или использовать метод 3 для декодирования
  //-------------------------------------------------------------------------------------------------------------------
}
void loop() {
   /////////////////////////////////////////////////////

  //////////////////////////////////////////////////////////////////////
  //Захват пакета,/////////////////////////////////////////////////////
  oregon.capture(0); // 1 - выводить в Serial сервисную информацию
  
  //Захваченные данные годны до следующего вызова capture
  //ОБработка полученного пакета//////////////////////////////////////////////
  if (oregon.captured)  {
    //Вывод информации в Serial
    Serial.print ((float) millis() / 1000, 1); //Время
    Serial.print ("s\t\t");
    //Версия протокола
    if (oregon.ver == 2) Serial.print("  ");
    if (oregon.ver == 3) Serial.print("3 ");
    
    //Информация о восстановлени пакета
    if (oregon.restore_sign & 0x01) Serial.print("s"); //восстановлены одиночные такты
    else  Serial.print(" ");
    if (oregon.restore_sign & 0x02) Serial.print("d"); //восстановлены двойные такты
    else  Serial.print(" ");
    if (oregon.restore_sign & 0x04) Serial.print("p "); //исправленна ошибка при распознавании версии пакета
    else  Serial.print("  ");

    //Вывод полученного пакета. Точки - это ниблы, содержащие сомнительные биты
    for (int q = 0;q < PACKET_LENGTH - 1; q++)
      if (oregon.valid_p[q] == 0x0F) Serial.print(oregon.packet[q], HEX);
      else Serial.print(".");
        
    //Время обработки пакета
    Serial.print("  ");
    Serial.print(oregon.work_time);
    Serial.print("ms ");
    
    if ((oregon.sens_type == THGN132 || (oregon.sens_type & 0x0FFF) == RTGN318 || oregon.sens_type == THGR810 || oregon.sens_type == THN132) && oregon.crc_c){
      Serial.print("\t");
      Serial.print(" TYPE: ");
      if (oregon.sens_type == THGN132) Serial.print("THGN132N");
      if (oregon.sens_type == THGR810) Serial.print("THGR810 ");
      if ((oregon.sens_type & 0x0FFF) == RTGN318) Serial.print("RTGN318");
      if (oregon.sens_type == THN132) Serial.print("THN132N ");
      Serial.print(" CHNL: ");
      Serial.print(oregon.sens_chnl);
      if (oregon.sens_tmp >= 0 && oregon.sens_tmp < 10) Serial.print(" TMP:  ");
      if (oregon.sens_tmp < 0 && oregon.sens_tmp >-10) Serial.print(" TMP: ");
      if (oregon.sens_tmp <= -10) Serial.print(" TMP:");
      if (oregon.sens_tmp >= 10) Serial.print(" TMP: ");
      Serial.print(oregon.sens_tmp, 1);
      Serial.print("C ");
      if (oregon.sens_type == THGN132 || oregon.sens_type == THGR810 || (oregon.sens_type & 0x0FFF) == RTGN318) {
        Serial.print("HUM: ");
        Serial.print(oregon.sens_hmdty, 0);
        Serial.print("%");
      }
      else Serial.print("        ");
      Serial.print(" BAT: ");
      if (oregon.sens_battery) Serial.print("F "); else Serial.print("e ");
      Serial.print("ID: ");
      Serial.print(oregon.sens_id, HEX);
    }

  if (oregon.sens_type == WGR800 && oregon.crc_c){
      Serial.print("\t");
      float wnddata;
      Serial.print(" TYPE: ");


      
      Serial.print("WGR800");
      Serial.print(" AVG WS: ");
      Serial.print(oregon.sens_avg_ws, 1);
      Serial.print("m/s MAX WS: ");
      Serial.print(oregon.sens_max_ws, 1);
      Serial.print("m/s WDIR: "); //N = 0, E = 4, S = 8, W = 12
      Serial.print(oregon.sens_wdir);
      Serial.print(" BAT: ");
      if (oregon.sens_battery) Serial.print("F "); else Serial.print("e ");
      Serial.print("ID: ");
      Serial.print(oregon.sens_id, HEX);
      //      Serial.print("checksumm = ");
      //      Serial.print(oregon.crc_c, HEX);
      //      Serial.print(oregon.sens_id);
    }    

    if (oregon.sens_type == UVN800 && oregon.crc_c){
      Serial.print("\t");
      float wnddata;
      Serial.print(" TYPE: ");
      Serial.print("UVN800");
      Serial.print(" UV IDX: ");
      Serial.print(oregon.UV_index);
      Serial.print(" BAT: ");
      if (oregon.sens_battery) Serial.print("F "); else Serial.print("e ");
      Serial.print("ID: ");
      Serial.print(oregon.sens_id, HEX);
    }  
    
    if (oregon.sens_type == PCR800 && oregon.crc_c){
      Serial.print("\t");
      float wnddata;
      Serial.print(" TYPE: ");
      Serial.print("PCR800,");
      Serial.print(" RAIN RATE: ");
      Serial.print(oregon.rain_rate, 3);
      //Serial.print(" | ");
      //Serial.print(oregon.rain_rate, 1);
      Serial.print(" mm/h, ");
      Serial.print(" TOTAL RAIN: ");
      Serial.print(oregon.total_rain, 5);
      //Serial.print(" | ");
      //Serial.print(oregon.total_rain, DEC);
      Serial.print(" mm, ");
      Serial.print(" BAT: ");
      if (oregon.sens_battery) Serial.print("F "); else Serial.print("e ");
      Serial.print("ID: ");
      Serial.print(oregon.sens_id, HEX);
      //Serial.print(oregon.oregon_data);
      //Serial.print("checksumm = ");
      //Serial.print(oregon.crc_c, HEX);
      //Serial.print(oregon.sens_id);
    }  
    //####################################
    // Определяем номер транспондера. Эта функция необхобима для возможности использования сразу нескольких транспондеров с различными номарами от 1 до 9(AAA1, AAA2, ... AAA9)
    if (oregon.transponder == AIR && oregon.crc_c){   // если префикс(первых три символа) соответствует значению AAA и есть контрольная сумма, 
      transponder_id = (oregon.sens_type);            // то используем уже весь префикс (плюсь одна цифра) это четыре символа.
//        Serial.print((oregon.sens_type), HEX);      // Таким образом полный префикс транспондера заносим в переменную.
//        Serial.println(" - transponder AAA");
    }
    //####################################
    if (oregon.sens_type == transponder_id && oregon.crc_c){
      latitude_   = ((float(oregon.lat_)/1000));      // устанавливаем запятую после третьего знака с конца - деление на 1000
      longitude_  = ((float(oregon.lon_)/1000));    
      altitude_   = (oregon.alt_);  
      // на перспективу + три параметра
      //curs_     = (oregon.cur_);    // курс
      //vario_    = (oregon.var_);    // вариометр
      //speed_    = (oregon.speed_);  // скорость
      Serial.print("\t");
      Serial.print(" Transponder: ");
      Serial.print((transponder_id), HEX);
      Serial.print("; Latitude: ");
      Serial.print((latitude_), 3);   // Отображаю с точностью 3 знака после запятой
      Serial.print("; Longitude: ");  //
      Serial.print((longitude_),3);   // Отображаю с точностью 3 знака после запятой
      Serial.print(" Altitude: "); 
      Serial.print(oregon.alt_);    // Serial.print(" метров над уровнем моря");   
      Serial.println(" m;");
//      Serial.println(oregon.tr_number);
      send_post();                    // Отправляем данные методом POST на сервер в БД
    }    
    
//#if ADD_SENS_SUPPORT == 1
//      if ((oregon.sens_type & 0xFF00) == THP && oregon.crc_c) {
//      Serial.print("\t");
//      Serial.print(" TYPE: ");
//      Serial.print("THP");
//      Serial.print(" CHNL: ");
//      Serial.print(oregon.sens_chnl);
//      if (oregon.sens_tmp > 0 && oregon.sens_tmp < 10) Serial.print(" TMP:  ");
//      if (oregon.sens_tmp < 0 && oregon.sens_tmp > -10) Serial.print(" TMP: ");
//      if (oregon.sens_tmp <= -10) Serial.print(" TMP:");
//      if (oregon.sens_tmp >= 10) Serial.print(" TMP: ");
//      Serial.print(oregon.sens_tmp, 1);
//      Serial.print("C ");
//      Serial.print("HUM: ");
//      Serial.print(oregon.sens_hmdty, 1);
//      Serial.print("% ");
//      Serial.print("PRESS: ");
//      Serial.print(oregon.sens_pressure, 1);
//      Serial.print("Hgmm");
//      Serial.print(" BAT: ");
//      Serial.print(oregon.sens_voltage, 2);
//      Serial.print("V");
//    }
//#endif
    Serial.println();
  }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////
//=======================================================================
void send_post() {
  HTTPClient http;    //Declare object of class HTTPClient
 //WiFiClient client

  String ADCData, station, postData;
  //int adcvalue=analogRead(A0);  //Read Analog value of LDR
  //ADCData = String(adcvalue);   //String to interger conversion
  //  int adcvalue = 12345;
  //ADCData = String(adcvalue);   //String to interger conversion
  //station = "A";
  
  //Post Data
  //postData = "status=" + ADCData + "&station=" + station ;
//  ADCData = String(latitude_);

  //String s_latitude_   = String(latitude_);     // преобразование полученных данных в String необходимо для того, что бы можно было передать POST запрос
  //String s_longitude_  = String(longitude_);    // если не преобразовать, то сервер примет этот параметр пустым
  //String s_altitude_   = String(altitude_);     //
  //String s_speed_    = String(speed_);        // speed_ - параметр пока не определён. Это на перспективу. 
//static float latitude_ = 23.679;
  char str_temp[5];
//  char f_lat;
  /* 4 is mininum width, 2 is precision; float value is copied onto str_temp*/
  String f_lat = dtostrf(latitude_,5, 3, str_temp); // float to string  - пока не удалять! Может понадобиться если возникнут проблемы с передачей POST
  //sprintf(latitude_,"%s F", str_temp);
  //String temp2 = String(str_temp);
  String s_latitude_ = String(latitude_, 3);
  String s_longitude_ = String(longitude_, 3);
  postData = "transponder=AIR1&latitude=" + s_latitude_ + "&longitude=" + s_longitude_ + "&altitude=" + String(altitude_);
  
//  http.begin("http://hitech:3000/post_to_db");              //Specify request destination
  http.begin("http://192.168.1.239:3000/post_to_db");              //Specify request destination
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");    //Specify content-type header
  int httpCode = http.POST(postData);   //Send the request
  String payload = http.getString();    //Get the response payload
  
  Serial.print("Request respons: ");
  Serial.print(httpCode);   //Print HTTP return code
  Serial.println(payload);    //Print request response payload

  http.end();  //Close connection
  
  //delay(5000);  //Post Data at every 5 seconds
}
//=======================================================================

void wait_timer(int del){
  unsigned long tm_marker = millis();
  while (millis() - tm_marker < del) yield();
  return;
    
}
