#include <Arduino.h>
#ifndef Oregon_TM_h
#define Oregon_TM_h

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// This Arduino code is for receive and transmit data using Oregon Scientific RF protocol version 2.1 and 3.0. 
//
// Last updated: 14 October 2019
//
// The folowed sensors data format are supported.
//
// Receive and transmit:
// THGN132N (THGR122N, THGN123N),
// RTGN318,
// THGR810.

// Receive only:
// THN132N,
// WGR800,	
// UVN800.
//
// Aslo supported self-developed sensors. Please contact author for additional infromation.
//
// This file is part of the Arduino OREGON_NR library.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// The MIT License (MIT)
//
// Copyright (c) 2019 Sergey Zawislak 
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Данная библиотека Ардуино предназначена для приема и передачи данных в формате беспроводного протокола Oregon Scientific v2.1 и v3.0
//
// Последнее обновление 14 Октября 2019
//
// Поддерживается формат следующих датчиков
//
// Приём и передача:
// THGN132N (THGR122N, THGN123N),
// RTGN318,
// THGR810.

// Тольок приём:
// THN132N,
// WGR800,  
// UVN800.
//
// Также поддерживаются датчики собственной разработки (за дополнительной документацей обращаться к автору)
//
//  Этот файл - часть библиотеки OREGON_NR
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2019 Ñåðãåé Çàâèñëÿê
//
// Äàííàÿ ëèöåíçèÿ ðàçðåøàåò ëèöàì, ïîëó÷èâøèì êîïèþ äàííîãî ïðîãðàììíîãî îáåñïå÷åíèÿ è ñîïóòñòâóþùåé äîêóìåíòàöèè 
// (â äàëüíåéøåì èìåíóåìûìè «Ïðîãðàììíîå Îáåñïå÷åíèå»), áåçâîçìåçäíî èñïîëüçîâàòü Ïðîãðàììíîå Îáåñïå÷åíèå áåç îãðàíè÷åíèé,
// âêëþ÷àÿ íåîãðàíè÷åííîå ïðàâî íà èñïîëüçîâàíèå, êîïèðîâàíèå, èçìåíåíèå, ñëèÿíèå, ïóáëèêàöèþ, ðàñïðîñòðàíåíèå, ñóáëèöåíçèðîâàíèå
// è/èëè ïðîäàæó êîïèé Ïðîãðàììíîãî Îáåñïå÷åíèÿ, à òàêæå ëèöàì, êîòîðûì ïðåäîñòàâëÿåòñÿ äàííîå Ïðîãðàììíîå Îáåñïå÷åíèå, ïðè ñîáëþäåíèè ñëåäóþùèõ óñëîâèé:
//
// Óêàçàííîå âûøå óâåäîìëåíèå îá àâòîðñêîì ïðàâå è äàííûå óñëîâèÿ äîëæíû áûòü âêëþ÷åíû âî âñå êîïèè èëè çíà÷èìûå ÷àñòè äàííîãî Ïðîãðàììíîãî Îáåñïå÷åíèÿ.
//
// ÄÀÍÍÎÅ ÏÐÎÃÐÀÌÌÍÎÅ ÎÁÅÑÏÅ×ÅÍÈÅ ÏÐÅÄÎÑÒÀÂËßÅÒÑß «ÊÀÊ ÅÑÒÜ», ÁÅÇ ÊÀÊÈÕ-ËÈÁÎ ÃÀÐÀÍÒÈÉ, ßÂÍÎ ÂÛÐÀÆÅÍÍÛÕ ÈËÈ ÏÎÄÐÀÇÓÌÅÂÀÅÌÛÕ, ÂÊËÞ×Àß ÃÀÐÀÍÒÈÈ ÒÎÂÀÐÍÎÉ 
// ÏÐÈÃÎÄÍÎÑÒÈ, ÑÎÎÒÂÅÒÑÒÂÈß ÏÎ ÅÃÎ ÊÎÍÊÐÅÒÍÎÌÓ ÍÀÇÍÀ×ÅÍÈÞ È ÎÒÑÓÒÑÒÂÈß ÍÀÐÓØÅÍÈÉ, ÍÎ ÍÅ ÎÃÐÀÍÈ×ÈÂÀßÑÜ ÈÌÈ. ÍÈ Â ÊÀÊÎÌ ÑËÓ×ÀÅ ÀÂÒÎÐÛ ÈËÈ ÏÐÀÂÎÎÁËÀÄÀÒÅËÈ 
// ÍÅ ÍÅÑÓÒ ÎÒÂÅÒÑÒÂÅÍÍÎÑÒÈ ÏÎ ÊÀÊÈÌ-ËÈÁÎ ÈÑÊÀÌ, ÇÀ ÓÙÅÐÁ ÈËÈ ÏÎ ÈÍÛÌ ÒÐÅÁÎÂÀÍÈßÌ, Â ÒÎÌ ×ÈÑËÅ, ÏÐÈ ÄÅÉÑÒÂÈÈ ÊÎÍÒÐÀÊÒÀ, ÄÅËÈÊÒÅ ÈËÈ ÈÍÎÉ ÑÈÒÓÀÖÈÈ, 
// ÂÎÇÍÈÊØÈÌ ÈÇ-ÇÀ ÈÑÏÎËÜÇÎÂÀÍÈß ÏÐÎÃÐÀÌÌÍÎÃÎ ÎÁÅÑÏÅ×ÅÍÈß ÈËÈ ÈÍÛÕ ÄÅÉÑÒÂÈÉ Ñ ÏÐÎÃÐÀÌÌÍÛÌ ÎÁÅÑÏÅ×ÅÍÈÅÌ. 
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#define TR_TIME 488
#define TWOTR_TIME 976
#define PULSE_SHORTEN_2   80
#define PULSE_SHORTEN_3   120

