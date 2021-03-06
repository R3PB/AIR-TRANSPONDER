#include "Oregon_TX_AIR.h"

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
// Äàííàÿ áèáëèîòåêà Àðäóèíî ïðåäíàçíà÷åíà äëÿ ïðèåìà è ïåðåäà÷è äàííûõ â ôîðìàòå áåñïðîâîäíîãî ïðîòîêîëà Oregon Scientific v2.1 è v3.0
//
// Ïîñëåäíåå îáíîâëåíèå 14 Îêòÿáðÿ 2019
//
// Ïîääåðæèâàåòñÿ ôîðìàò ñëåäóþùèõ äàò÷èêîâ
//
// Ïðè¸ì è ïåðåäà÷à:
// THGN132N (THGR122N, THGN123N),
// RTGN318,
// THGR810.

// Òîëüîê ïðè¸ì:
// THN132N,
// WGR800,	
// UVN800.
//
// Òàêæå ïîääåðæèâàþòñÿ äàò÷èêè ñîáñòâåííîé ðàçðàáîòêè (çà äîïîëíèòåëüíîé äîêóìåíòàöåé îáðàùàòüñÿ ê àâòîðó)
//
//  Ýòîò ôàéë - ÷àñòü áèáëèîòåêè OREGON_NR
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




