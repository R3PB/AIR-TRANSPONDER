/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Скетч для передачи показаний с беспроводных датчиков Oregon Scientific THN132N или THGN132N на сервис “Народный Мониторинг” (narodmon.ru)
//с помощью Arduino-совместимых плат на основе ESP8266 (Wemos D1, NodeMCU).

//Для подключения необходимы:
//- Сам датчик Oregon Scientific THN132N, THGN132N, THGN123 и т.п.,
//- ПРоцессорная плата на основе ESP8266 (Wemos D1 или NodeMCU),
//- Приёмник OOK 433Мгц (Питание 3В, подключается к D7 процессорной платы),
//- WiFi подключение к Интернет
//- Arduino IDE с установленной поддержкой ESP8266-совместимых устройств и библиотекой Oregon_NR
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Oregon_NR_MY.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
//#include <ESP8266WebServer.h>
//#include <ESP8266mDNS.h>
//#include <Wire.h>                 // Подключаем библиотеку Wire
//#include <Adafruit_BME280.h>        // Подключаем библиотеку Adafruit_BME280
//#include <Adafruit_Sensor.h>        // Подключаем библиотеку Adafruit_Sensor

/*#include <SPI.h>
#define BME_SCK 14
#define BME_MISO 12
#define BME_MOSI 13
#define BME_CS 15*/

//Режимы работы
#define TEST_MODE       false       //Режим отладки (данные на narodmon.ru не отсылаются)
//#undef TEST_MODE

#define SEND_INTERVAL 120000        //350000 - 5 min   //Как часто отсылать данные на сервер
//#define SEND_INTERVAL 12000
#define CONNECT_TIMEOUT 10000       //Время ожидания  соединения
#define DISCONNECT_TIMEOUT 10000    //Время ожидания отсоединения

//#define mac "#FF:FF:FF:FF:FF:FF"  //МАС-адрес на narodmon.ru d8-f1-5b-07-1c-0b
//#define mac "#D8:F1:5B:07:1C:0B"  //МАС-адрес на narodmon.ru d8-f1-5b-07-1c-0b
#define mac "#ESP826677671C0B"      //МАС-адрес на narodmon.ru d8-f1-5b-07-1c-0b
//#define mac "#ESP8266FFFFFFFF"    //ID или MAC на narodmon.ru

const char* ssid = "ASUS";          //Параметры входа в WiFi
const char* password = "123456789";
//настройки сети для хитрого маршрута
//IPAddress ip(192,168,137,17);     //статический IP 
//IPAddress gateway(192,168,137,1);
//IPAddress subnet(255,255,255,0);
//IPAddress dns1(8,8,8,8);
//ESP8266WebServer server(80);
//****************************************************************************************

//Oregon_NR oregon(13, 13, 2, true);// Приёмник 433Мгц подключён к D7 (GPIO13), Светодиод на D2 подтянут к +пит.
Oregon_NR oregon(15, 15, 3, true);  // Приёмник 433Мгц подключён к D7 (GPIO13), Светодиод на D2 подтянут к +пит.

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Массивы для хранения полученных данных для датчиков Oregon:
    float r_tmp[3];                 //Температура
    float  r_hmdty[3];              //Влажность. Если = 101 - значит нет данных
    bool  r_bat[3], bat_pcr, bat_wrt; //Флаг батареи
    bool  r_isreceived[3];          //Флаг о том, что по данному каналу приходил хоть один правильный пакет и данные актуальны    
    word  r_type[3];                //Тип датчика
    unsigned long rcv_time[3];      // времена прихода последних пакетов
    float  number_of_receiving[3];  //сколько пакетов получено в процессе сбора данных
    float wa_speed;                 // Скорость ветра
    float mw_speed;                 // Макс скорость ветра
    float w_d;                      // Направление ветра
    float r_rate;                   // Интенсивность осадков - дождя
    float r_total;                  // Колличество осадков - дождя
    float tempC;                    // Температура
    float humidity;                 // Влажность
    float pressureHgmm;             // Давление мм Рт.ст
    

//****************************************************************************************

    #define BLUE_LED 2                  //Индикация подключения к WiFi
    #define GREEN_LED 14                //Индикатор успешной доставки пакета а народмон
    #define RED_LED 255                 //Индикатор ошибки доставки пакета на народмон
