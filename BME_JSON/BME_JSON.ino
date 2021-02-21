#include <ESP8266WiFi.h>                                // Подключаем библиотеку ESP8266WiFi
//#include <Wire.h>                                       // Подключаем библиотеку Wire
#include <Adafruit_BME280.h>                            // Подключаем библиотеку Adafruit_BME280
#include <Adafruit_Sensor.h>                            // Подключаем библиотеку Adafruit_Sensor
 
#define SEALEVELPRESSURE_HPA (1013.25)                  // Задаем высоту
 
Adafruit_BME280 bme;                                    // Установка связи по интерфейсу I2C
 
const char* ssid = "ASUS";          // Название Вашей WiFi сети
const char* password = "123456789";     // Пароль от Вашей WiFi сети
 
WiFiServer server(80);                                  // Указываем порт Web-сервера
String header;
 
void setup() {
  Serial.begin(115200);                                 // Скорость передачи 115200
  bool status;
                                                       
  if (!bme.begin(0x77)) {                               // Проверка инициализации датчика
    Serial.println("Could not find a valid BME280 sensor, check wiring!"); // Печать, об ошибки инициализации.
    while (1);                                          // Зацикливаем
  }
 
  Serial.print("Connecting to ");                       // Отправка в Serial port 
  Serial.println(ssid);                                 // Отправка в Serial port 
  WiFi.begin(ssid, password);                           // Подключение к WiFi Сети
  while (WiFi.status() != WL_CONNECTED) {               // Проверка подключения к WiFi сети
    delay(500);                                         // Пауза
    Serial.print(".");                                  // Отправка в Serial port 
  }
 
  Serial.println("");                                   // Отправка в Serial port 
  Serial.println("WiFi connected.");                    // Отправка в Serial port 
  Serial.println("IP address: ");                       // Отправка в Serial port 
  Serial.println(WiFi.localIP());                       // Отправка в Serial port 
  server.begin();                                  
}
 
void loop(){
  WiFiClient client = server.available();               // Получаем данные, посылаемые клиентом 
 
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
            client.println("Refresh: 10");              // Автоматическое обновление каждые 10 сек 
            client.println();
            // Формирую данные в формате JSON
            client.print("{\"Temp\" :\"");          
            client.print(bme.readTemperature());
            client.print("\",\"Pres\" :\"");
            client.print(bme.readPressure());
            client.print("\",\"Hum\" :\"");
            client.print(bme.readHumidity());
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