//Конструктор
Oregon_TM::Oregon_TM(byte tr_pin)
{
  TX_PIN = tr_pin;
  pinMode(TX_PIN, OUTPUT); 
  digitalWrite(TX_PIN, LOW);
  // PTT_PIN = ptt_pin_;
}
Oregon_TM::Oregon_TM(void)
{
  pinMode(TX_PIN, OUTPUT);      // DATA pin
  digitalWrite(TX_PIN, LOW);    //   
  pinMode(PTT_PIN, OUTPUT);     // PTT pin
  digitalWrite(PTT_PIN, HIGH);  // PTT ON ??? нужно доработать, так как после отключения питания от ардумны, ptt становится всегда ВКЛЮЧЕН! - переделать!
  pinMode(TX_LED, OUTPUT);
  digitalWrite(TX_LED, HIGH);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Функции передатчика////////////////////////////////////////////////////////////////////////////
///// ФОРМИРУЕМ "0" //////////////////////////////////////////////////////////////////////////////////////////////
void Oregon_TM::sendZero(void)
{
  if (protocol == 2){
    while (time_marker + TR_TIME * 4 >= micros());
    time_marker += TR_TIME * 4;
    digitalWrite(TX_PIN, HIGH);
    delayMicroseconds(TR_TIME - PULSE_SHORTEN_2);
    digitalWrite(TX_PIN, LOW);
    delayMicroseconds(TWOTR_TIME + PULSE_SHORTEN_2);
    digitalWrite(TX_PIN, HIGH);
  }
  if (protocol == 3)
    {
      if (prevstate) while (time_marker + TWOTR_TIME - PULSE_SHORTEN_3 >= micros());
      else while (time_marker + TWOTR_TIME >= micros());
      
    time_marker += TWOTR_TIME;
    
    if (prevbit && prevstate)
    {
      digitalWrite(TX_PIN, LOW);
      prevstate = 0;
      prevbit = 0;
      return;            
    }
    if (prevbit && !prevstate)
    {
      digitalWrite(TX_PIN, HIGH);
      delayMicroseconds(TWOTR_TIME);
      prevstate = 1;
      prevbit = 0;
      return;                  
    }
    if (!prevbit && prevstate)
    {
      digitalWrite(TX_PIN, LOW);
      delayMicroseconds(TR_TIME);
      digitalWrite(TX_PIN, HIGH);
      prevbit = 0;
      return;                  
    }
    if (!prevbit && !prevstate)
    {
      digitalWrite(TX_PIN, HIGH);
      delayMicroseconds(TR_TIME);
      digitalWrite(TX_PIN, LOW);
      prevbit = 0;
      return;                  
    }
  }
}
///// ФОРМИРУЕМ "1" //////////////////////////////////////////////////////////////////////////////////////////////

void Oregon_TM::sendOne(void)
{
  if (protocol == 2){
    while (time_marker + TR_TIME * 4 - PULSE_SHORTEN_2>= micros());
    time_marker += TR_TIME * 4;
    digitalWrite(TX_PIN, LOW);
    delayMicroseconds(TR_TIME + PULSE_SHORTEN_2);
    digitalWrite(TX_PIN, HIGH);
    delayMicroseconds(TWOTR_TIME - PULSE_SHORTEN_2);
    digitalWrite(TX_PIN, LOW);
  }
  
  if (protocol == 3)
  {
    if (prevstate) while (time_marker + TWOTR_TIME - PULSE_SHORTEN_3 >= micros());
    else while (time_marker + TWOTR_TIME >= micros());
    time_marker += TWOTR_TIME;
    
    if (!prevbit && prevstate)
    {
      digitalWrite(TX_PIN, LOW);
      prevstate = 0;
      prevbit = 1;
      return;
    }
    if (!prevbit && !prevstate)
    {
      digitalWrite(TX_PIN, HIGH);
      prevstate = 1;
      prevbit = 1;
      return;      
    }
    if (prevbit && prevstate)
    {
      digitalWrite(TX_PIN, LOW);
      delayMicroseconds(TR_TIME);
      digitalWrite(TX_PIN, HIGH);
      prevbit = 1;
      return;      
    }
    if (prevbit && !prevstate)
    {
      digitalWrite(TX_PIN, HIGH);
      delayMicroseconds(TR_TIME);
      digitalWrite(TX_PIN, LOW);
      prevbit = 1;
      return;      
    }
    
  }
}
///// ФОРМИРУЕМ MSB//////////////////////////////////////////////////////////////////////////////////////////////

void Oregon_TM::sendMSB(byte data)
{
  (bitRead(data, 4)) ? sendOne() : sendZero();
  (bitRead(data, 5)) ? sendOne() : sendZero();
  (bitRead(data, 6)) ? sendOne() : sendZero();
  (bitRead(data, 7)) ? sendOne() : sendZero();
  if (protocol == 2) time_marker += timing_corrector2;       //Поправка на разницу тактовых частот 1024.07Гц и 1024.60Гц
  if (protocol == 3) time_marker += timing_corrector3;
                     
  
}
///// ФОРМИРУЕМ LSB//////////////////////////////////////////////////////////////////////////////////////////////
 
void Oregon_TM::sendLSB(byte data)
{
  (bitRead(data, 0)) ? sendOne() : sendZero();
  (bitRead(data, 1)) ? sendOne() : sendZero();
  (bitRead(data, 2)) ? sendOne() : sendZero();
  (bitRead(data, 3)) ? sendOne() : sendZero();
  if (protocol == 2) time_marker += timing_corrector2;       //Поправка на разницу тактовых частот 1024.07Гц и 1024.60Гц
  if (protocol == 3) time_marker += timing_corrector3;
}
////// формируем BYTE (MSB LSB)/////////////////////////////////////////////////////////////////////////////////////////////

void Oregon_TM::sendData()
{
  for(byte i = 0; i < 11; ++i)     // mr.Tech Увеличил на байт(один цикл), так как не вмещались параметры altitude и vario для  транспондеров AIR1,...
  // for(byte i = 0; i < 10; ++i)  //original
  {
    sendMSB(SendBuffer[i]);
     if (i < 10)                    // mr.Tech соответственно здесь так же увеличил на один цикл
    // if (i < 9)                   // original
    sendLSB(SendBuffer[i]);
  if (protocol == 2) time_marker += 4;       //Поправка на разницу тактовых частот 1024.07Гц и 1024.60Гц
  //if (protocol == 3) time_marker += 4;
    //Поправка на разницу тактовых частот 1024.07Гц и 1024Гц
  }
}
///////////////////////////////////////////////////////////////////////////////////////////////////
 
void Oregon_TM::sendOregon()
{
    time_marker=micros();
    sendPreamble();
    sendLSB(0xA);
    sendData();
}
///////////////////////////////////////////////////////////////////////////////////////////////////

void Oregon_TM::sendPreamble(void)
{
  if (protocol == 2){
    sendLSB(0xF);
    sendLSB(0xF);
    time_marker += 9; 
    sendLSB(0xF);
    sendLSB(0xF);
    time_marker += 9;
  }
  if (protocol == 3){
    sendLSB(0xF);
    sendLSB(0xF);
    sendLSB(0xF);
    sendLSB(0xF);
    time_marker += 4;
    sendLSB(0xF);
    sendLSB(0xF);
    time_marker += 3;
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
 
void Oregon_TM::calculateAndSetChecksum132(void)
{
  byte CCIT_POLY = 0x07;
  SendBuffer[7] &= 0xF0;
  SendBuffer[8] = 0x00;
  SendBuffer[9] = 0x00;
  byte summ = 0x00;
  byte crc = 0x3C;
  byte cur_nible;
  for(int i = 0; i<8; i++) 
  {
    cur_nible = (SendBuffer[i] & 0xF0) >> 4;
    summ += cur_nible;
    if (i !=3)
    {
      crc ^= cur_nible;
      for(int j = 0; j < 4; j++)
      if (crc & 0x80) crc = (crc << 1) ^ CCIT_POLY;
      else crc <<= 1;
    }  
    cur_nible = SendBuffer[i] & 0x0F;
    summ += cur_nible;
    if (i !=2)
    {
      crc ^= cur_nible;
      for(int j = 0; j < 4; j++)
      if (crc & 0x80) crc = (crc << 1) ^ CCIT_POLY;
      else crc <<= 1;
    }  
  }
  SendBuffer[7] += summ & 0x0F;
  SendBuffer[8] += summ & 0xF0;
  SendBuffer[8] += crc & 0x0F;
  SendBuffer[9] += crc & 0xF0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////

void Oregon_TM::calculateAndSetChecksum318()
{
  byte CCIT_POLY = 0x07;
  SendBuffer[7] = SendBuffer[7] & 0xF0;
  SendBuffer[8] = 0x00;
  SendBuffer[9] = 0x00;
  byte summ = 0x00;
  byte crc = 0x00;
  byte cur_nible;
  for(int i = 0; i<8; i++) 
  {
    cur_nible = (SendBuffer[i] & 0xF0) >> 4;
    summ += cur_nible;
    if (i !=3)
    {
      crc ^= cur_nible;
      for(int j = 0; j < 4; j++)
      if (crc & 0x80) crc = (crc << 1) ^ CCIT_POLY;
      else crc <<= 1;
    }  
    cur_nible = SendBuffer[i] & 0x0F;
    summ += cur_nible;
    if (i !=2)
    {
      crc ^= cur_nible;
      for(int j = 0; j < 4; j++)
      if (crc & 0x80) crc = (crc << 1) ^ CCIT_POLY;
      else crc <<= 1;
    }  
  }
  SendBuffer[7] += summ & 0x0F;
  SendBuffer[8] += summ & 0xF0;
  SendBuffer[8] += crc & 0x0F;
  SendBuffer[9] += crc & 0xF0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////

void Oregon_TM::calculateAndSetChecksum810()
{
  byte CCIT_POLY = 0x07;
  SendBuffer[7] = SendBuffer[7] & 0xF0;
  SendBuffer[8] = 0x00;
  SendBuffer[9] = 0x00;
  byte summ = 0x00;
  byte crc = 0x00;
  byte cur_nible;
  for(int j = 0; j < 4; j++)
  if (crc & 0x80) crc = (crc << 1) ^ CCIT_POLY;
  else crc <<= 1;
  
  for(int i = 0; i<8; i++) 
  {
    cur_nible = (SendBuffer[i] & 0xF0) >> 4;
    summ += cur_nible;
   
      crc ^= cur_nible;
      for(int j = 0; j < 4; j++)
      if (crc & 0x80) crc = (crc << 1) ^ CCIT_POLY;
      else crc <<= 1;
   
    cur_nible = SendBuffer[i] & 0x0F;
    summ += cur_nible;
   
      crc ^= cur_nible;
      for(int j = 0; j < 4; j++)
      if (crc & 0x80) crc = (crc << 1) ^ CCIT_POLY;
      else crc <<= 1;
   
  }
  SendBuffer[7] += summ & 0x0F;
  SendBuffer[8] += summ & 0xF0;
  SendBuffer[8] += crc & 0x0F;
  SendBuffer[9] += crc & 0xF0;
}
//// CALCULATE CHECKSUM AAA ///////////////////////////////////////////////////////////////////////////////////////////////
void Oregon_TM::calculateAndSetChecksumAAA()
{
  
  byte CCIT_POLY = 0x07;
  //SendBuffer[7] = SendBuffer[7] & 0xF0;
  SendBuffer[8] = 0x00;
  SendBuffer[9] = 0x00;
  byte summ = 0x00;
  byte crc = 0x00;
  byte cur_nible;
  byte pp = 0x00;

  // for(int x=0; x < 9; x++){ 
  //     crc += pp;
  //     pp++;  
  //   }

  for(int j = 0; j < 4; j++)
  if (crc & 0x80) crc = (crc << 1) ^ CCIT_POLY;
  else crc <<= 1;
  
  for(int i = 0; i<8; i++) 
  {
    cur_nible = (SendBuffer[i] & 0xF0) >> 4;
    summ += cur_nible;
   
      crc ^= cur_nible;
      for(int j = 0; j < 4; j++)
      if (crc & 0x80) crc = (crc << 1) ^ CCIT_POLY;
      else crc <<= 1;
   
    cur_nible = SendBuffer[i] & 0x0F;
    summ += cur_nible;
   
      crc ^= cur_nible;
      for(int j = 0; j < 4; j++)
      if (crc & 0x80) crc = (crc << 1) ^ CCIT_POLY;
      else crc <<= 1;
   
  }
   SendBuffer[8] += summ & 0x0F;
   SendBuffer[9] += summ & 0xF0;
  //SendBuffer[9] += summ;
  // SendBuffer[7] += summ & 0x0F;
  // SendBuffer[8] += summ & 0xF0;
  // SendBuffer[8] += crc & 0x0F;
  // SendBuffer[9] += crc & 0xF0;
  //SendBuffer[9] += crc & 0XFF;
  // SendBuffer[9] |= 0x0F;
  //SendBuffer[11] = 0XFF;
  //SendBuffer[12] = 0XFF;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void Oregon_TM::SendPacket()
{
  if (sens_type == THGN132)
    calculateAndSetChecksum132();
  if (sens_type == RTGN318)
    calculateAndSetChecksum318();
  if (sens_type == THGR810)
    calculateAndSetChecksum810();
   if (sens_type == AIRx)
    calculateAndSetChecksumAAA();
  if (sens_type == THP)
    calculateAndSetChecksumTHP();

  
  sendOregon();
  digitalWrite(TX_PIN, LOW);
  if (protocol == 2){
    delayMicroseconds(TWOTR_TIME*15);
    sendOregon();
    digitalWrite(TX_PIN, LOW);
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Функции кодирования данных//////////////////////////////////////////////////////////////////////
//// SENSOR TYPE (ID) /////////////////////////////////////////////////////////////////////////////
void Oregon_TM::setType(word type)  // из скетча в параметре setType(здесь) получаем тип заголовка транспондера(датчика) AAA1, AAA2,  и т.д...
  {
    sens_type = type;               // определяем его как sens_type
    if (type == THP)
    {
      SendBuffer[0] = 0x55;
      return;
    }
    SendBuffer[0] = (type & 0xFF00) >> 8;
    SendBuffer[1] = type & 0x00FF;
  }
///// CHANNEL //////////////////////////////////////////////////////////////////////////////////////////////

void Oregon_TM::setChannel(byte channel)
  {
    byte channel_code;
    if (sens_type == THGN132)
    {
      if (channel <= 1) 
      {
        channel_code = 0x10; 
        setId(0xE3);
        send_time = 39000;
      }
      if (channel == 2) 
      {
        channel_code = 0x20; 
        setId(0xE3);
        send_time = 41000;
      }
      if (channel == 3) 
      {
        channel_code = 0x40; 
        setId(0xBB);
        send_time = 43000;
      }
      protocol = 2;
    }

    if (sens_type == RTGN318)
    {

      if (channel <= 1) 
      {
        channel_code = 0x10; 
        setId(0xF1);
        send_time = 53000;
      }
      if (channel == 2) 
      {
        channel_code = 0x20; 
        setId(0x92);
        send_time = 59000;
      }
      if (channel == 3) 
      {
        channel_code = 0x30; 
        setId(0xAA);
        send_time = 61000;
      }

      if (channel == 4) 
      {
        channel_code = 0x40; 
        setId(0x8A);
        send_time = 67000;
      }
      
      if (channel >= 5) 
      {
        channel_code = 0x50; 
        setId(0xB1);
        send_time = 71000;
      }
      protocol = 2;
    }

if (sens_type == THGR810)
    {
      if (channel <= 1) 
      {
        channel_code = 0x10; 
        setId(0xCB);
        //send_time = 53000; //original
        send_time = 15000; //mr.Tech
      }
      if (channel == 2) 
      {
        channel_code = 0x20; 
        setId(0x69);
        send_time = 59000;
      }
      if (channel == 3) 
      {
        channel_code = 0x30; 
        setId(0xAA);
        send_time = 61000;
      }
      if (channel == 4) 
      {
        channel_code = 0x40; 
        setId(0x8A);
        send_time = 67000;
      }
      if (channel == 5) 
      {
        channel_code = 0x50; 
        setId(0xB1);
        send_time = 71000;
      }
      if (channel == 6) 
      {
        channel_code = 0x60; 
        send_time = 79000;
      }
      if (channel == 7) 
      {
        channel_code = 0x70; 
        send_time = 83000;
      }
      if (channel == 8) 
      {
        channel_code = 0x80; 
        send_time = 87000;
      }
      if (channel == 9) 
      {
        channel_code = 0x90; 
        send_time = 91000;
      }
      if (channel >= 10) 
      {
        channel_code = 0xA0; 
        send_time = 93000;
      }
      protocol = 3;
    }
    // SendBuffer[2]&= 0x0F;
    // SendBuffer[2] += channel_code & 0xF0;
    
    // МОЙ ДАТЧИК AAA1 - БОРТ 1 ////
    // int send_time_transponder = 5000;
if (sens_type == AIRx)            // Определяем тип заголовка
      {
        // Временно отключено 31.07.2020 Так как формирование Заголовгов AAA1, AAA2, AAA3.... транспондеров и их каналов(таймслотов) сейчас происходит по другомупринципу.
        // Уникальный номер транспондера(идентификатор) указывается как 0xAAAx (См. в Oregon_TX_AIR.h #define AIRx    0xAAA0) 
        // Номер канала берётся из transmit([параметр 1],[параметр 2]) - параметр 2 . Тоесть, сейчас он указывается в пользовательском ino - файле. 
        //if (channel <= 1)             // Смотрим какой канал указан
        // {
          // channel_code = 0xA1;        // Номер канала (timeslot). содержит часть id заголовка - AAA1 Временно отключено 31.07.2020
          // send_time = send_time_transponder;          // интервал отправки пакетов в канале 1. Для датчика AAA1 интервал (timeslot) = 10000
        //}
        // if (channel_code = 0xA3)
        // {
        //   offset_t = 2100;
        // }
        send_time = send_time_transponder;          // интервал отправки пакетов в канале 1. Для датчика AAA1 интервал (timeslot) = 10000
        protocol = 3;
        // SendBuffer[2]&= channel_code; // Заносим в LSB номер канала. Т.о. в id указали номер канала(timeslot). Временно отключено 31.07.2020
      }
      else                            // если тип заголовка не относится к AIR, то испльзуем протокол без изменения(пока что это нужно для отладки датчиков OREGON)
      {
        SendBuffer[2]&= 0x0F;     
        SendBuffer[2] += channel_code & 0xF0;
      }
    // if (sens_type == AIR2)
    //   {
    //     send_time = send_time_transponder;
    //   }
}
///// ROLLING CODE //////////////////////////////////////////////////////////////////////////////////////////////

void Oregon_TM::setId(byte ID)
  {
    if (sens_type == AIRx){           // проверяем заголовки AIR
      SendBuffer[2] = 0x00;           // Обнулим регистр. В этом регистре начинается переменная lat_ которая содержит долготу по GPS
    }
    else                              // если нет то оставим все как было(для OREGON)
    {
    SendBuffer[2]&= 0xF0;
    SendBuffer[2] += (ID & 0xF0) >> 4;
    SendBuffer[3]&= 0x0F;
    SendBuffer[3] += (ID & 0x0F) << 4;
     }
  }
///// BATTERY FLAG //////////////////////////////////////////////////////////////////////////////////////////////

void Oregon_TM::setBatteryFlag(bool level)
  {
    SendBuffer[3] &= 0xFB;
    if (level) SendBuffer[3] |= 0x04;
  }
///////////////////////////////////////////////////////////////////////////////////////////////////

void Oregon_TM::setStartCount(byte startcount)
  {
     SendBuffer[3] &= 0xF4;
    if (startcount == 8) SendBuffer[3] |= 0x08;
    if (startcount == 2) SendBuffer[3] |= 0x02;
    if (startcount == 1) SendBuffer[3] |= 0x01;
  }
///// TEMPERATURE //////////////////////////////////////////////////////////////////////////////////////////////
  
void Oregon_TM::setTemperature(float temp)
  {
    if(temp < 0)
    {
      SendBuffer[5] = 0x08;
      temp *= -1;
    }
    else
    {
      SendBuffer[5] = 0x00;
    }
    byte tempInt = (byte) temp;
    byte td = (tempInt / 10);
    byte tf = tempInt - td * 10;
    byte tempFloat = (temp - (float)tempInt) * 10;
    
    SendBuffer[5] += (td << 4);
    SendBuffer[4] = tf;
    SendBuffer[4] |= (tempFloat << 4);
  }
///// LAT mr.Tech //////////////////////////////////////////////////////////////////////////////////////////////
  
void Oregon_TM::setLatitude(long lat_)
  {
    // !!! на приёмнике, чтобы прочесть координаты LAT, не забудь проверить наличие установленного флага во 2-м байте. адрес 0x80 
    // при чтении координат, в виде HEX, для получения правильного числа по модулю не забудь исключить этот флаг. Иначе значение координаты lat будет не нерным.
    // 
    //lat_ = 54729313; // test
    //lat_ *= -1;      // test
    //lat_ = 89999999;// максимально возможное число для переменной lat. Это даёт возможность в остатке адрессного пространства в MSB (0x80, 0x40) исвользовать 2 последних бита в качестве флага "-"
    if(lat_ < 0)
    {
      SendBuffer[2] = 0x80;             // если значение < 0 отправляю в этот бит единицу, это знак "-" для переменной lat
      lat_ *= -1;                       // делаю число противоположным
    }
    else
    {
      SendBuffer[2] = 0x00;             // если число lat положительное то обнуляю регистры
    }
    long lat_most_nibles = lat_ / 1000; // укорачиваю LON до 6-ти знаков. Т.к. макс.значение lat имеет 179 999 999, то отрезаем 3-последних символа координат по долготе
                                        // полученное значение вмещается в 5 ниблов. Это 0F FF FF - два споловинкой байта:)
    byte hi = highByte(lat_most_nibles >> 4);     // беру от LAT первых три символа. А так как они не вмещаются в байт (не хвавтает 4 бита), смещаю на 4 бита вправо
    byte hi_two = highByte(lat_most_nibles << 4); // оставшихся 4-х бита заношу в отдельную переменную и смещаю влево 
    byte low = lowByte(lat_most_nibles);
    // заполням буфер полученными данными GPS lat 
    SendBuffer[3] = 0x00;               // обнуляю
    SendBuffer[4] = 0x00;               // обнуляю
    SendBuffer[2] |= hi;                // заполняю 2-й байт(2 нибла) содержит часть lat - 1-й и 2-й символы. при этом не забывем, чтопервый бит в MSB содержит флаг 0 или 1. его не перетираем!!!
    SendBuffer[3] += hi_two;            // заполняю 3-й байт(2 нибла). он содержит часть lat - 3-й символ 
    SendBuffer[4] += low << 4;          // заполняю 4-й полубайт (1 нибл) так же путём смещения на 4 бита влево. Нибл содержит окончание координаты по широте - переменная lat
  }                                     

///// LON mr.Tech //////////////////////////////////////////////////////////////////////////////////////////////
  
void Oregon_TM::setLongitude(long lon_)
  {
    // !!! на приёмнике, чтобы прочесть координаты LON, не забудь проверить наличие установленных флагов в 4-м байте. адрес 0x80 - флаг "-" для долготы, 0x40 
    // при чтении координат, в виде HEX, для получения правильного числа по модулю не забудь исключить эти флаги. Иначе значение координаты lon будет не нерным.
    // 
    //lon_ = 89383136; // test
    // lon_ *= -1;      // test
    // lon_ = 179999999;// максимально возможное число для переменной lon. Это даёт возможность в остатке адрессного пространства в MSB MSB (0x80, 0x40) исвользовать 2 последних бита в качестве флага "-"
    if(lon_ < 0)
    { 
      SendBuffer[4] |= 0x08;             // если значение < 0 отправляю в этот бит единицу, это знак "-" для переменной lon
      lon_ *= -1;                       // делаю число противоположным(опять положительным)
    }
    else
    {
      SendBuffer[4] &= ~0x08;             // если число lon положительное то обнуляю флаг знака "+", "-" Знак плюс или минус кодируется и передаётся для правильного определения координат...
    }                                     // в этом байте так же содержится ещё один бит(флаг) определяющий параметр/переменная VAR - нужен для определения скороподъёмности
    long lon_most_nibles = lon_ / 1000; // укорачиваю LON до 6-ти знаков. Т.к. макс.значение lat имеет 179 999 999, то отрезаем 3-последних символа координат по долготе
                                        // полученное значение вмещается в 5 ниблов. Это 0F FF FF - два споловинкой байта:)
    byte hi = highByte(lon_most_nibles >> 8);// беру от LON первых три символа. А так как они не вмещаются в байт (не хвавтает 4 бита), смещаю на 8 бит вправо
    byte hi_two = highByte(lon_most_nibles); // оставшихся 4-х бита заношу в отдельную переменную и смещаю влево 
    byte low = lowByte(lon_most_nibles);
    // заполням буфер полученными данными GPS lon 
    SendBuffer[5] = 0x00;               // обнуляю
    SendBuffer[6] = 0x00;               // обнуляю
    SendBuffer[4] |= hi;                // заполняю 2-й байт(2 нибла) содержит часть lon - 1-й и 2-й символы. при этом не забывем что, первый бит в MSB содержит флаг 0 или 1. его НЕ ПЕРЕПИСЫВАТЬ!!!
    SendBuffer[5] += hi_two;            // заполняю 3-й байт(2 нибла). он содержит часть lon - 3-й символ 
    SendBuffer[6] += low;               // заполняю 4-й полубайт (1 нибл). Нибл содержит окончание координаты по долготе - переменная lon
  }

///// ALTITUDE //////////////////////////////////
    // ещё не доделано!!!
  void Oregon_TM::setAltitude(int alt_)
  {
    // alt_ = 2550; //test - максимальное значение высоты 255 * 1000 = 2550 метров. Умножение на 1000 производися на стороне приёма!
    // alt_ = 2100.5;
    // таким образом точность будет составлять 10 метров
    // ограничение высоты в один байт. Для увеличения необходимо пожертвовать еще полбайта...
    byte altitudos = alt_ / 10;
    // SendBuffer[7] = SendBuffer[7] & 0xF0;
    SendBuffer[7] = 0x00;
    // SendBuffer[8] = 0x00;
    // SendBuffer[7] += altitudos;
    SendBuffer[7] += alt_ / 10;
    //SendBuffer[8] += altitudos & 0xF0;
   // SendBuffer[9] &= 0XAA;
   //  SendBuffer[8] += summ & 0x0F;
   // SendBuffer[9] += summ & 0xF0;
  }
///// HUMIDITY //////////////////////////////////////////////////////////////////////////////////////////////

void Oregon_TM::setHumidity(byte hum)
  {
      SendBuffer[6] = (hum/10);
      SendBuffer[6] += (hum - (SendBuffer[6] * 10)) << 4;
  }
///// COMFORT //////////////////////////////////////////////////////////////////////////////////////////////

void Oregon_TM::setComfort(float temp, byte hum)
  {
      if (hum > 70)
     {
      SendBuffer[7] = 0xC0;
      return;
     }
      if (hum < 40)
     {
      SendBuffer[7] = 0x80;
      return;
     }
      if (temp > 20 && temp < 25)
     {
      SendBuffer[7] = 0x40;
      return;
     }
     else SendBuffer[7] = 0x00;
     return;
    // SendBuffer[10] = 0x11;
    // SendBuffer[11] = 0x22;
  }
// Передача - mr.Tech //////////////////////////////////////////////////////////////////////
int corrector_;
bool Oregon_TM::transmit(int second)
{
    if ( second >= offset_t &&  millis() >= time_marker_send) 
    ///////// if (second >= offset_t && millis() >= time_marker_send && send_time)
      {
        corrector_ = (second - offset_t) ;
        if (corrector_ < 0) corrector_ = 0;
      Serial.print(" !!!!!!!!!!!!!!!Секунды полученные через transmit библиотеку!!!!!!!!!!!!!!!!!: ");Serial.println(second);
      digitalWrite(PTT_PIN, LOW);// Включение режима TX на 7-й ноге - начало TX. Добавлено 09.03.2020
      digitalWrite(TX_LED, LOW);
      GPSsquelshOn(); //отключаю чтение потока с GPS на время передачи
      delay(TR_TIME); // задержка перед началом преамбулы.
      SendPacket();
      digitalWrite(PTT_PIN, HIGH);// Отключение режима TX на 7-й ноге - конец TX. Добавлено 09.03.2020
      digitalWrite(TX_LED, HIGH);
      GPSsquelshOff(); // обратно подключаю поток с GPS приёмника
      // time_marker_send = ((millis() + 5000) - corrector_ ) + TR_TIME;
      time_marker_send = ((millis() + send_time) - corrector_) + 334;
      // time_marker_send = ((millis() + send_time) - corrector_) + 1000;
      return true;
}
  else return false;
}

void Oregon_TM::GPSsquelshOn()
{
    pinMode(5, OUTPUT);
    digitalWrite(5, HIGH);
    pinMode(4, INPUT);
    digitalWrite(4, HIGH);
}
void Oregon_TM::GPSsquelshOff()
{
    pinMode(5, INPUT);
    digitalWrite(5, LOW);
    pinMode(4, OUTPUT);
    digitalWrite(4, LOW);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//Поддержка датчика THP
///////////////////////////////////////////////////////////////////////////////////////////////////


void Oregon_TM::setChannelTHP(byte channel)
  {
  SendBuffer[1] &= 0x0F;
  SendBuffer[1] += channel << 4;
  }
///////////////////////////////////////////////////////////////////////////////////////////////////  
void Oregon_TM::setBatteryTHP( word bat_voltage)
  {
    SendBuffer[6] = (bat_voltage & 0x0FF0) >> 4;
    SendBuffer[7] &= 0x0F;
    SendBuffer[7] += (bat_voltage & 0x000F) << 4;
   
  }
///////////////////////////////////////////////////////////////////////////////////////////////////  
void Oregon_TM::setTemperatureTHP(float bme_temperature)
  {
    word temp_code;
    if (bme_temperature < -100 || bme_temperature > 100) temp_code = 0x0FFF;
    else temp_code = (word)((bme_temperature + 100) * 10);
    SendBuffer[2] = temp_code & 0x00FF;
    SendBuffer[1] &= 0xF0;
    SendBuffer[1] += (temp_code & 0x0F00) >> 8;
  }
///////////////////////////////////////////////////////////////////////////////////////////////////  
void Oregon_TM::setHumidityTHP(float bme_humidity)
  {
    word hum_code;
    if (bme_humidity > 100) hum_code = 0x0FFF;
    else hum_code = (word)(bme_humidity * 10);
    SendBuffer[3] = (hum_code & 0x0FF0) >> 4;
    SendBuffer[4] &= 0x0F;
    SendBuffer[4] += (hum_code & 0x000F) << 4;
  }
///////////////////////////////////////////////////////////////////////////////////////////////////
void Oregon_TM::setPressureTHP(float bme_pressure)
  {
    word pres_code;
    if (bme_pressure < 500) pres_code = 0x0000;
    else pres_code = (word)((bme_pressure - 500) * 10);
    SendBuffer[5] = pres_code & 0x00FF;
    SendBuffer[4] &= 0xF0;
    SendBuffer[4] += (pres_code & 0x0F00) >> 8;
  }
///////////////////////////////////////////////////////////////////////////////////////////////////
void Oregon_TM::setErrorTHP()
  {
    SendBuffer[1] |= 0x0F;
    SendBuffer[2] = 0xFF;
    SendBuffer[3] = 0xFF;
    SendBuffer[4] = 0xFF;
    SendBuffer[5] = 0xFF;
  }
///////////////////////////////////////////////////////////////////////////////////////////////////
void Oregon_TM::calculateAndSetChecksumTHP()
{
  byte CCIT_POLY = 0x07;
  SendBuffer[7] = SendBuffer[7] & 0xF0;
  SendBuffer[8] = 0x00;
  SendBuffer[9] = 0x00;
  byte summ = 0x00;
  byte crc = 0x00;
  byte cur_nible;
  for(int i = 0; i<8; i++) 
  {
    cur_nible = (SendBuffer[i] & 0xF0) >> 4;
    summ += cur_nible;
    crc ^= cur_nible;
    for(int j = 0; j < 4; j++)
      if (crc & 0x80) crc = (crc << 1) ^ CCIT_POLY;
      else crc <<= 1;
      
    cur_nible = SendBuffer[i] & 0x0F;
    summ += cur_nible;
    crc ^= cur_nible;
    for(int j = 0; j < 4; j++)
      if (crc & 0x80) crc = (crc << 1) ^ CCIT_POLY;
      else crc <<= 1;
  }
  SendBuffer[7] += summ & 0x0F;
  SendBuffer[8] += summ & 0xF0;
  SendBuffer[8] += crc & 0x0F;
  SendBuffer[9] += crc & 0xF0;
}