//    #define BME280_ADDRESS 0x77
//    Adafruit_BME280 bme;

    //Параметоы соединения с narodmon:
    //IPAddress nardomon_server(94,19,113,221);
    char narodomon_server[] = "narodmon.ru"; 
    int port=8283;
    WiFiClient client; //Клиент narodmon
    WiFiServer server(80);  // Указываем порт Web-сервера
    String header;
    const unsigned long postingInterval = SEND_INTERVAL; 
    unsigned long lastConnectionTime = 0;                   
    boolean lastConnected = false;                          
    unsigned long cur_mark;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//SETUP//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void setup()
{
  
  pinMode(BLUE_LED, OUTPUT);        
  pinMode(GREEN_LED, OUTPUT);        
  pinMode(RED_LED, OUTPUT);
//  pinMode(1, INPUT);
//  pinMode(2, INPUT); 
//  pinMode(3, INPUT); 
//  pinMode(4, INPUT);
//  pinMode(5, INPUT); 
digitalWrite(2, LOW);  
digitalWrite(1, LOW);
digitalWrite(15, LOW);       
  
  /////////////////////////////////////////////////////
  //Инициализация памяти
  for (int i = 0; i < 3; i++)
  {
    rcv_time[i] = 7000000;
    r_isreceived[i] = 0;
    number_of_receiving[i] = 0;
  }
  
  /////////////////////////////////////////////////////
  //Запуск Serial-ов
  Serial.begin(115200);
  Serial.println("");
  if (TEST_MODE) Serial.println("Test mode");

  /////////////////////////////////////////////////////
  //Запуск Wifi
  wifi_connect();
  /////////////////////////////////////////////////////
  
  digitalWrite(BLUE_LED, HIGH);    
  if (test_narodmon_connection()){
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(RED_LED, LOW);    
  }
  else {
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(RED_LED, HIGH);
  }

  //вкючение прослушивания радиоканала  
  oregon.start(); 
  oregon.receiver_dump = false;
  //Если не распознаются последние байты принятого пакета можно попробовать подстроить частоту захвата данных
  oregon.timing_correction = 0;  // коррекция частоты завхвата данных. Обычно достаоточно подобрать значение от -5 до 5
  oregon.decode_method = 1;      //или использовать метод 3 для декодирования

  //подключение датчика давления, температуры и влажности BME280
//    Wire.begin(D1, D2);
//  Wire.setClock(100000);
//  if (!bme.begin(BME280_ADDRESS )) {                                                  // Проверка инициализации датчика
//    Serial.println("Could not find a valid BME280 sensor, check wiring!");            // Печать, об ошибке инициализации.
//    while (1) delay(3000);                                                              // Зацикливаем
//  }
//  Wire.begin();
  //  Wire.setClock(100000); 
//  if (!bme.begin(BME280_ADDRESS ))
//    {
//     Serial.println("Could not find a valid BME280 sensor, check wiring!");
//     while (1)
//      {
//      yield();
//      delay(3000);
//      }
//    }
  //
  server.begin();
}
//////////////////////////////////////////////////////////////////////
//LOOP//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void loop() 
{
//    if (!bme.begin(BME280_ADDRESS ))
//    {
//     Serial.println("Could not find a valid BME280 sensor, check wiring!");
////     while (1)
//      {
//      yield();
//   //   delay(3000);
//      }
//    }
  WiFiClient client = server.available();               // Получаем данные, посылаемые клиентом 
 
  //////////////////////////////////////////////////////////////////////
  //Защита от подвисаний/////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////  
  if  (micros() > 0xFFF00000) while ( micros() < 0xFFF00000); //Висим секунду до переполнения
  if  (millis() > 0xFFFFFC0F) while ( millis() < 0xFFFFFC0F); //Висим секунду до переполнения

  //////////////////////////////////////////////////////////////////////
  //Проверка полученных данных,/////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////  
  bool is_a_data_to_send = false;
  for (int i = 0; i < 3; i++){
    if (r_isreceived[i]) is_a_data_to_send = 1;                 // Есть ли данные для отправки?
  }
  
  is_a_data_to_send = 1; //Отладка!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  
  //////////////////////////////////////////////////////////////////////
  //Отправка данных на narodmon.ru/////////////////////////////////////
  //////////////////////////////////////////////////////////////////////  
  
  if (millis() - lastConnectionTime > postingInterval && is_a_data_to_send)  {
    //Обязательно отключить прослушивание канала
    oregon.stop();
    digitalWrite(BLUE_LED, HIGH);    
    if (send_data()){
      digitalWrite(GREEN_LED, HIGH);
      digitalWrite(RED_LED, LOW);
    }
    else {
      digitalWrite(GREEN_LED, LOW);
      digitalWrite(RED_LED, HIGH);
    }
    oregon.start();    
  }

  //////////////////////////////////////////////////////////////////////
  //Захват пакета,//////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////  
  oregon.capture(0); // (1) - отладка отображается в консоль
  //
  //Захваченные данные годны до следующего вызова capture

  //////////////////////////////////////////////////////////////////////
  //ОБработка полученного пакета//////////////////////////////////////////////
  if (oregon.captured) 
  {
    //Вывод информации в Serial
    if ((oregon.sens_type == THGN132 || oregon.sens_type == THN132) && oregon.crc_c){
      Serial.print("TIME:\t");
      Serial.print(millis()/1000);
      Serial.print("sec\t");
      //if (oregon.sens_type == THGN132)  for (int q = 0;q < PACKET_LENGTH - 2; q++) Serial.print(oregon.packet[q], HEX);
      if (oregon.sens_type == THGN132)  for (int q = 0;q < PACKET_LENGTH - 1; q++) Serial.print(oregon.packet[q], HEX);
      //if (oregon.sens_type == THN132)  for (int q = 0;q < PACKET_LENGTH - 4; q++) Serial.print(oregon.packet[q], HEX);
      if (oregon.sens_type == THN132)  for (int q = 0;q < PACKET_LENGTH - 4; q++) Serial.print(oregon.packet[q], HEX);
      Serial.print(" TYPE: ");
      if (oregon.sens_type == THGN132) Serial.print("THGN132N");
      if (oregon.sens_type == THN132) Serial.print("THN132N ");
      Serial.print(" CHNL: ");
      Serial.print(oregon.sens_chnl);
      if (oregon.sens_tmp > 0 && oregon.sens_tmp < 10) Serial.print(" TMP:  ");
      if (oregon.sens_tmp < 0 && oregon.sens_tmp >-10) Serial.print(" TMP: ");
      if (oregon.sens_tmp <= -10) Serial.print(" TMP:");
      if (oregon.sens_tmp >= 10) Serial.print(" TMP: ");
      Serial.print(oregon.sens_tmp, 1);
      Serial.print("C ");
      if (oregon.sens_type == THGN132) {
        Serial.print("HUM: ");
        Serial.print(oregon.sens_hmdty, 1);
        Serial.print("%");
      }
            
      Serial.print(" BAT: ");
      if (oregon.sens_battery) Serial.print("F "); else Serial.print("e ");
      Serial.print("ID: ");
      Serial.print(oregon.sens_id, HEX);
      //Serial.print(" PACKETS: ");
      //Serial.print(oregon.packets_received);
      //Serial.print(" PROC. TIME: ");
      //Serial.print(oregon.work_time);
      //Serial.println("ms ");
      Serial.println("");
               
      byte _chnl = oregon.sens_chnl - 1;
      number_of_receiving[ _chnl]++;
      r_type[_chnl] = oregon.sens_type;
      r_bat[_chnl] = oregon.sens_battery;
      r_isreceived[_chnl] = 1;
      r_tmp[_chnl] = r_tmp[_chnl] * ((number_of_receiving[_chnl] - 1) / number_of_receiving[_chnl]) + oregon.sens_tmp / number_of_receiving[_chnl];
      r_hmdty[_chnl] = r_hmdty[_chnl] * ((number_of_receiving[_chnl] - 1) / number_of_receiving[_chnl]) + oregon.sens_hmdty / number_of_receiving[_chnl];
      rcv_time[_chnl] = millis();         
    }
    
    if (!oregon.crc_c && oregon.packets_received)
    {
      Serial.println("TIME:\t");
      Serial.print(millis()/1000);
      Serial.print("sec\t");
      for (int q = 0; q < PACKET_LENGTH - 1 ; q++) {
        if (oregon.valid_p[q] >= 0x0F) Serial.print(oregon.packet[q], HEX);
        else Serial.print("x");
      }
      Serial.println(" CORRUPTED PACKET ");
    }
  }

if (oregon.sens_type == WGR800 && oregon.crc_c){
      Serial.println("\t");
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

      //####
      //      float wa_speed[3];             // Скорость ветра
      //      float mw_speed[3];             // Макс скорость ветра
      //      bool  w_direction[3];          // Направление ветра
      //      float r_rate[3];               // Интенсивность осадков - дождя
      //      float r_total[3];              // Колличество осадков - дождя
      //####
      byte _chnl = oregon.sens_chnl - 1;
      number_of_receiving[ _chnl]++;
      r_type[_chnl] = oregon.sens_type;
      r_bat[_chnl] = oregon.sens_battery;
      r_isreceived[_chnl] = 1;
      //r_tmp[_chnl] = r_tmp[_chnl] * ((number_of_receiving[_chnl] - 1) / number_of_receiving[_chnl]) + oregon.sens_tmp / number_of_receiving[_chnl];
      bat_wrt = oregon.sens_battery;
      wa_speed = oregon.sens_avg_ws;
      mw_speed = oregon.sens_max_ws;
      // translate rumb to direction in DEG
      switch (oregon.sens_wdir) {
        case 0: {w_d = 360; break;}   // 0-N
        case 1: {w_d = 22.5; break;}  // 1-NNE
        case 2: {w_d = 45; break;}    // 2-NE
        case 3: {w_d = 67.5; break;}  // 3-ENE
        case 4: {w_d = 90; break;}    // 4-E
        case 5: {w_d = 112.5; break;} // 5-ESE
        case 6: {w_d = 135; break;}   // 6-SE
        case 7: {w_d = 157.5; break;} // 7-SSE
        case 8: {w_d = 180; break;}   // 8-S
        case 9: {w_d = 202.5; break;} // 9-SSW
        case 10: {w_d = 225; break;}  // A-SW
        case 11: {w_d = 247.5; break;}// B-WSW
        case 12: {w_d = 270; break;}  // C-W
        case 13: {w_d = 292.5; break;}// D-WNW
        case 14: {w_d = 315; break;}  // E-NW
        case 15: {w_d = 337.5; break;}// F-NNW
      }
      rcv_time[_chnl] = millis();
    }    
    if (oregon.sens_type == PCR800 && oregon.crc_c){
      Serial.println("\t");
      float wnddata;
      Serial.print(" TYPE: ");
      Serial.print("PCR800,");
      Serial.print(" RAIN RATE: ");
      Serial.print(oregon.rain_rate, 4);
      //Serial.print(oregon.rain_rate, 3);
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
      //Serial.print(oregon.sens_battery, HEX);
      //Serial.print(oregon.oregon_data);
      //Serial.print("checksumm = ");
      //Serial.print(oregon.crc_c, HEX);
      //Serial.print(oregon.sens_id);
      bat_pcr = oregon.sens_battery;
      r_rate = oregon.rain_rate;
      r_total = oregon.total_rain;
    }  
//    tempC = bme.readTemperature();
//    humidity = bme.readHumidity();
//    pressureHgmm = bme.readPressure() / 133.322;

    // Print to serial monitor
   // printToSerial(tempC, humidity, pressureHgmm);
      ////////////////////////////////////////////
      /// JSON client ////////////////////////////
      if (client) {                                         
      Serial.println("New Client.");                      // Отправка "Новый клиент"
      String currentLine = "";                            // Создаем строку для хранения входящих данных от клиента
      while (client.connected()) {                        // Пока есть соединение с клиентом 
        if (client.available()) {                         // Если клиент активен 
          char c = client.read();                         // Считываем посылаемую информацию в переменную "с"
          Serial.write(c);                                // Отправка в Serial port 
          header += c;
          if (c == '\n') {                                // Вывод HTML страницы 
            if (currentLine.length() == 0) {
              client.println("HTTP/1.1 200 OK");          // Стандартный заголовок HTTP
              client.println("Access-Control-Allow-Origin: *"); // разрешаю cross-origin запросы
              client.println("Content-type:text/html ");
              client.println("Connection: close");        // Соединение будет закрыто после завершения ответа
              client.println("Refresh: 13");              // Автоматическое обновление каждые 13 сек 
              client.println();
              // Формирую данные в формате JSON
              client.print("{\"w_d\" :\"");          
              client.print(w_d);
              client.print("\",\"wa_speed\" :\"");
              client.print(wa_speed);
              client.print("\",\"mw_speed\" :\"");
              client.print(mw_speed);
              client.println("\"}");
              client.println();
              break;
            } else { 
              currentLine = "";
            }
          } else if (c != '\r') {  
            currentLine += c;      
          }
        }
      }
      header = "";
      client.stop();
      Serial.println("Client disconnected.");
      Serial.println("");
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//***************************************************************************************************************************************
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void wifi_connect() {
  
  Serial.println();
  Serial.print("Connecting to ");
  Serial.print(ssid);
  unsigned long cur_mark = millis();
  bool blink = 0;
  //WiFi.config(ip, gateway, subnet, dns1);     ////////// comment if dhcp 
  WiFi.begin(ssid, password);
  do {
      while (WiFi.status() != WL_CONNECTED) {
      if (blink) {
        digitalWrite(BLUE_LED, LOW);
        
      }
      else {
        digitalWrite(BLUE_LED, HIGH);

      }
      blink = !blink;
      delay(500);
      Serial.print(".");
      //Подключаемся слишком долго. Переподключаемся....
      if ((millis() - cur_mark) > CONNECT_TIMEOUT){
        blink = 0; 
        digitalWrite(BLUE_LED, HIGH);
        WiFi.disconnect();
        delay(3000);
        cur_mark = millis();
        WiFi.begin(ssid, password);
      }
    }
  } while (WiFi.status() != WL_CONNECTED);

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println(WiFi.localIP());

}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool test_narodmon_connection() {
  if (client.connect(narodomon_server, port)) {  
    client.println("##");
    cur_mark = millis();
    do {
      wait_timer(10);
      if ((millis() - cur_mark) > CONNECT_TIMEOUT) {
        Serial.println("narodmon.ru is not responding");
        client.stop();
        return 0;
      }
    } while (!client.connected());
    Serial.println("narodmon.ru is attainable");
    client.stop();
    return 1;
  } 
  else {
    Serial.println("connection to narodmon.ru failed");
    client.stop();
    return 0;
  }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool send_data() {

  //Если соединения с сервером нет, по переподключаемся
  if (WiFi.status() != WL_CONNECTED) wifi_connect();
  
  bool what_return = false;
  if (client.connect(narodomon_server, port)) {  
    //Отправляем MAC-адрес
    Serial.println(' ');
    String s = mac;
    Serial.print(s);
    if (!TEST_MODE) client.print(s);
//    if (TEST_MODE) Serial.println(s);
//    if (TEST_MODE) Serial.println("YOU IN TEST MODE !!!!!!!!!!!");
//    if (TEST_MODE) Serial.println("YOU IN TEST MODE !!!!!!!!!!!");
//    if (!TEST_MODE) Serial.println("YOU ARE NOT IN THE TEST MODE !!!!!!!!!!!");
    //Отправляем данные Oregon
    sendOregonData();
        
    //Завершаем передачу
    client.print("##");
    Serial.println("##");
    //Ждём отключения клиента
    cur_mark = millis();
    do {
      yield();
      if (millis() > cur_mark + DISCONNECT_TIMEOUT) break;
    } while (!client.connected());
      
    Serial.println(' ');
    client.stop();
    what_return = true;
  } 
  else {
    Serial.println("connection to narodmon.ru failed");
    client.stop();
  }
  lastConnectionTime = millis();

  //Обнуляем флаги полученных данных
  for (int i = 0; i < 3; i++) {
    r_isreceived[i] = 0;
    number_of_receiving[i] = 0;
  }
  
  return what_return;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void sendOregonData() {
//   float rain_deviation, rain_deviation_2;
//   rain_deviation = random(1, 3) * 0.001;
//   rain_deviation_2 = random(1, 3) * 0.0001;
  
    String s;
    //  WRT800 wind average speed ####
    s = "\n#WS1#";
    s += (float)wa_speed;
    Serial.println(s);
    if (!TEST_MODE) client.print(s);
    
    //  WRT800 max wind speed ########
    s = "\n#WS2#";
    s += mw_speed;
    Serial.print(s);
    if (!TEST_MODE) client.print(s);

    //  WRT800 wind direction #########
    w_d += random(-1, 1) * 0.01; //added deviation 
    s = "\n#DIR#";
    s += w_d;
    //was encoded  0-N, 1-NNE, 2-NE, 3-ENE, 4-E, 5-ESE, 6-SE, 7-SSE, 8-S, 9-SSW, A-SW, B-WSW, C-W, D-WNW, E-NW,F-NNW
    Serial.println(s);
    if (!TEST_MODE) client.print(s);
    // v battery
    s = "\n#VBAT1#";
    switch (bat_wrt) {
        case 1: {s += " 99"; break;} 
        default: s += " 50";
    }
//    s += bat_;
    Serial.println(s);
    if (!TEST_MODE) client.print(s);
    //#####################################
    
    //  PCR800 rain deviation - especially for narodmon.ru #######################################
    //  float rain_deviation; // rain_deviation
    //  r_rate = oregon.rain_rate;
    //  r_total = oregon.total_rain;
    //  rain_deviation = random(1, 3) * 0.0025;
    //  total_rain_deviation = random(1, 3) * 0.0001;
    
    //  PCR800  rain rate ########################################################################
    s = "\n#RAIN1#";
    //    r_rate += rain_deviation; // addin deviation of last digit - .00$ - important for narodmon.ru 
    r_rate += random(0, 3) * 0.0026;
    s += String(r_rate, 4);   // show .000 digits - important for narodmon.ru
    Serial.println(s);
    if (!TEST_MODE) client.print(s);
    //  PCR800  total rain ######################################################################
    s = "\n#RAIN2#";
    //    r_total += rain_deviation;
    r_total += random(0, 3) * 0.0026;
    s += String(r_total, 4); // show .0000 digits - for narodmon.ru
    Serial.println(s);
    if (!TEST_MODE) client.print(s);
    // v battery
    s = "\n#VBAT2#";
    //s += bat_;
    switch (bat_pcr) {
        case 1: {s += " 99"; break;} 
        default: s += " 50";
    }
    Serial.println(s);
    if (!TEST_MODE) client.println(s);

//    // TEMPERATURE ##
//    s = "\n#T1#";
//    s += tempC;
//    Serial.println(s);
//    if (!TEST_MODE) client.print(s);
//
//    // HUMIDITY ##
//    s = "\n#H1#";
//    s += humidity;
//    Serial.println(s);
//    if (!TEST_MODE) client.print(s);
//
//    // PRESURE ##
//    s = "\n#P1#";
//    s += pressureHgmm;
//    Serial.println(s);
//    if (!TEST_MODE) client.print(s);
    //    ########################################################################################

    
//  if (r_isreceived[0]) {
//    s = "\n#T1#";
//    s += r_tmp[0];
//    if (r_hmdty[0] > 0 && r_hmdty[0] <= 100 ){
//      s += "\n#H1#";
//      s += r_hmdty[0];
//    }
//    Serial.println(s);
//    if (!TEST_MODE) client.println(s);
//
//  }
//  if (r_isreceived[1]) {
//    s = "\n#T2#";
//    s += r_tmp[1];
//    if (r_hmdty[1] > 0 && r_hmdty[1] <= 100 ){
//      s += "\n#H2#";
//      s += r_hmdty[1];
//    }
//     s += "\n#W1#";
//    s += "0.1 m/s";
//    Serial.println(s);
//    if (!TEST_MODE) client.println(s);
//  }
//  if (r_isreceived[2]) {
//    s = "\n#T3#";
//    s += r_tmp[2];
//    if (r_hmdty[2] > 0 && r_hmdty[2] <= 100 ){
//      s += "\n#H3#";
//      s += r_hmdty[2];
//    }
//     s += "\n#W1#";
//    s += "0.3 m/s";
//    Serial.println(s);
//    if (!TEST_MODE) client.println(s);
//  }
}

/////////////////////////////////////////////////////////////////////////////
void printToSerial(float tempC, float humidity, float pressurePascals)
{
//// Temperature
Serial.print("Temperature: ");
Serial.print(tempC);
Serial.println("C");
//// Barometric pressure
Serial.print("Pressure: ");
 Serial.print(pressureHgmm);
 Serial.println("Hgmm");
//// Humidity
 Serial.print("Humidity: ");
 Serial.print(humidity);
 Serial.println("%");
delay(3000);
}
//////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////
// ЗАМЕНА DELAY, которая работает и не приводит к вылету...
////////////////////////////////////////////////////////////////////////////////////////
void wait_timer(int del){
  unsigned long tm_marker = millis();
  while (millis() - tm_marker < del) yield();
  return;
    
}