#define THGN132   0x1D20 
#define THGR810   0xF824
#define RTGN318   0xDCC3 
#define THP	    0x5500
#define AIR     0xAAA               // Префикс - идентификатор пакета, принедлежащего всем транспондерам
// #define AIR1    0xAAA1
// #define AIR2    0xAAA2
// #define AIR3    0xAAA3
// #define AIR4    0xAAA4
// #define AIR5    0xAAA5
// #define AIR6    0xAAA6
// #define AIR7    0xAAA7
// #define AIR8    0xAAA8
// #define AIR9    0xAAA9
// #define AIRx
#define AIRx    0xAAA1              // Определяем уникальный номер транспондера AAA0 ... AAA9
// #define flag_start 0x02

static byte TX_PIN = 19;            // установка порта передачи данных (A5)
static byte PTT_PIN = 18;           // установка порта для PTT (A4)
static byte TX_LED  = 15;           // индикация передачи порт (A1)
static byte GPS_LED  = 16;          // индикация приёма GPS (A2)
static const int buffersize = 10;   // original

class Oregon_TM
{
  public:

    byte SendBuffer[buffersize];
    byte protocol = 2;
    word sens_type = 0x0000;               
    int timing_corrector2 = 4;
    int timing_corrector3 = 2;
    bool flag_start = 0;            // флаг синхронизации таймера с нулевой секундой. 31.07.2020
    int send_time_transponder = 5000;//период передачи транспондера  

    Oregon_TM(byte); 
    Oregon_TM(); 
    void setType(word);
    void setChannel( byte);
    void setId(byte);
    void setBatteryFlag(bool);
    void setStartCount(byte);
    void setTemperature(float);
    void setHumidity(byte);
    void setComfort(float, byte);
    bool transmit(int, int);
    void SendPacket();

    void setErrorTHP();
    void setPressureTHP(float);
    void setTemperatureTHP(float);
    void setBatteryTHP(word);
    void setChannelTHP(byte);
    void setHumidityTHP(float);
    // mr.Tech
    void setLatitude(long);
    void setLongitude(long);
    void setAltitude(int);
    void GPSsquelshOn();
    void GPSsquelshOff();
    // end


  private:
    
    void sendZero(void);
    void sendOne(void);
    void sendMSB(const byte);
    void sendLSB(const byte);
    void sendData();
    void sendOregon();
    void sendPreamble();
    void calculateAndSetChecksum132();
    void calculateAndSetChecksum318();
    void calculateAndSetChecksum810();
    void calculateAndSetChecksumAAA();


    void calculateAndSetChecksumTHP();

    unsigned long time_marker = 0;
    unsigned long time_marker_send = 0;
    unsigned long send_time = 0;
    bool prevbit = 1;
    bool prevstate = 1;


};

#endif

