#include "Oregon_NR_AIR.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// !!!!!!!!!!! Release by R3PB. AIR-TRANSPONDER SYSTEM !!!!!!!!!!!!!
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
// PCR800 added by mr.Tech since 20.02.2020
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


//Âñ¸, ÷òî îòíîñèòñÿ ê ïðåðûâàíèþ/////////////////////////////////////
#ifndef Oregon_NR_int
#define Oregon_NR_int
static volatile unsigned long pm;
static volatile unsigned long pl, timer_mark;  

#if defined ( ESP8266 )
void ICACHE_RAM_ATTR receiver_interruption(void) {  
#elif defined ( ESP32 )
void ICACHE_RAM_ATTR receiver_interruption(void) {  
#else
void receiver_interruption(void) {  
#endif

  if(digitalRead(RECEIVER_PIN)){
  //Начало импульса 
    pl = 0;
    pm = micros();  
  }
  else{
    //Конец импульса
    //Вычисляется время окончания и длина
    pl = micros() - pm;  
    //pm += pl;
  }
  //yield();
}  
#endif

//////////////////////////////////////////////////////////////////////
Oregon_NR::Oregon_NR(byte MHZ, byte MHZ_INT)
{
  INT_NO = MHZ_INT;
  RECEIVER_PIN = MHZ;
  pinMode(MHZ, INPUT); // Вывод, на который подключён приёмник
  digitalWrite(MHZ, HIGH); // ДОБАВЛЕННО mr.Tech ВРЕМЕННО ТОЛЬКО ДЛЯ ОТЛАДКИ. нЕОБХОДИМО УДАЛИТЬ!
}

Oregon_NR::Oregon_NR(byte MHZ, byte MHZ_INT, byte led, bool pull_up)
{
  INT_NO = MHZ_INT;
  LED = led;
  PULL_UP = pull_up;
  RECEIVER_PIN = MHZ;
  pinMode(MHZ, INPUT);    // Вывод, на который подключён приёмник
  pinMode(LED, OUTPUT);   // Вывод светодиода
  digitalWrite(MHZ, HIGH); // ДОБАВЛЕННО mr.Tech ВРЕМЕННО ТОЛЬКО ДЛЯ ОТЛАДКИ. нЕОБХОДИМО УДАЛИТЬ!
}

//////////////////////////////////////////////////////////////////////
void Oregon_NR::start()
{
  packet_number = 0;
  packets_received = 0;
  start_pulse_cnt = 0;
  receive_status = FIND_PACKET;
  led_light(false);
  attachInterrupt(INT_NO, receiver_interruption, CHANGE);  
}
//////////////////////////////////////////////////////////////////////
void Oregon_NR::stop()
{
  detachInterrupt(INT_NO);  
}
//////////////////////////////////////////////////////////////////////
//Захват и анализ пакета
//DEBUG_INFO - в Serial выводится информация о захвате данных 
//////////////////////////////////////////////////////////////////////
void Oregon_NR::capture(bool DEBUG_INFO)
{
////////////////////////////////////////////////////////
// Возвращаемся к исходному состоянию
//maybe_packet = 0; 
  packets_received = 0;
  sens_type = 0;
  crc_c = 0;
  captured = 0;
  data_val = 0;
  data_val2 = 0;

  
  ////////////////////////////////////////////////////////
  // Чтение данных с приёмника 
  noInterrupts();
  pulse_length = pl;
  pl = 0;
  pulse_marker = pm;
  interrupts();

  ////////////////////////////////////////////////////////
  // Пришёл импульс
  if (pulse_length != 0 && receive_status == FIND_PACKET){  
  // Если импульс пришёл слишком поздно для конкретной версии протокола, то это первый импульс 
    if (pulse_marker - pre_marker > (PER_LENGTH2 * 2 + LENGTH_TOLERANCE) && ver == 2) start_pulse_cnt = 0;
    if (pulse_marker - pre_marker > (PER_LENGTH3 * 2 + LENGTH_TOLERANCE) && ver == 3) start_pulse_cnt = 0;

    if (start_pulse_cnt == 0){
      ////////////////////////////////////////////////////////
      //Найдена первый "правильный" импульс - определяем тип протокола
      if (pulse_length < (MAX_LENGTH2 + LENGTH_TOLERANCE) && pulse_length > (MIN_LENGTH2 -  LENGTH_TOLERANCE) && catch2 ) {
        start_pulse_cnt++;
        pre_marker = pulse_marker;
        pulse_length = 0;
	ver = 2;
      }

      if (pulse_length < (MAX_LENGTH3 + LENGTH_TOLERANCE) && pulse_length > (MIN_LENGTH3 -  LENGTH_TOLERANCE) && catch3 ) {
        start_pulse_cnt++;
        pre_marker = pulse_marker;
        pulse_length = 0;
	ver = 3;
      }
    }
    else{
      ///////////////////////////////////////////////////////////
      //Найдена следующий "правильный" импульс
      if (pulse_length <= (MAX_LENGTH2 + LENGTH_TOLERANCE) && pulse_length >= (MIN_LENGTH2 -  LENGTH_TOLERANCE) && catch2) {
        ///////////////////////////////////////////////////////////
        //Åñëè èìïóëüñ â ïðàâèëüíîì ìåñòå, òî äîáàâëÿåì ñ÷¸ò÷èê íàéäåíûõ ñòàðòîâûõ èìïóëüñîâ
        if(pulse_marker - pre_marker > (PER_LENGTH2*2-LENGTH_TOLERANCE) && pulse_marker - pre_marker < (PER_LENGTH2 * 2 + LENGTH_TOLERANCE) && ver == 2)
	{
          start_pulse_cnt++;
          pre_marker = pulse_marker;
          pulse_length = 0;
        }
        ///////////////////////////////////////////////////////////
        //Следующий импульс находится в неправильном месте
        //Назначаем его первым
        else
	{
          start_pulse_cnt = 1;
          pre_marker = pulse_marker;
          pulse_length = 0;
          ver = 2;
        }
      }

      if (pulse_length <= (MAX_LENGTH3 + LENGTH_TOLERANCE) && pulse_length >= (MIN_LENGTH3 -  LENGTH_TOLERANCE) && catch3) {
        ///////////////////////////////////////////////////////////
        //Если импульс в правильном месте, то добавляем счётчик найденых стартовых импульсов
        if(pulse_marker - pre_marker > (PER_LENGTH3*2-LENGTH_TOLERANCE) && pulse_marker - pre_marker < (PER_LENGTH3 * 2 + LENGTH_TOLERANCE) && ver == 3)
	{
          start_pulse_cnt++;
          pre_marker = pulse_marker;
          pulse_length = 0;
        }
        ///////////////////////////////////////////////////////////
        //Следующий импульс находится в неправильном месте
        //Назначаем его первым
        else
	{
          start_pulse_cnt = 1;
          pre_marker = pulse_marker;
          pulse_length = 0;
          ver = 3;
        }
      }
    }
  }
//*************************************************************************************
  /////////////////////////////////////////////////////////////////////
  // Если найден первый пакет и вышло вермя ожидания второго
  // Не ждём второго, а переходм в режим анализа
  // Также, не ждём второго пакета если отключЄн режим сращивани¤ пакетов
  if (packet_number == 1 && (millis() - first_packet_end) > 200) receive_status = ANALIZE_PACKETS;
  if (packet_number == 1 && (!IS_ASSEMBLE || ver == 3 )) receive_status = ANALIZE_PACKETS;

  
  //////////////////////////////////////////////////////////////////////
  //Если Найдено нужное количество правильных импульсов в нужных местах, то возможно это пакет. Начинаем СБОР ДАННЫХ

  if (start_pulse_cnt == CATCH_PULSES && receive_status == FIND_PACKET) {
    

    start_pulse_cnt = 0;
    if (packet_number == 0)
    {
      read_tacts = collect(collect_data);
      first_packet_end = millis();
      packet_number = 1;
    }
    else
    {
      read_tacts2 = collect(collect_data2);
      packet_number = 2;
      receive_status = ANALIZE_PACKETS;
    }
  }
  //////////////////////////////////////////////////////////////////////
  // Анализ данных////////////////////////////////////////////////////////
  if  (receive_status == ANALIZE_PACKETS) {     
    //////////////////////////////////////////////////////////////////////
    //Если пришёл только кусок посылки, то не стоит и обрабатывать
    if ((ver ==2 && read_tacts < 136 && read_tacts2 < 136) || (ver ==3 && read_tacts < 80)) 
    {
      receive_status = FIND_PACKET;
      start_pulse_cnt = 0;  
      packet_number = 0;
       return;
    }
    //Отключаем прерывание, чтобы сократить время обработки
    detachInterrupt(INT_NO);
   
    led_light(true);
    restore_sign = 0;
    work_time = millis();   //Расчёт времени обработки пакета
      //Дамп собранных данных       
      //Для посылки без помех значения импульсов должны быть примерно
      // v2 - 87 07  и изредка 86 06, т.к. длина импульса 883мс и 395мс
      // v3 - 86 06 и изредка 87 07  т.к. длина импульса 838 и 350мс
        if (DEBUG_INFO && receiver_dump){
          Serial.println(" ");
          Serial.print("SCOPE1 ");
          for(int bt = 0; bt < read_tacts; bt++){
            Serial.print((collect_data[bt] & 0xF0) >> 4, HEX);
            Serial.print(collect_data[bt] & 0x0F, HEX);
            Serial.print(' ');
          }
          Serial.println(" ");
          if ( packet_number == 2)
          {
            Serial.print("SCOPE2 ");
            for(int bt = 0; bt < read_tacts2; bt++){
              Serial.print((collect_data2[bt] & 0xF0) >> 4, HEX);
              Serial.print(collect_data2[bt] & 0x0F, HEX);
              Serial.print(' ');
            }
            Serial.println(" ");
          }

        }


    //////////////////////////////////////////////
    //Обработка первой записи
    //Расшифровываем запись. Данные сохраянем в decode_tacts[]
    get_tacts(collect_data, read_tacts);
    bool halfshift;

    if (get_data(0, ver, collect_data) > get_data(1, ver, collect_data)){
      data_val = get_data(0, ver, collect_data);
      halfshift = 0;
    }
    else {
      data_val = get_data(1, ver, collect_data);
      halfshift = 1;
    }
    //////////////////////////////////////////////
    //Ищем позицию синхронибла
    synchro_pos = get_synchro_pos(collect_data);
    //////////////////////////////////////////////
    //Выводим посылку
    if (DEBUG_INFO){
    if (packet_number == 2)   Serial.print("1)     ");
    if (packet_number == 1)   Serial.print("RESULT ");
      for(int bt = 0; bt < READ_BITS; bt++) {
        if ((ver == 3 && bt <= read_tacts) || (ver == 2 && bt <= read_tacts / 2)){
          if (collect_data[bt] > 128 + 1) Serial.print('I');
          if (collect_data[bt] < 128 - 1) Serial.print('O');
          if (collect_data[bt] == 128 + 1) Serial.print('i');
          if (collect_data[bt] == 128 - 1) Serial.print('o');
          if (collect_data[bt] == 128) Serial.print('.');
           if (receiver_dump && ver == 2) Serial.print("     ");
           if (receiver_dump && ver == 3) Serial.print("  ");
        }
        else Serial.print(' ');
      }
        Serial.print(" OSV:");
        Serial.print(ver);
        Serial.print(" SYN:");
	if (synchro_pos < 255)
        Serial.print(synchro_pos);
        else  Serial.print("NO");
        Serial.print(" TIME:");
        Serial.println (millis() / 1000);
    }
    //////////////////////////////////////////////
    //Аналогично обрабатываем вторую запись
    if (packet_number == 2){
      get_tacts(collect_data2, read_tacts2);

      if (get_data(0, ver, collect_data2) > get_data(1, ver, collect_data2)) {
        data_val2 = get_data(0, ver, collect_data2);
        halfshift = 0;
      }
      else {
        data_val2 = get_data(1, ver, collect_data2);
        halfshift = 1;
      }

      synchro_pos2 = get_synchro_pos(collect_data2);
      if (DEBUG_INFO){
        Serial.print("2)     ");
       for(int bt = 0; bt < READ_BITS; bt++) {
        if (bt <= read_tacts2 / 2){
          if (collect_data2[bt] > 128 + 1) Serial.print('I');
          if (collect_data2[bt] < 128 - 1) Serial.print('O');
          if (collect_data2[bt] == 128 + 1) Serial.print('i');
          if (collect_data2[bt] == 128 - 1) Serial.print('o');
          if (collect_data2[bt] == 128) Serial.print('.');
           if (receiver_dump && ver == 2) Serial.print("     ");
           if (receiver_dump && ver == 3) Serial.print("  ");
        }
        else Serial.print(' ');
      }

        Serial.print(" OSV:");
        Serial.print(ver);
        Serial.print(" SYN:");
	if (synchro_pos2 < 255)
          Serial.print(synchro_pos2);
        else
          Serial.print("NO");;
        }
      }
      byte* result_data, result_data_start, aux_data;
      int correlation;
      


      //////////////////////////////////////////////
      //СОПОСТАВЛЕНИЕ ПАКЕТОВ
      //Если пакет один, то и сопоставлять не из чего
       if (packet_number == 1) result_data = collect_data;     
       //////////////////////////////////////////////
       //А вот если два, то нужна СБОРКА ПАКЕТА
       //вычисляем оптимальное "смещение" пакетов друг относительно друга
       if (packet_number == 2) {
        correlation = correlate_data(collect_data, collect_data2);
        if (DEBUG_INFO){
          Serial.print(" COR: ");
          Serial.println(correlation);
        }
        //////////////////////////////////////////////
        //Собираем данные в пакет, где синхронибл найден раньше                
        //////////////////////////////////////////////

        if (synchro_pos >= synchro_pos2)
        {
          result_size = read_tacts2;
          result_data = collect_data2;
          correlation = -correlation;
          assemble_data(collect_data2, collect_data, correlation);
        }
        else
        {
          result_size = read_tacts;
          result_data = collect_data;
          assemble_data(collect_data, collect_data2, correlation);
        }
      }
    //////////////////////////////////////////////
    //Вывод готовой посылки
    if (DEBUG_INFO && packet_number == 2){
      Serial.print("RESULT ");
      byte* rdt = result_data;
      for(int bt = 0; bt < READ_BITS; bt++){
        if (bt <= result_size / 2){
          if (*rdt > 128 + 1) Serial.print('I');
          if (*rdt < 128 - 1) Serial.print('O');
          if (*rdt == 128 + 1) Serial.print('i');
          if (*rdt == 128 - 1) Serial.print('o');
          if (*rdt == 128) Serial.print('.');
           if (receiver_dump && ver == 2) Serial.print("     ");
           if (receiver_dump && ver == 3) Serial.print("  ");
        }
        else Serial.print(' ');
          rdt++;
      }
      Serial.println(" ");
    }



     //Проверяем, дало ли что-нибудь сращивание - отключил. Это даёт всего лишь флаг, но занимает много времени
    //////////////////////////////////////////////

 //   if (get_data(halfshift, ver, result_data) > data_val && get_data(halfshift, ver, result_data) > data_val2 && ver == 2)
//	restore_sign ^= 8;

    //////////////////////////////////////////////
    //Извлекаем из тактовой последовательности биты
    sens_type = 0;
    if (get_info_data(result_data, packet, valid_p))
    {
      sens_type = get_sensor(packet);         //Определяем тип пакета по типу датчика
      transponder = get_transponder(packet);  // Определяем префикс транспондера (префикс AAA)
      // tr_number = get_tr_number(packet);      // Определяем номер транспондера - цифра идущая после префикса 
      restore_data(packet, sens_type);        // Восстанавливаем данные по типу датчика
      crc_c = check_CRC(packet, sens_type);   // Проверяем CRC, если оно верно, то все сомнительные биты делаем уверенными
      //Если не все байты определены уверенно, нельзя считать, что пакет корректный
      byte secresingV;
      if (sens_type == THGN132 || (sens_type & 0xFF00) == GAS) secresingV = PACKET_LENGTH - 4;
      if (sens_type == THN132) secresingV = PACKET_LENGTH - 6;
      for (byte www = 0; www < (PACKET_LENGTH - secresingV + 2); www++)
      if (valid_p[www] < 0x0f) crc_c = false;
      //Захват пакета происходит только в случае, если найдена стартовая последовательность (нибл синхронизации)
      //Если не было синхрониблов - то не о чем вообще разговаривать
      if ( synchro_pos != 255 && packet_number == 1)  captured = 1;
      if ( (synchro_pos2 != 255 || synchro_pos2 != 255) && packet_number == 2)  captured = 1;
      //Захват куска посылки не считается
      if ((ver ==2 && read_tacts < 136) || (ver ==3 && read_tacts < 80))   captured = 0;
    }
    // else if (data_val >=64 || data_val2 >=64) maybe_packet = 1;

// #if ADD_SENS_SUPPORT == 1
//       sens_tmp2 = 404;
//       sens_CO = 255;
//       sens_CH = 255;
//       sens_pressure = 0;
//       sens_voltage = 0;
//       sens_current = 0;
//       sens_ip22  = 0;
//       sens_ip72 = 0;
//       sens_lockalarm = 0;
//       sens_pump_count = 0;
//       sens_capacitance = 0;
//       sens_drop_counter = 0;
// #endif
    sens_id = 0;
    sens_chnl = 0;
    sens_battery = 0;
    sens_tmp = 0;
    sens_hmdty = 0;
    UV_index = 0;
    lightness = 0;
    sens_avg_ws = 0;
    sens_max_ws = 0;
    sens_wdir = 0;
    rain_rate = 0;
    total_rain = 0;

////////////////////////////////////////////////////////////////////////////////////////////////////    
//Расшифровка датчиков Орегон
////////////////////////////////////////////////////////////////////////////////////////////////////    
    if ((sens_type == THGN132 || sens_type == THN132 || (sens_type  & 0x0FFF)== RTGN318 || sens_type == THGR810) && crc_c){

      sens_id = get_id(packet);
      sens_chnl = get_channel(packet);
      sens_battery = get_battery(packet);
      sens_tmp = get_temperature(packet);
      if (sens_type == THGN132 || (sens_type  & 0x0FFF)== RTGN318 || sens_type == THGR810) 
        sens_hmdty = get_humidity(packet);
      else sens_hmdty = 0;
    }

    if (sens_type == WGR800 && crc_c){
      sens_id = get_id(packet);
      sens_battery = get_battery(packet);
      sens_avg_ws = get_avg_windspeed(packet);
      sens_max_ws = get_max_windspeed(packet);
      sens_wdir = get_winddirection(packet);
    }   

    if (sens_type == UVN800 && crc_c){
      sens_id = get_id(packet);
      sens_battery = get_battery(packet);
      UV_index = get_UV(packet);
      lightness = get_light(packet);
    }

    if (sens_type == PCR800 && crc_c){
      sens_id = get_id(packet);
      sens_battery = get_battery(packet);
      rain_rate = get_rain_rate(packet);
      total_rain = get_total_rain(packet);
    }

////////////////////////////////////////////////////////////////////////////////////////////////////    
//Расшифровка датчиков AIR транспондеров
////////////////////////////////////////////////////////////////////////////////////////////////////    
    if (transponder == AIR && crc_c){
    // if (sens_type == AIR1 && crc_c){
      lon_ = get_longitude(packet);
      lat_ = get_latitude(packet);
      alt_ = get_altitude(packet);
    }


// #if ADD_SENS_SUPPORT == 1
//////////////////////////////////////////////////////////////////////////////////////////////////    
//Расшифровка комплексных газовых датчиков
////////////////////////////////////////////////////////////////////////////////////////////////////    
    // if ((sens_type & 0xFF00) == GAS && crc_c){
    //   sens_id = 0;
    //   sens_battery = 0;
    //   sens_chnl = get_gas_channel(packet);
    //   sens_tmp = get_gas_temperature_out(packet);
    //   if (packet[9] == 0x0F) sens_tmp = 404;
    //   sens_tmp2 = get_gas_temperature_in(packet);
    //   if (packet[12] == 0x0F) sens_tmp2 = 404;
    //   sens_hmdty = get_gas_hmdty(packet);
    //   sens_CO = get_gas_CO(packet);
    //   sens_CH = get_gas_CH(packet);
    // }
//////////////////////////////////////////////////////////////////////////////////////////////////    
//Расшифровка датчиков пожарной сигнализации
////////////////////////////////////////////////////////////////////////////////////////////////////    
    // if ((sens_type & 0xFF00) == FIRE && crc_c){
    //   sens_id = 0;
    //   sens_battery = 0;
           
    //   sens_chnl = get_gas_channel(packet);
    //   sens_ip22 = get_fire_ip22(packet);
    //   sens_ip72 = get_fire_ip72(packet);
    //   sens_lockalarm = get_fire_lockalarm(packet);
    // }
//////////////////////////////////////////////////////////////////////////////////////////////////    
//Расшифровка датчиков THP
////////////////////////////////////////////////////////////////////////////////////////////////////    
    // if ((sens_type & 0xFF00) == THP && crc_c){
    //   sens_chnl = get_gas_channel(packet);
    //   sens_voltage = get_thp_voltage(packet);
    //   sens_tmp = get_thp_temperature(packet);
    //   sens_hmdty = get_thp_humidity(packet);
    //   sens_pressure = get_thp_pressure(packet);
    // }

//////////////////////////////////////////////////////////////////////////////////////////////////    
//Расшифровка датчиков тока и напряжения
////////////////////////////////////////////////////////////////////////////////////////////////////    
    // if ((sens_type & 0xFF00) == CURRENT && crc_c){
    //   sens_id = 0;
    //   sens_battery = 0;
           
    //   sens_chnl = get_gas_channel(packet);
    //   sens_current = get_current(packet);
    //   sens_voltage = get_voltage(packet);
    //   sens_pump_count = get_pump_count(packet);
    // }

//////////////////////////////////////////////////////////////////////////////////////////////////    
//Расшифровка датчиков осадков емкостного типа
////////////////////////////////////////////////////////////////////////////////////////////////////    
    // if ((sens_type & 0xFF00) == CAPRAIN && crc_c){
    //   sens_id = 0;
    //   sens_battery = 0;
           
    //   //sens_heater = 
    //   sens_drop_counter = get_dropcounter(packet);
    //   sens_capacitance = get_capacitance(packet);
    // }
    
// #endif
////////////////////////////////////////////////////////////////////////////////
//Остальные вычисления
//Возвращаем всё в исходное состояние и включаем прослушивание приёмника
    work_time = millis() - work_time;
    packets_received = 0;
    if (data_val >= 64 && synchro_pos != 255) packets_received++;
    if (data_val2 >= 64 && synchro_pos2 != 255) packets_received++;
    receive_status = FIND_PACKET;
    start_pulse_cnt = 0;  
    packet_number = 0;
    led_light(false);
    //Serial.print("LED = ");
    //Serial.println(LED);
    attachInterrupt(INT_NO, receiver_interruption, CHANGE);  
  }
}
////////////////////////////////////////////////////////////////////////////////////////////////////
//Извлекает из записи тактовую последовательности
//Параметры: cdptr - указатель на записанную тактовую последовательность
//Результат пишется в массив decode_tacts
////////////////////////////////////////////////////////////////////////////////////////////////////
void Oregon_NR::get_tacts(byte* cdptr, byte bitsize){
  
//Сброс массивов
  for(int bt = 0 ; bt < bitsize; bt++) decode_tacts[bt] = 2;      //Изначально такт неизвестен

//Расшифровка тактов      
  byte* cdp = cdptr;        
  for(int bt = 0 ; bt < bitsize; bt++)
  {
      if (ver == 2 && decode_method == 1)  
    {
      if ((*cdp & 0xf0) > 0x20 && (*cdp & 0x0f) > 0x03) decode_tacts[bt] = 1;
      if ((*cdp & 0xf0) < 0x30 && (*cdp & 0x0f) < 0x05) decode_tacts[bt] = 0;
      if ((*cdp & 0xf0) < 0x20 && (*cdp & 0x0f) > 0x04) decode_tacts[bt] = 4;
      if ((*cdp & 0xf0) > 0x40 && (*cdp & 0x0f) < 0x02) decode_tacts[bt] = 3;
    }
      if (ver == 2 && decode_method == 2)  
    {
      if (*cdp == 0x88 || *cdp == 0x87  || *cdp == 0x86 || *cdp == 0x85 || *cdp == 0x84 || *cdp == 0x83 || *cdp == 0x78 ||  *cdp == 0x77 || *cdp == 0x68 || *cdp == 0x58 ) decode_tacts[bt] = 1;              // Òàêò 11 (Â ÈÄÅÀËÅ 87, ÍÎ ÈÇ ÇÀ ÑÄÂÈÃÀ ÍÀ 3 ÒÀÊÒÀ ÌÎÆÅÒ ÁÛÒÜ ÎÒ 58 À 84)
      if (*cdp == 0x00 || *cdp == 0x01  || *cdp == 0x02 || *cdp == 0x03 || *cdp == 0x10 || *cdp == 0x20 || *cdp == 0x30) decode_tacts[bt] = 0;                                                         // Òàêò 00 (Â ÈÄÅÀËÅ 00, ÍÎ ÈÇ ÇÀ ÑÄÂÈÃÀ ÍÀ 3 ÒÀÊÒÀ ÌÎÆÅÒ ÁÛÒÜ ÎÒ 30 ÄÀ 03)
      if (*cdp == 0x05 || *cdp == 0x06  || *cdp == 0x07 || *cdp == 0x08 || *cdp == 0x15 || *cdp == 0x16 || *cdp == 0x17 || *cdp == 0x24 || *cdp == 0x25 || *cdp == 0x34 || *cdp == 0x35) decode_tacts[bt] = 4; // Òàêò 01 (Â ÈÄÅÀËÅ 07, ÍÎ ÈÇ ÇÀ ÑÄÂÈÃÀ ÍÀ 3 ÒÀÊÒÀ ÌÎÆÅÒ ÁÛÒÜ ÄÎ 34)
      if (*cdp == 0x50 || *cdp == 0x60  || *cdp == 0x70 || *cdp == 0x80 || *cdp == 0x51 || *cdp == 0x61 || *cdp == 0x71 || *cdp == 0x42 || *cdp == 0x52 || *cdp == 0x43 || *cdp == 0x53) decode_tacts[bt] = 3; // Òàêò 10 (Â ÈÄÅÀËÅ 70, ÍÎ ÈÇ ÇÀ ÑÄÂÈÃÀ ÍÀ 3 ÒÀÊÒÀ ÌÎÆÅÒ ÁÛÒÜ ÄÎ 43)
    }
    if (ver == 3 && decode_method == 1)  
    {
      if ((*cdp & 0xf0) > 0x20 && (*cdp & 0x0f) > 0x04) decode_tacts[bt] = 1;
      if ((*cdp & 0xf0) < 0x30 && (*cdp & 0x0f) < 0x05) decode_tacts[bt] = 0;
      if ((*cdp & 0xf0) < 0x20 && (*cdp & 0x0f) > 0x02) decode_tacts[bt] = 4;
      if ((*cdp & 0xf0) > 0x20 && (*cdp & 0x0f) < 0x02) decode_tacts[bt] = 3;
    }
    if (ver == 3 && decode_method == 2)  
    {
      if (*cdp == 0x87 || *cdp == 0x86  || *cdp == 0x85 || *cdp == 0x84 || *cdp == 0x83 || *cdp == 0x82 || *cdp == 0x78 ||  *cdp == 0x77 || *cdp == 0x76 || *cdp == 0x68 || *cdp == 0x67 ) decode_tacts[bt] = 1;              // Òàêò 11 (Â ÈÄÅÀËÅ 87, ÍÎ ÈÇ ÇÀ ÑÄÂÈÃÀ ÍÀ 3 ÒÀÊÒÀ ÌÎÆÅÒ ÁÛÒÜ ÎÒ 58 À 84)
      if (*cdp == 0x00 || *cdp == 0x01  || *cdp == 0x02 || *cdp == 0x03 || *cdp == 0x10 || *cdp == 0x20 || *cdp == 0x30) decode_tacts[bt] = 0;                                                         // Òàêò 00 (Â ÈÄÅÀËÅ 00, ÍÎ ÈÇ ÇÀ ÑÄÂÈÃÀ ÍÀ 3 ÒÀÊÒÀ ÌÎÆÅÒ ÁÛÒÜ ÎÒ 30 ÄÀ 03)
      if (*cdp == 0x05 || *cdp == 0x06  || *cdp == 0x07 || *cdp == 0x08 || *cdp == 0x15 || *cdp == 0x16 || *cdp == 0x17 || *cdp == 0x24 || *cdp == 0x25 || *cdp == 0x34 || *cdp == 0x35) decode_tacts[bt] = 4; // Òàêò 01 (Â ÈÄÅÀËÅ 07, ÍÎ ÈÇ ÇÀ ÑÄÂÈÃÀ ÍÀ 3 ÒÀÊÒÀ ÌÎÆÅÒ ÁÛÒÜ ÄÎ 34)
      if (*cdp == 0x50 || *cdp == 0x60  || *cdp == 0x70 || *cdp == 0x80 || *cdp == 0x51 || *cdp == 0x61 || *cdp == 0x71 || *cdp == 0x42 || *cdp == 0x52 || *cdp == 0x43 || *cdp == 0x53) decode_tacts[bt] = 3; // Òàêò 10 (Â ÈÄÅÀËÅ 70, ÍÎ ÈÇ ÇÀ ÑÄÂÈÃÀ ÍÀ 3 ÒÀÊÒÀ ÌÎÆÅÒ ÁÛÒÜ ÄÎ 43)
    }
      if (decode_method == 3)  
    {
      if ((((*cdp) >> 4) + (*cdp & 0x0F)) < 5)  decode_tacts[bt] = 0;
      if (((((*cdp) >> 4) + (*cdp & 0x0F)) > 4) && ((((*cdp) >> 4) + (*cdp & 0x0F)) < 10))
	{
          if (((*cdp) >> 4) > (*cdp & 0x0f))  decode_tacts[bt] = 3;    
          if (((*cdp) >> 4) < (*cdp & 0x0f))  decode_tacts[bt] = 4;  
          if (((*cdp) >> 4) == (*cdp & 0x0f) && (*(cdp - 1) & 0x0F) < 4 )  decode_tacts[bt] = 4;  
          if (((*cdp) >> 4) == (*cdp & 0x0f) && (*(cdp - 1) & 0x0F) > 4 )  decode_tacts[bt] = 3;  
	}
      if ((((*cdp) >> 4) + (*cdp & 0x0F)) > 10) decode_tacts[bt] = 1;

    }
   *cdp++;
  }

// Печать расшифорвки
 if (receiver_dump)
 {
   byte* cdp = cdptr;        
   Serial.print("BEFORE ");


  for(int bt = 0 ; bt < bitsize; bt++)

   {

     if (decode_tacts[bt] == 1) Serial.print("II"); 
     if (decode_tacts[bt] == 0) Serial.print("OO"); 
     if (decode_tacts[bt] == 2) Serial.print("__"); 
     if (decode_tacts[bt] == 3) Serial.print("IO"); 
     if (decode_tacts[bt] == 4) Serial.print("OI"); 
     Serial.print(" "); 
     *cdp++;
  }
   Serial.println();
 }


// Расшифровали всё, что смогли с ходу
//Проверяем допустима ли тактовая последовательность

  for(int bt = 1; bt < bitsize; bt++)
  {
//    if (decode_tacts[bt] == 2)
    {
       //Õ0 0X - íåäîïóñòèìà
      if ((decode_tacts[bt - 1]  == 0  || decode_tacts[bt - 1]  == 3) && (decode_tacts[bt] == 0 || decode_tacts[bt] == 4)) decode_tacts[bt] = 2;
       //Õ1 1X - íåäîïóñòèìà
      if ((decode_tacts[bt - 1]  == 1  || decode_tacts[bt - 1]  == 4) && (decode_tacts[bt] == 1 || decode_tacts[bt] == 3)) decode_tacts[bt] = 2;
    }
  }

//Восстановление одиночных тактов  
  for(int bt = 1; bt < (bitsize - 1); bt++)
  {
    if (decode_tacts[bt] == 2)
    {
       //Õ0 __ 0Õ
       //Õ0 11 0Õ     
      if ((decode_tacts[bt - 1]  == 0  || decode_tacts[bt - 1]  == 3) && (decode_tacts[bt + 1] == 0 || decode_tacts[bt + 1] == 4)) {
        decode_tacts[bt] = 1;
        restore_sign ^= 2;
      }
       //Õ0 __ 1Õ
       //Õ0 10 1Õ     
      if ((decode_tacts[bt - 1]  == 0  || decode_tacts[bt - 1]  == 3) && (decode_tacts[bt + 1] == 1 || decode_tacts[bt + 1] == 3)){
        decode_tacts[bt] = 3;
        restore_sign ^= 2;
        }
       //Õ1 __ 0Õ
       //Õ1 01 0Õ     
      if ((decode_tacts[bt - 1]  == 1  || decode_tacts[bt - 1]  == 4) && (decode_tacts[bt + 1] == 0 || decode_tacts[bt + 1] == 4)){
        decode_tacts[bt] = 4;
        restore_sign ^= 2;
      }
       //Õ1 __ 1Õ
       //Õ1 00 1Õ     
      if ((decode_tacts[bt - 1]  == 1  || decode_tacts[bt - 1]  == 4) && (decode_tacts[bt + 1] == 1 || decode_tacts[bt + 1] == 3)){
        decode_tacts[bt] = 0;
        restore_sign ^= 2;
      }
    }
  }

  //восстановление потерянных полутактов
   cdp = cdptr;        
   for(int bt = 1 ; bt < (bitsize - 1); bt++)
   {
     if (decode_tacts[bt] == 2)
     {
     //Õ0 _0
     //Õ0 10
     if ((*cdp & 0x0f) < 0x05 && (decode_tacts[bt - 1] == 0 || decode_tacts[bt - 1] == 3)){
        decode_tacts[bt] = 3; 
        restore_sign ^= 1;
     }
     //Õ1 _1
     //Õ1 01
     if ((*cdp & 0x0f) > 0x04 && (decode_tacts[bt - 1] == 1 || decode_tacts[bt - 1] == 4)){
        decode_tacts[bt] = 4; 
        restore_sign ^= 1;
     }
      //0_ 0X
      //01 0X
     if ((*cdp & 0xF0) < 0x50 && (decode_tacts[bt + 1] == 0 || decode_tacts[bt + 1] == 4)){
        decode_tacts[bt] = 4; 
        restore_sign ^= 1;
     }
       //1_ 1X
      //10 1X
     if ((*cdp & 0xF0) > 0x40 && (decode_tacts[bt + 1] == 1 || decode_tacts[bt + 1] == 3)){
        decode_tacts[bt] = 3; 
        restore_sign ^= 1;
     }
    }
   *cdp++;
  }

  //Снова проверяем допустима ли тактовая последовательность, а то что мы там восстановили - неизвестно :)

  for(int bt = 1; bt < bitsize; bt++)
  {
    {
       //Õ0 0X - íåäîïóñòèìà
      if ((decode_tacts[bt - 1]  == 0  || decode_tacts[bt - 1]  == 3) && (decode_tacts[bt] == 0 || decode_tacts[bt] == 4)) decode_tacts[bt] = 2;
       //Õ1 1X - íåäîïóñòèìà
      if ((decode_tacts[bt - 1]  == 1  || decode_tacts[bt - 1]  == 4) && (decode_tacts[bt] == 1 || decode_tacts[bt] == 3)) decode_tacts[bt] = 2;
    }
   *cdp++;
  }
 

//Определение версии пакета по преамбуле
//Если преамбула распознана на несколько тактов уверенно, то можно судить о версии пакета

  //001100110011 -> v2
  if (/*decode_tacts[0] == 0 && decode_tacts[1] == 1  &&*/ decode_tacts[2] == 0 && decode_tacts[3] == 1 && decode_tacts[4] == 0 && decode_tacts[5] == 1 && ver == 3){
   ver = 2;
   restore_sign ^=4;
  }
  //110011001100 -> v2
  if (/*decode_tacts[0] == 1 && decode_tacts[1] == 0  &&*/ decode_tacts[2] == 1 && decode_tacts[3] == 0 && decode_tacts[4] == 1 && decode_tacts[5] == 0 && ver == 3){
   ver = 2;
   restore_sign ^=4;
  }
  //010101010101 -> v3
  if (/*decode_tacts[0] == 4 && decode_tacts[1] == 4  &&*/ decode_tacts[2] == 4 && decode_tacts[3] == 4 && decode_tacts[4] == 4 && decode_tacts[5] == 4 && ver == 2){
   ver = 3;
   restore_sign ^=4;
  }
  //101010101010 -> v3
  if (/*decode_tacts[0] == 3 && decode_tacts[1] == 3  &&*/ decode_tacts[2] == 3 && decode_tacts[3] == 3 && decode_tacts[4] == 3 && decode_tacts[5] == 3 && ver == 2){
   ver = 3;
   restore_sign ^=4;
  }

// Печать расшифорвки
 if (receiver_dump)
 {
   byte* cdp = cdptr;        
   Serial.print("AFTER  ");
   for(int bt = 0 ; bt < bitsize; bt++)
   {
     if (decode_tacts[bt] == 1) Serial.print("II"); 
     if (decode_tacts[bt] == 0) Serial.print("OO"); 
     if (decode_tacts[bt] == 2) Serial.print("__"); 
     if (decode_tacts[bt] == 3) Serial.print("IO"); 
     if (decode_tacts[bt] == 4) Serial.print("OI"); 
     Serial.print(" "); 
     *cdp++;
  }
   Serial.println();
 }


  return;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
//Извлекает из тактовой последовательности битовую
//Параметры: cdptr - указатель на записанные данные
// btt - смещение в тактах. Смещение на такт при анализе может помочь восстановить пакет, у которого разрушено начало
// Функция возвращает качество или "годность" расшифровки - количество уверенно узнаных тактов.
// Сравнивая годность с btt=0 и btt=1 выбираем лучшую
////////////////////////////////////////////////////////////////////////////////////////////////////
int Oregon_NR::get_data(int btt, byte p_ver, byte* cdptr){ //btt - смещение на такт при анализе может поммочь восстановить пакет, у которого разрушено начало
  
  byte* cdp = cdptr;
//×èñòèì ìàññèâ
  for(int bt=0 ; bt<READ_BITS; bt++){
    
    *cdp = 128;
    cdp++;
  }
  cdp = cdptr;

  *cdp = (128+2); //Первый бит всегда единица!!!
  cdp++;
  int packet_validity = 0;

  if (p_ver == 2){
    for(int bt=1 ; bt < READ_BITS; bt++){
    
      if(decode_tacts[bt*2-btt]==0) *cdp-=1; // Если 00 - то возможно здесь 0
      //if(decode_tacts[bt*2-btt]==0) *cdp-=1; // начальный вес 1 отбрасывает датчики со сбитой синхронизацией. 2 - ловит с ними
      if(decode_tacts[bt*2-btt]==1) *cdp+=1; // Если 11 - то предположительно здесь 1
      if(decode_tacts[bt*2-2-btt]==1 && decode_tacts[bt*2-1-btt]==4) *cdp-=1; //Если до этого было уверенно 1101 - то это добавляет уверенности, что здесь 0
      if(decode_tacts[bt*2-2-btt]==0 && decode_tacts[bt*2-1-btt]==3) *cdp+=1;
      if(decode_tacts[bt*2-2-btt]==0 && decode_tacts[bt*2-1-btt]==1) *cdp-=1;
      if(decode_tacts[bt*2-2-btt]==1 && decode_tacts[bt*2-1-btt]==0) *cdp+=1;
    
      if(decode_tacts[bt*2+2-btt]==1 && decode_tacts[bt*2+1-btt]==3) *cdp-=1; // Если после этого идёт 1011 - то это добавляет уверенности, что здесь 0
      if(decode_tacts[bt*2+2-btt]==0 && decode_tacts[bt*2+1-btt]==4) *cdp+=1;
      if(decode_tacts[bt*2+2-btt]==0 && decode_tacts[bt*2+1-btt]==1) *cdp-=1;
      if(decode_tacts[bt*2+2-btt]==1 && decode_tacts[bt*2+1-btt]==0) *cdp+=1;
    
        //Подсчитываем кол-во достоверных бит в пакете
      if (*cdp>(129))  packet_validity += *cdp - 128;
      if (*cdp<(127)) packet_validity += 128 - *cdp;
      cdp++;
    }
  return packet_validity; //возвращаем кол-во достоверных байтов
  }

  if (p_ver == 3){
    for(int bt = 1 ; bt < READ_BITS; bt++){
    
      if (*(cdp - 1) > 128) // åñëè äî ýòîãî áûëà 1
      {
        if (decode_tacts[bt - btt] == 0 || decode_tacts[bt - btt] == 1) *cdp -= 2; // Åñëè 00 èëè 11 - òî çäåñü 0
        if (decode_tacts[bt - btt] == 3 || decode_tacts[bt - btt] == 4) *cdp += 2; // Åñëè 01 èëè 10 - òî çäåñü 1
      }
      if (*(cdp - 1) < 128) // åñëè äî ýòîãî áûëà 0
      {
        if (decode_tacts[bt - btt] == 0 || decode_tacts[bt - btt] == 1) *cdp += 2; // Åñëè 00 èëè 11 - òî çäåñü 1
        if (decode_tacts[bt - btt] == 3 || decode_tacts[bt - btt] == 4) *cdp -= 2; // Åñëè 01 èëè 10 - òî çäåñü 0
      }

     // åñëè äî ýòîãî íåïîíÿòíî ÷òî, òî ñêîðåå âñåãî íå óäàëîñü âîññòàíîâèòü öåëûõ äâà òàêòà. ñìîòðèì íà íåñêîëüêî áèò íàçàä

     //Âîññòàíîâëåíèå î÷åð¸äíîñòè ïðè ïðîïóñêå äóõ áèòîâ ê ðÿäó
      if (*(cdp - 1) == 128 && *(cdp - 2) == 128) 
      {
        //0 __ __ 0 - íå ìåíÿåòñÿ
        if ((decode_tacts[bt - btt] == 0 || decode_tacts[bt - btt] == 3) && (decode_tacts[bt - btt - 2] == 0 || decode_tacts[bt - btt - 2] == 4))
	{
	 if (*(cdp - 2) > 128) *cdp += 1;
	 if (*(cdp - 2) < 128) *cdp -= 1;
	}
	else 
	{
        //1 __ __ 0 èëè 0 __ __ 1- ìåíÿåòñÿ
	 if (*(cdp - 2) > 128) *cdp -= 1;
	 if (*(cdp - 2) < 128) *cdp += 1;
	}
      }

     //Âîññòàíîâëåíèå î÷åð¸äíîñòè ïðè ïðîïóñêå òð¸õ áèòîâ ê ðÿäó
      if (*(cdp - 1) == 128 && *(cdp - 2) == 128  && *(cdp - 3) == 128) 
      {
        //0 __ __ 0 - íå ìåíÿåòñÿ
        if ((decode_tacts[bt - btt] == 0 || decode_tacts[bt - btt] == 3) && (decode_tacts[bt - btt - 3] == 0 || decode_tacts[bt - btt - 3] == 4))
	{
	 if (*(cdp - 2) > 128) *cdp += 1;
	 if (*(cdp - 2) < 128) *cdp -= 1;
	}
	else 
	{
        //1 __ __ 0 èëè 0 __ __ 1- ìåíÿåòñÿ
	 if (*(cdp - 2) > 128) *cdp -= 1;
	 if (*(cdp - 2) < 128) *cdp += 1;
	}
      }

     //Âîññòàíîâëåíèå î÷åð¸äíîñòè ïðè ïðîïóñêå ÷åòûð¸õ áèòîâ ê ðÿäó
      if (*(cdp - 1) == 128 && *(cdp - 2) == 128  && *(cdp - 3) == 128  && *(cdp - 4) == 128) 
      {
        //0 __ __ 0 - íå ìåíÿåòñÿ
        if ((decode_tacts[bt - btt] == 0 || decode_tacts[bt - btt] == 3) && (decode_tacts[bt - btt - 4] == 0 || decode_tacts[bt - btt - 4] == 4))
	{
	 if (*(cdp - 2) > 128) *cdp += 1;
	 if (*(cdp - 2) < 128) *cdp -= 1;
	}
	else 
	{
        //1 __ __ 0 èëè 0 __ __ 1- ìåíÿåòñÿ
	 if (*(cdp - 2) > 128) *cdp -= 1;
	 if (*(cdp - 2) < 128) *cdp += 1;
	}
      }
      //Áîëüøå íèáëà ñ ïîìîùüþ êîíòðîëüíîé ñóììû âñ¸ ðàâíî íå âîññòàíîâèòü.


        //Ïîäñ÷èòûâàåì êîë-âî äîñòîâåðíûõ áèò â ïàêåòå
      if (*cdp>(129))  packet_validity += *cdp - 128;
      if (*cdp<(127)) packet_validity += 128 - *cdp;
      cdp++;
    }
  return packet_validity; //âîçâðàùàåì êîë-âî äîñòîâåðíûõ áàéòîâ
  }


}
////////////////////////////////////////////////////////////////////////////////////////////////////
//Прослушивание канала с частотой дискретизации 16384Гц 
//т.е. запись раз в 61мкс
//Каждому такт соответствует байт. Такт делится на два полутакта. В каждом полутакте проводится 8 измерений.
//При наличии сигнала при измерении добавляется 1 к соответствующему ниблу. Отсутствие сигнала - 0x00. Наличие сигнала в такте 0х88.
//cdptr - указатель на область памяти, куда записываются сигнал
//dtl - указатель на количество считанных тактов
////////////////////////////////////////////////////////////////////////////////////////////////////
int Oregon_NR::collect(byte* cdptr){
  
  bool cdp_prev_null;
  byte* cdp = cdptr;
  byte nulls_found = 0;
  byte bt2 = 0;
  //////////////////////////////////////////////////////
  //Çàïèñü íà÷èíàåì ñ ýòîãî ìîìåíòà (êîíåö ïîñëåäíåãî èìïóëüñÿ çàöåïêè + 1/16 òàêòà)
  if (ver == 2) 
  {
    pre_marker += 946; //äâà òàêòà
    *cdp = 0x87;   //Ïåðâûå äâà òàêòà èçâåñòíû - 11. Ìû æå ïîéìàëè èìïóëüñ!
    cdp++;
  }

  if (ver == 3)
  {
    pre_marker += 1434; //òðè òàêòà
    *cdp = 0x07;       //Ïåðâûå ÷åòûðå òàêòà èçâåñòíû  - 0101. Ìû æå ïîéìàëè èìïóëüñ!
    *(cdp + 1) = 0x07;  
    cdp += 2;
  }

  //////////////////////////////////////////////////////
  //Íà÷èíàåì ÷èòàòü äàííûå â ïàìÿòü
  // Ìàêñèìàëüíàÿ äëèíà ïîàñûëêà äëÿ v3 - 104ÁÈÒÀ, THN132 - 76 áÈÒ + êàê ìèíèìóì 3 áèòà 111, êîòîðûå ìû óæå íàøëè
  byte bt;

  for (bt = 0 ; bt < READ_BITS2; bt++) {        
    *cdp = 0;
    for (byte ckl = 0; ckl < 8; ckl++) {            // ×èòàåì 8 ðàç çà ïîëóòàêò
      pre_marker += 61;
      while (micros() < pre_marker);
      if (digitalRead(RECEIVER_PIN)) *cdp += 0x10;  // Èçìåðåíèÿ çàïèûâàåì â ñòàðøèé ïîëóáàéò
    }
    for (byte ckl = 0; ckl < 8; ckl++) {
      pre_marker += 61;
      while (micros() < pre_marker);
      if (digitalRead(RECEIVER_PIN)) *cdp += 1;     // Â ñëåäóþùèé ïîëóòàêò èçìåðåíèÿ çàïèûâàåì â ìëàäøèé  ïîëóáàéò. Ýòî ýêîíîìèò ïàìÿòü.

    }
    bt2++;
    // Èäåàëüíûé ïåðèîä 976.5625
    //  Êàæäûå 7 òàêòîâ äîáàâëÿòü 4ìêñ äëÿ âûðàâíèâàíèÿ ïåðèîäà ñ 976ìêñ äî 976.5714ìêñ + 0.009%
    if (bt2 == 7) 
    { 
      pre_marker += (4 + timing_correction) ;
      bt2 = 0;
    }

    /////////////////////////////////////////////
    //Åñòü âðåìÿ äî ïðèõîäà ñëåäóþùåãî ïîëóòàêòà
    //Ìîæíî ïðîâåðèòü, à íå çàêîí÷èëàñü ëè ïîñûëêà
    //Åñëè â êàíàëå ïîñëåäíåå âðåìÿ ïóñòîòà èëè ñëàáûå ïîìåõè, òî ýòî äîáàâëÿåò óâåðåííîñòè, ÷òî íàáëþäàåì îêîí÷àíèå ïàêåòà
    yield();
    if ((*cdp & 0xf0) < 0x30 && (*cdp & 0x0f) < 0x05)  nulls_found++;
    else nulls_found = 0;
    cdp++;

    /////////////////////////////////////////////
    //Åñëè áîëåå empty_space ïóñòûõ ïîëåé â çàïèñè, òî
    //ýòî âåðîÿòíåå âñåãî êîíåö ïîñûëêè. Äàëüøå ÷èòàòü íåò ñìûñëà
    //empty_space - ÷èñëî ýìïèðè÷åñêîå, çàâèñèò îò òèïà ïðè¸ìíèêà è óðîâíÿ ñèãíàëà
    //Åñëè óìåíüøèòü, âîçìîæíî ñïóòàòü ñ ïîâðåæäåíèåì ïàêåòà
    //Åñëè óâåëè÷èòü, òî ìîæíî íå ïðåêðàòèòü ÷òåíèå è íà÷àòü çàïèñûâàòü ïîìåõè
    if (nulls_found > empty_space ) return bt;










    /////////////////////////////////////////////
    //Æä¸ì ïðèõîäà âðåìåíè ñëåäóþùåãî ïîëóòàêòà

    while (micros() < pre_marker);                    
  }
  return bt;
} 
////////////////////////////////////////////////////////////////////////////////////////////////////
//Определение смещения пакетов друг относительно друга
//В качестве параметров передаются указатели на массивы данных
// Возваращаяется смещение
// >0 - второй пакет начался раньше, <0 - Первый пакет начался раньше
////////////////////////////////////////////////////////////////////////////////////////////////////
int Oregon_NR::correlate_data(byte* ser1, byte* ser2){
  
  byte best_correl = 0;
  int best_shift = 0;
  byte best_correl_back = 0;
  int best_shift_back = 0;
  byte shift_score[READ_BITS];
  byte* s1;
  byte* s2;
  byte* s2t = ser2;
  //ñìåùàåì ïåðâûé ïàêåò îòíîñèòåëüíî âòîðîãî
  for (byte sht = 0; sht < READ_BITS; sht++){
    s1 = ser1;
    s2 = s2t;
    shift_score[sht] = 0;
    for(byte sp = 0; sp < READ_BITS - sht; sp++){
      if ((*s1 > (128+1) && *s2 > (128+1))||(*s1 < (128-1) && *s2 < (128-1)) ) shift_score[sht]++;
      s2++;
      s1++;
    }
    yield();
    s2t++;
  }
  for (int i = 0; i < READ_BITS; i++){

    if (shift_score[i] > best_correl){
      best_correl = shift_score[i];
      best_shift = i;
    }
  }
 
//Òåïåðü íàîáîðîò -âòðîé ïàêåò îòíîñèòåëüíî ïåðâîãî
  
  byte* s1t = ser1;
  for (byte sht = 0; sht < READ_BITS; sht++){
    s2 = ser2;
    s1 = s1t;
    shift_score[sht] = 0;
    for (byte sp = 0; sp < READ_BITS-sht; sp++){
      
      if ((*s1 > (128+1) && *s2 > (128+1))||(*s1 < (128-1) && *s2 < (128-1)) ) shift_score[sht]++;
      s2++;
      s1++;
    }
    yield();
    s1t++;
  }
// Èùåì íàèëó÷øåå ñîâïàäåíèå äëÿ îáîèõ âàðèàíòîâ 
    
  for (int i = 0; i < READ_BITS; i++){
    
    if (shift_score[i] > best_correl_back){
      best_correl_back = shift_score[i];
      best_shift_back = i;
    }
  }
  //È âîçâðàùàåì ñàìîå ëó÷øåå èç äâóõ
  if (best_correl_back > best_correl) return -best_shift_back;
  else return best_shift;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
//Сборка из двух пакетов
//В качестве параметров передаются указатели на массивы данных
// Причём первым должен идти результирующий пакет, т.е. тот который имеет более длинную преамбулу.
//shift - смещение втрого пакета относительного первого
////////////////////////////////////////////////////////////////////////////////////////////////////
void Oregon_NR::assemble_data(byte* s1, byte* s2, int shift){
  
  if (shift >= 0) {
    for (int g = 0; g < shift; g++) s2++;
    for (int i = 0; i < READ_BITS - shift; i++){
      if(*s1 < (128 + 2) && *s1 > (128 - 2) && (*s2 > (128 + 1) || *s2 < (128 - 1))) {
      *s1 = *s2;
      }
      s1++;
      s2++;
    }
  }
  else {
    for (int g = 0; g < -shift; g++) s1++;
    for (int i = 0; i < READ_BITS + shift; i++){
      if(*s1 < (128 + 2) && *s1 > (128 - 2) && (*s2 > (128 + 1) || *s2 < (128 - 1))) {
      *s1 = *s2;
      }
      s1++;
      s2++;
    }
  }
}
////////////////////////////////////////////////////////////////////////////////////////////////////
//Возвращает позицию синхронибла в посылке. 0xFF - нет синхронибла
// 
//code - указатель на расшифрованную битовую последовательность
//result - указатель на кодовую посылку
////////////////////////////////////////////////////////////////////////////////////////////////////
int Oregon_NR::get_synchro_pos(byte* code){
  
  bool syn_found = false;
  byte* cp = code;
  int i = 0;
  for (i = 0; i < READ_BITS - 8; i++){
    if (!consist_synchro && (*cp < 128 && *(cp + 1) > 128 && *(cp + 2) < 128 && *(cp + 3) > 128)) {
      syn_found = true;
      break;
    }

    if (consist_synchro && (*cp < 127 && *(cp + 1) > 129 && *(cp + 2) < 127 && *(cp + 3) > 129)) {
      syn_found = true;
      break;
    }
    
    cp++;
  }
  if (!syn_found) return 0xFF;
  //Ïîñëåäîâàòåëüíîñòü íàøëè, íî íàäî óáåäèòüñÿ, ÷òî ïåðåä ýòèì èä¸ò ïåðàìáóëà, ò. å. óâåðåííûå åäèíèöû 
  // Â ïðåàìáóëå íå ìîæåò áûòü íóëåé! - ýòî ãëàâíîå
  //Ïðåàìáóëó íàäî ïðîñìàòðèâàòü íà 16-3 = 13 áèòîâ íàçàä. Íó õîòÿ áû íà 10!!!
  
  for (byte g = i; i - g < 10 && g > 0; g --){
    cp --;
    if (*cp < 127) return 0xFF; // Ïåðåä ñèíõðîíèáëîì â ïðåàìáóëå íå ìîæåò áûòü óâåðåííîãî íóëÿ. Íåò òóò ñèíõðîíèáëà!
  }
  return (byte) i;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
//Создаёт кодовую посылку
//code - указатель на расшифрованную битовую последовательность
//result - указатель на кодовую посылку
//valid - указатель на карту достоверности кодовой посылки
////////////////////////////////////////////////////////////////////////////////////////////////////
int Oregon_NR::get_info_data(byte* code, byte* result, byte* valid){

  byte* rd = result;
  byte* vd = valid;
  //×èñòèì ìàññèâû
  for (int l = 0; l < PACKET_LENGTH; l++){
    *vd = 0;
    *rd = 0;
    vd++;
    rd++;
  }
  rd = result;
  vd = valid;
  
  int csm;
  for (csm = 0; csm < 30; csm++){
    if ( !consist_synchro && (*code < 128 && *(code + 1) > 128 && *(code + 2) < 128 && *(code + 3) > 128)) break; //Íàéäåíà ïîñëåäîâàòåëüíîñòü 0101 
    if (  consist_synchro && (*code < 127 && *(code + 1) > 129 && *(code + 2) < 127 && *(code + 3) > 129)) break; 
    code++;
  }
  // Ñèíõðîíèáë â ïåðâûõ 20 áèòàõ íå íàéäåí, òàêîé ïàêåò íå ðàñøèôðóåøü âî âòîðîé âåðñèè ïðîòîêîëà!
  if (ver == 2 & csm > 22) return 0; 
  // ÄËÿ òðåòüåé âåðñèè ïðîòîêîëà öèôðà èíàÿ
  if (ver == 3 & csm > 30) return 0; 
  //Ïåðåõîäèì íà íà÷àëî ñ÷èòûâàíèÿ
  code += 4;
  int ii = 0;
  for (int i = 0; i < READ_BITS - csm; i++) 
  {
    // ×òîáû íå âûéòè çà ïðåäåëû
    if (i >= PACKET_LENGTH * 4) break;
    byte multipl;
    switch (ii){
      case 0: {multipl = 0x01; break;}
      case 1: {multipl = 0x02; break;}
      case 2: {multipl = 0x04; break;}
      case 3: {multipl = 0x08; break;}
    }

    if (*code > 128 ) {
      *rd += multipl;
      *vd += multipl;
    }
    if (*code < 128 ) *vd += multipl;
    code ++;
    ii ++;
    if (ii == 4) {
      ii = 0;
      vd++;
      rd++;
    }
  }
  return 1;
}
//
////////////////////////////////////////////////////////////////////////////////////////////////////
//Функции расшифровки данных с датчиков
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
//Возвращает значение температуры
//oregon_data - указатель на кодовую посылку
////////////////////////////////////////////////////////////////////////////////////////////////////
float Oregon_NR::get_temperature(byte* oregon_data){
  if (((sens_type & 0x0FFF) == RTGN318 || sens_type == THGR810 || sens_type == THGN132 || sens_type == THN132) && crc_c)
  {
    float tmprt;
    oregon_data += 8;
    //исправляем возможные ошибки:
    for (int g=0;g < 4; g++)  if (*(oregon_data + g) > 9) *(oregon_data + g) = *(oregon_data + g) - 8;
    tmprt += *(oregon_data) * 0.1;
    tmprt += *(oregon_data + 1);
    tmprt += *(oregon_data + 2) * 10;
    return (*(oregon_data + 3)) ? -tmprt : tmprt;
  }
//  else return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
//Возвращает тип сенсора.
//oregon_data - указатель на кодовую посылку
////////////////////////////////////////////////////////////////////////////////////////////////////
word Oregon_NR::get_sensor(byte* oregon_data){

    return (word)(*(oregon_data))*0x1000 + (*(oregon_data+1))*0x0100 + (*(oregon_data+2))*0x10 + *(oregon_data+3);
}

  // Указываем в каких ниблах находится префикс транспондера AAA - первые три нибла
word Oregon_NR::get_transponder(byte* oregon_data){

    return (word)(*(oregon_data))*0x100 + (*(oregon_data+1))*0x010 + (*(oregon_data+2));
}
  // указываем в каком нибле находится номер(цифра) транспондера идущая после префикса. Отключил, не пригодилось.
// byte Oregon_NR::get_tr_number(byte* oregon_data){

//     return (word)(*(oregon_data+3));
// }
////////////////////////////////////////////////////////////////////////////////////////////////////
//Возвращает значение канала
//oregon_data - указатель на кодовую посылку
////////////////////////////////////////////////////////////////////////////////////////////////////
byte Oregon_NR::get_channel(byte* oregon_data){
  if (crc_c)
  {
    byte channel = 0;  
//    word sens_type = get_sensor(oregon_data);
    if (sens_type == THGN132 || sens_type == THN132 )
    {
      switch (*(oregon_data + 4))  
      {  
      case 1:  
        channel = 1;  
        break;  
      case 2:  
        channel = 2;  
        break;  
      case 4:  
        channel = 3;  
        break;  
      }  
    }
    if ((sens_type & 0x0FFF) == RTGN318 || sens_type == THGR810 )
        channel = *(oregon_data + 4);  
    return channel;  
  }
  else return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
byte Oregon_NR::get_battery(byte* oregon_data){
  if (((sens_type & 0x0FFF) == RTGN318 || sens_type == THGR810 || sens_type == THGN132 || sens_type == THN132 ||  sens_type == WGR800 ||  sens_type == UVN800 ||  sens_type == PCR800) && crc_c)
  return (*(oregon_data+7) & 0x4) ? 0 : 1;  
  else  return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
//Возвращает значение влажности
//oregon_data - указатель на кодовую посылку
////////////////////////////////////////////////////////////////////////////////////////////////////
float Oregon_NR::get_humidity(byte* oregon_data){

  if (((sens_type & 0x0FFF) == RTGN318 || sens_type == THGR810 || sens_type == THGN132)  && crc_c ){
    byte tmprt = 0;
    oregon_data += 12;
  //исправляем возможные ошибки:
    for (int g=0; g < 2; g++)  if (*(oregon_data + g) > 9) *(oregon_data + g) = *(oregon_data + g) - 8;
    tmprt = *(oregon_data);
    tmprt += *(oregon_data + 1) * 10;
    return (float)tmprt;
  }
  else return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
//Возвращает id датчика
//oregon_data - указатель на кодовую посылку
////////////////////////////////////////////////////////////////////////////////////////////////////
byte Oregon_NR::get_id(byte* oregon_data){
  if (((sens_type & 0x0FFF) == RTGN318 || sens_type == THGR810 || sens_type == THGN132 || sens_type == THN132 ||  sens_type == WGR800 ||  sens_type == UVN800 || sens_type == PCR800) && crc_c)
  {
    byte tmprt;
    oregon_data += 5;
    tmprt = *(oregon_data) * 0x10;
    tmprt += *(oregon_data + 1);
    return tmprt;
  }
  else return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//Возвращает среднее значение ветра в м/c
//oregon_data - указатель на кодовую посылку
////////////////////////////////////////////////////////////////////////////////////////////////////
float Oregon_NR::get_avg_windspeed(byte* oregon_data)
{
  if (sens_type == WGR800 && crc_c){
    float tmprt;
    tmprt = *(oregon_data + 15) * 0x10;
    tmprt += *(oregon_data + 14);
    return tmprt / 10;
  }
  else return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//Возвращает значение максимального порыва ветра в м/c
//oregon_data - указатель на кодовую посылку
////////////////////////////////////////////////////////////////////////////////////////////////////
float Oregon_NR::get_max_windspeed(byte* oregon_data)
{
  if (sens_type == WGR800 && crc_c){
    float tmprt;
    tmprt = *(oregon_data + 12) * 0x10;
    tmprt += *(oregon_data + 11);
    return tmprt / 10;
  }
  else return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//Âîçâðàùàåò íàïðàâëåíèå âåòðà â êâàäðàíòàõ
//oregon_data - óêàçàòåëü íà êîäîâóþ ïîñûëêó
////////////////////////////////////////////////////////////////////////////////////////////////////
byte Oregon_NR::get_winddirection(byte* oregon_data)
{
  if (sens_type == WGR800 && crc_c){
    byte tmprt;
    return *(oregon_data + 8) & 0x0F;
    //Êâàäðàíòû  0-N, 1-NNE, 2-NE, 3-ENE, 4-E, 5-ESE, 6-SE, 7-SSE, 8-S, 9-SSW, A-SW, B-WSW, C-W, D-WNW, E-NW,F-NNW
  }
  else return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//Âîçâðàùàåò UV-èíäåêñ
//oregon_data - óêàçàòåëü íà êîäîâóþ ïîñûëêó
////////////////////////////////////////////////////////////////////////////////////////////////////
byte Oregon_NR::get_UV(byte* oregon_data)
{
  if (sens_type == UVN800 && crc_c){
    byte tmprt;
    tmprt = *(oregon_data + 9) * 0x10;
    tmprt += *(oregon_data + 8);
    return tmprt;
  }
  else return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//Âîçâðàùàåò îñâåù¸ííîñòü â óñëîâíûõ åäèíèöàõ
//oregon_data - óêàçàòåëü íà êîäîâóþ ïîñûëêó
////////////////////////////////////////////////////////////////////////////////////////////////////
byte Oregon_NR::get_light(byte* oregon_data)
{
  if (sens_type == UVN800 && crc_c){
    byte tmprt;
    tmprt = *(oregon_data + 11) * 0x10;
    tmprt += *(oregon_data + 10);
    tmprt -= 0x6F;
    return tmprt;
  }
  else return 0;
}

// #################### mr.Tech edition 20.02.2020 ###############################################
// RAIN GAUGE#####################################################################################
float Oregon_NR::get_rain_rate(byte* oregon_data)
{
   if (sens_type == PCR800 && crc_c){
    float tmprt;
    tmprt = (oregon_data[11]);
    tmprt += (oregon_data[10]) * 0.1;
    tmprt += (oregon_data[9]) * 0.01;
    tmprt += (oregon_data[8]) * 0.001;
    return tmprt * 2.54; //if your PCR800 is for metric readings
    //return tmprt_; // empirical
  }
  else return 0;
  //Serial.print(oregon_data);
}

float Oregon_NR::get_total_rain(byte* oregon_data)
{
   if (sens_type == PCR800 && crc_c){
    float tmprt;
    // tmprt = *(oregon_data + 17) * 0x2710;//0x3E8; // * 0x64; // digits before coma (ratio digit). Set quality .00000
    // tmprt += *(oregon_data + 16) * 0x3E8; //0x64 - 1000; .0 digit after coma 
    // tmprt += *(oregon_data + 15) * 0x64; //0x0A - 100; .00
    // tmprt += *(oregon_data + 14) * 0x0A; //0x0A - 10; .000
    // tmprt += *(oregon_data + 13); // .0000
    // // tmprt += (oregon_data[12]) * 0.0001;
    // return tmprt / 10000 * 2.54; //if your PCR800 is designed for metric measurement
    tmprt = *(oregon_data + 17);//0x3E8; // * 0x64; // digits before coma (ratio digit). Set quality .00000
    tmprt += *(oregon_data + 16); // * 0.1; //0x64 - 1000; .0 digit after coma 
    tmprt += *(oregon_data + 15) * 0.1;// 0.01; //0x0A - 100; .00
    tmprt += *(oregon_data + 14) * 0.01;//0.001; //0x0A - 10; .000
    tmprt += *(oregon_data + 13) * 0.001;//0.0001; // .0000
    tmprt += (oregon_data[12]) * 0.0001;//0.00001;
    return tmprt * 2.54; //if your PCR800 is for metric readings
    //return tmprt_; // empirical
  }
  else return 0;
}
// ###############################################################################################
word Oregon_NR::get_latitude(byte* oregon_data){
    if (transponder == AIR && crc_c){
  // if (sens_type == AIR1 && crc_c){
    long temp;
    temp = (oregon_data[5]) * 0x1000 ;
    temp += (oregon_data[6]) * 0x100;
    temp += (oregon_data[7]) * 0x10;
    temp += (oregon_data[8]) * 0x1;
    temp >> 4;
    temp |= (oregon_data[4]) * 0x10000;
    return temp;
  }
  else return 0;
}

word Oregon_NR::get_longitude(byte* oregon_data){
    if (transponder == AIR && crc_c){
  // if (sens_type == AIR1 && crc_c){
    long temp;
    temp = (oregon_data[10]) * 0x1000 ;
    temp += (oregon_data[11]) * 0x100;
    temp += (oregon_data[12]) * 0x10;
    temp += (oregon_data[13]) * 0x1;
    temp >> 4;
    temp |= (oregon_data[10]) * 0x10000;
    return temp;
  }
  else return 0;
}

word Oregon_NR::get_altitude(byte* oregon_data){
  if (transponder == AIR && crc_c){
  // if (sens_type == AIR1 && crc_c){
    int temp;
    // temp = (oregon_data[14]) + 0xAA;
    temp = (oregon_data[14]) * 0x10;
    temp += (oregon_data[15]);
    temp *= 10;
    return temp;
  }
  else return 0;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//Проверка CRC
//oregon_data - указатель на кодовую посылку
////////////////////////////////////////////////////////////////////////////////////////////////////
bool Oregon_NR::check_CRC(byte* oregon_data, word sens_type){

  byte* pp=oregon_data;
  byte crc, resived_crc, truecrc, resived_truecrc, i;
  crc=0;
  byte CCIT_POLY = 0x07;
  
  if (sens_type==THGN132){
 //CHKSUM 1...15 
 //CRC 1...5,8...15 STARTSUM = 3Ch, POLY = 07h
    truecrc = 0x3C;
    for(int x=0; x < 15; x++){
      crc += *pp;
      if ( x != 5 && x != 6){
        truecrc ^= *pp;
        for(i = 0; i<4; i++) 
          if(truecrc & 0x80) truecrc = (truecrc << 1) ^ CCIT_POLY;
          else truecrc <<= 1;
      }
      pp++;  
    }
    for(i = 0; i<4; i++) 
      if(truecrc & 0x80) truecrc = (truecrc << 1) ^ CCIT_POLY;
      else truecrc <<= 1;

    resived_crc = (*(oregon_data+15))+(*(oregon_data+16))*0x10;
    resived_truecrc = (*(oregon_data+17))+(*(oregon_data+18))*0x10;
    received_CRC = truecrc;
    return (resived_crc == crc && resived_truecrc == truecrc)? 1 : 0;
  }

  if ((sens_type & 0x0FFF) == RTGN318){
 //CHKSUM 1...15 
 //CRC 1...5,8...15 STARTSUM = 00h, POLY = 07h
    truecrc = 0x0;
    for(int x=0; x<15; x++){
      crc += *pp;
      if ( x != 5 && x != 6){
        truecrc ^= *pp;
        for(i = 0; i<4; i++) 
          if(truecrc & 0x80) truecrc = (truecrc << 1) ^ CCIT_POLY;
          else truecrc <<= 1;
      }
      pp++;  
    }
    for(i = 0; i<4; i++) 
      if(truecrc & 0x80) truecrc = (truecrc << 1) ^ CCIT_POLY;
      else truecrc <<= 1;

    resived_crc = (*(oregon_data+15))+(*(oregon_data+16))*0x10;
    resived_truecrc = (*(oregon_data+17))+(*(oregon_data+18))*0x10;
    received_CRC = truecrc;
    return (resived_crc == crc && resived_truecrc == truecrc)? 1 : 0;
  }

  if (sens_type == THGR810){
 //CHKSUM 1...15 
 //CRC 00h,1...15 STARTSUM = 00h, POLY = 07h
    truecrc = 0x0;
    for(i = 0; i<4; i++) 
      if(truecrc & 0x80) truecrc = (truecrc << 1) ^ CCIT_POLY;
      else truecrc <<= 1;
    for(int x=0; x<15; x++){
      crc += *pp;
      truecrc ^= *pp;
      for(i = 0; i<4; i++) 
        if(truecrc & 0x80) truecrc = (truecrc << 1) ^ CCIT_POLY;
        else truecrc <<= 1;
      pp++;  
    }
    for(i = 0; i<4; i++) 
      if(truecrc & 0x80) truecrc = (truecrc << 1) ^ CCIT_POLY;
      else truecrc <<= 1;

    resived_crc = (*(oregon_data+15))+(*(oregon_data+16))*0x10;
    resived_truecrc = (*(oregon_data+17))+(*(oregon_data+18))*0x10;
    received_CRC = truecrc;
    return (resived_crc == crc && resived_truecrc == truecrc)? 1 : 0;
  }

  if (sens_type == UVN800 ){
 //CHKSUM 1...13 
 //CRC 00h,1...13 STARTSUM = 00h, POLY = 07h
    truecrc = 0x0;
    for(i = 0; i<4; i++) 
      if(truecrc & 0x80) truecrc = (truecrc << 1) ^ CCIT_POLY;
      else truecrc <<= 1;
    for(int x=0; x<13; x++){
      crc += *pp;
      truecrc ^= *pp;
      for(i = 0; i<4; i++) 
        if(truecrc & 0x80) truecrc = (truecrc << 1) ^ CCIT_POLY;
        else truecrc <<= 1;
      pp++;  
    }
    for(i = 0; i<4; i++) 
      if(truecrc & 0x80) truecrc = (truecrc << 1) ^ CCIT_POLY;
      else truecrc <<= 1;

    resived_crc = (*(oregon_data+13))+(*(oregon_data+14))*0x10;
    resived_truecrc = (*(oregon_data+15))+(*(oregon_data+16))*0x10;
    received_CRC = truecrc;
    return (resived_crc == crc && resived_truecrc == truecrc)? 1 : 0;
  }


  if (sens_type == WGR800){
   //CHKSUM 1...17
    for(int x=0; x < 17; x++){ 
      crc += *pp;
      pp++;  
    }

    resived_crc = (*(oregon_data+17))+(*(oregon_data+18))*0x10;
    return (resived_crc == crc)? 1 : 0;
  }
  //### mr.Tech 20.02.2020 ######################
  if (sens_type == PCR800){
   //CHKSUM 1...18
    for(int x=0; x < 18; x++){ 
      crc += *pp;
      pp++;  
    }

    resived_crc = (*(oregon_data+18))+(*(oregon_data+19))*0x10;
    return (resived_crc == crc)? 1 : 0;
  }

  //transponder AIR1 ######################################################################################
  if (transponder == AIR){
  // if (sens_type == AIR1){
  // if (sens_type == AIR1 || sens_type == AIR2 || sens_type == AIR3){
 //CHKSUM 1...18
    for(int x=0; x <17; x++){                                       // считаем ниблы полезной информации от нулевого до указанного
      crc += *pp;                                                   // crc - вычисленная контрольная сумма
      pp++;  
    }
    resived_crc = (*(oregon_data+17))+(*(oregon_data+18))*0x10;     // Принятая контрольная сумма
    return (resived_crc == crc)? 1 : 0;                             // сравниваем вычисленную с принятой
  }                                                                 
  // !!! Не забудь. Если необходимо будет расширить поле полезной информации, 
  // то необходимо сдвинуть указатели на контрольную сумму(*(oregon_data+17))+(*(oregon_data+18)) в соответствующую позицию,
  // и не забыть увеличить кол-во тактов(циклов) в трансмиттере. Т.к. в библиотеке трансмиттера (void Oregon_TM::sendData()) сейчас 11 тактов
  //##################################################

  if (sens_type==THN132){

 //CHKSUM 1...12 
 //CRC 1...5,8...12 STARTSUM = D6h, POLY = 07h
    truecrc = 0xD6;
    for(int x=0; x<12; x++){
      crc += *pp;
      if ( x != 5 && x != 6){
        truecrc ^= *pp;
        for(i = 0; i<4; i++) 
          if(truecrc & 0x80) truecrc = (truecrc << 1) ^ CCIT_POLY;
          else truecrc <<= 1;
      }
      pp++;  
    }
    for(i = 0; i<4; i++) 
      if(truecrc & 0x80) truecrc = (truecrc << 1) ^ CCIT_POLY;
      else truecrc <<= 1;

    resived_crc = (*(oregon_data+12))+(*(oregon_data+13))*0x10;
    resived_truecrc = (*(oregon_data+14))+(*(oregon_data+15))*0x10;
    received_CRC = truecrc;
    return (resived_crc == crc && resived_truecrc == truecrc)? 1 : 0;
  }

#ifdef ADD_SENS_SUPPORT == 1

  if ((sens_type & 0xFF00) == GAS || (sens_type & 0xFF00) == THP || (sens_type & 0xFF00) == FIRE || (sens_type & 0xFF00) == CURRENT  || (sens_type & 0xFF00) == CAPRAIN){
    truecrc = 0x00;
    for(int x=0; x<15; x++){
      crc += *pp;
      truecrc ^= *pp;
      for(i = 0; i<4; i++) 
        if(truecrc & 0x80) truecrc = (truecrc << 1) ^ CCIT_POLY;
        else truecrc <<= 1;
      pp++;  
    }
    for(i = 0; i<4; i++) 
      if(truecrc & 0x80) truecrc = (truecrc << 1) ^ CCIT_POLY;
      else truecrc <<= 1;

    resived_crc = (*(oregon_data+15))+(*(oregon_data+16))*0x10;
    resived_truecrc = (*(oregon_data+17))+(*(oregon_data+18))*0x10;
    received_CRC = truecrc;
    return (resived_crc == crc && resived_truecrc == truecrc)? 1 : 0;

  }
#endif

  return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
//Востановление данных по типу датчика
////////////////////////////////////////////////////////////////////////////////////////////////////
void Oregon_NR::restore_data(byte* oregon_data, word sens_type){
  
  byte* pp=oregon_data;
  if (sens_type==THGN132){
    pp+=8;
    for(int x=0; x<6; x++){
      if(*pp>9 && x!=3) *pp-=8;
      pp++;  
    }
  }
  if (sens_type==THN132){
    pp+=8;
    for(int x=0; x<3; x++){
      if(*pp>9) *pp-=8;
      pp++;  
    }
  }
  return;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
void Oregon_NR::led_light(bool led_on) {
  if (LED != 0xFF) { 
    if (PULL_UP && led_on) digitalWrite(LED, LOW);
    if (PULL_UP && !led_on) digitalWrite(LED, HIGH);
    if (!PULL_UP && led_on) digitalWrite(LED, HIGH);
    if (!PULL_UP && !led_on) digitalWrite(LED, LOW);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
//
#if ADD_SENS_SUPPORT == 1
////////////////////////////////////////////////////////////////////////////////////////////////////
//Ôóíêöèè ðàñøèôðîâêè äàííûõ äàò÷èêîâ GAS
//gas_data - óêàçàòåëü íà êîäîâóþ ïîñûëêó
////////////////////////////////////////////////////////////////////////////////////////////////////
byte Oregon_NR::get_gas_channel(byte* gas_data){

  return gas_data[2];
}
////////////////////////////////////////////////////////////////////////////////////////////////////
float Oregon_NR::get_gas_temperature_out(byte* gas_data){

  int temperat = gas_data[9] * 0x0100 + gas_data[8] * 0x0010 + gas_data[7];
  return ((float)(-1000 + temperat)) / 10;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
float Oregon_NR::get_gas_temperature_in(byte* gas_data){

  int temperat = gas_data[12] * 0x0100 + gas_data[11] * 0x0010 + gas_data[10];
  return ((float)(-1000 + temperat)) / 10;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
byte Oregon_NR::get_gas_hmdty(byte* gas_data){

  return gas_data[14] * 0x10 + gas_data[13];
}
////////////////////////////////////////////////////////////////////////////////////////////////////
byte Oregon_NR::get_gas_CO(byte* gas_data){

  return gas_data[6] * 0x10 + gas_data[5];
}
////////////////////////////////////////////////////////////////////////////////////////////////////
byte Oregon_NR::get_gas_CH(byte* gas_data){

  return gas_data[4] * 0x10 + gas_data[3];
}

////////////////////////////////////////////////////////////////////////////////////////////////////
byte Oregon_NR::get_fire_ip22(byte* fire_data){

  return fire_data[4] * 0x10 + fire_data[5];
}
////////////////////////////////////////////////////////////////////////////////////////////////////
byte Oregon_NR::get_fire_ip72(byte* fire_data){

  return fire_data[6] * 0x10 + fire_data[7];
}
////////////////////////////////////////////////////////////////////////////////////////////////////
byte Oregon_NR::get_fire_lockalarm(byte* fire_data){

  return fire_data[8] * 0x10 + fire_data[9];
}


////////////////////////////////////////////////////////////////////////////////////////////////////
float Oregon_NR::get_current(byte* current_data){

  return ((float)(current_data[4] * 0x1000 + current_data[5] * 0x0100  + current_data[6] * 0x0010  + current_data[7])) / 1000;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
float Oregon_NR::get_voltage(byte* voltage_data){

  return ((float)(voltage_data[8] * 0x1000 + voltage_data[9] * 0x0100  + voltage_data[10] * 0x0010  + voltage_data[11])) / 10;
}

word Oregon_NR::get_pump_count(byte* voltage_data){

  return (voltage_data[12] * 0x0100 + voltage_data[13] * 0x0010  + voltage_data[14]);
}
unsigned long Oregon_NR::get_dropcounter(byte* packetdata){

  return (packetdata[10] * 0x10000  + packetdata[11] * 0x1000  + packetdata[12] * 0x100 + packetdata[13] * 0x10 + packetdata[14]);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
int Oregon_NR::get_capacitance(byte* packetdata){

  return (packetdata[6] * 0x1000  + packetdata[7] * 0x100  + packetdata[8] * 0x10 + packetdata[9]);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
float Oregon_NR::get_thp_temperature(byte* current_data){

  return (float)(current_data[3] * 0x0100 + current_data[4] * 0x0010  + current_data[5] * 0x0001) / 10 - 100;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
float Oregon_NR::get_thp_pressure(byte* current_data){

  return (float)(current_data[9] * 0x0100 + current_data[10] * 0x0010  + current_data[11] * 0x0001) / 10 + 500;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
float Oregon_NR::get_thp_voltage(byte* current_data){

  return (float)(current_data[12] * 0x0100 + current_data[13] * 0x0010  + current_data[14] * 0x0001) * 0.01;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
float Oregon_NR::get_thp_humidity(byte* current_data){

  return (float)(current_data[6] * 0x0100 + current_data[7] * 0x0010  + current_data[8] * 0x0001) / 10;
}
#endif



