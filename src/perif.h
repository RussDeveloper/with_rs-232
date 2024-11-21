#include <SPI.h>
#include "globals.h"
#include "Arduino.h"
#define old_pins

#ifdef old_pins
#define nss1				10
#define leds1				1
#define l_sh1				48
#define sck1				12
#define miso1				13
#define mosi1               11

#define nss2				38
#define leds2				2
#define l_sh2				47
#define sck2				36
#define miso2				37
#define mosi2               35
#endif

#ifndef old_pins
#define nss1				10
#define leds1				1
#define l_sh1				11
#define sck1				48
#define miso1				13
#define mosi1       11

#define nss2				38
#define leds2				2
#define l_sh2				47
#define sck2				36
#define miso2				37
#define mosi2       35
#endif

#define num_nodules          5


HardwareSerial SerialPort(2);
extern EventGroupHandle_t main_event_group;
typedef struct 
{
 JsonDocument reseive; 
 JsonDocument transmit;
 bool rx_flag;
 bool tx_flag;
} com_drive;

com_drive rs_232;

xQueueHandle RFID_dat;
//Указатели на валидные массивы сенсоров
char *sensors_ptr1, *sensors_ptr2;
//Приемные буферы
char    spi1_buff[2][110],      //Буферы временного хранения для организации сравнения показаний от предыдущего считывания
        spi2_buff[2][110],      //Буферы временного хранения для организации сравнения показаний от предыдущего считывания
        sens_buff[50],          //Данные сенсоров после фиксации изменения
        sens_delta[50],         //Битовая маска разницы
        sens_change,            //знак разницы - приход - 1, уход - 0
        load_buff[5*num_nodules],
        card_buff[10],
        *valid_sensors_buff[2]
                        ;

unsigned int flags;             //Переменная глобальных флагов

void spi_task( void * parameter) 
{
    SPIClass  spi_1(HSPI), 
            spi_2(FSPI);
    char i,j,           //Переменные циклов
    buff[2][110],       
    num_bloks[2],       //Переменные автоматического определения количества блоков
    flag,               //Флаг, показывающий в какую часть двумерного массива в данный момент произошла загрузка
    k,t;                //Временные переменные

  static char *ptr1, *ptr2, num_cells = num_nodules*5;

  // Настройка пинов
  spi_1.begin(sck1, miso1, mosi1); //SCLK, MISO, MOSI, SS
  spi_2.begin(sck2, miso2, mosi2); //SCLK, MISO, MOSI, SS

  spi_1.setFrequency(500000);
  spi_1.setDataMode(SPI_MODE0);
  spi_2.setDataMode(SPI_MODE0);
/*
  spi_1.setDataMode(SPI_MODE2);
  spi_2.setDataMode(SPI_MODE2);
*/
  spi_1.transfer(0);
  spi_2.transfer(0);

  pinMode(nss2, OUTPUT);  
  pinMode(nss1, OUTPUT);  

  pinMode(leds1 ,OUTPUT);
  pinMode(leds2 ,OUTPUT);
  pinMode(l_sh1 ,OUTPUT);
  pinMode(l_sh2 ,OUTPUT);

  digitalWrite(l_sh1, HIGH);
  digitalWrite(l_sh2, HIGH);

  Serial.println("SPI task запущена");

  sensors_ptr1 = &spi1_buff[0][0];
  sensors_ptr2 = &spi2_buff[0][0];
/**/
  num_bloks[1]=0;

  digitalWrite(l_sh1, LOW);			//Записать значения в регистры
	digitalWrite(l_sh2, LOW);			//Записать значения в регистры
	    vTaskDelay(1);
	digitalWrite(l_sh1,HIGH);
	digitalWrite(l_sh2,HIGH);

    load_buff[11] = 0x0f;
    load_buff[0] = 0xf0;

  delay(100);
  
  if(1)
  {  
  do{
    for(i=0;i<5;i++)
    {
      j=spi_1.transfer(0x00);
      Serial.println(j, HEX);
    }
      if(j==0x99)
        num_bloks[0]++;
    }while(j==0x99);

    Serial.println("Блоки первого порта:");
    delay(1);
    Serial.println(num_bloks[0], HEX);
  
  do{
      for(i=0;i<5;i++)
      {
        j=spi_2.transfer(0x00);
        //Serial.println(j, HEX);
      }
        if(j==0x99)
          num_bloks[1]++;
      }while(j==0x99);

      Serial.println("Блоки второго порта:");
      delay(1);
      Serial.println(num_bloks[1], HEX);
  }
 
  for(;;) 
  {
    digitalWrite(leds1, HIGH);			//Включить диоды сенсоров
    digitalWrite(leds2, HIGH);			//Включить диоды сенсоров
	    vTaskDelay(1);
	  digitalWrite(l_sh1, LOW);			//Записать значения в регистры
	  digitalWrite(l_sh2, LOW);			//Записать значения в регистры
	    vTaskDelay(1);
	  digitalWrite(l_sh1,HIGH);
	  digitalWrite(l_sh2,HIGH);
	    vTaskDelay(1);
	  digitalWrite(leds1, LOW);
	  digitalWrite(leds2, LOW);


    if(ptr1 == &spi1_buff[0][0])
      {
        ptr1 = &spi1_buff[1][0];
        ptr2 = &spi2_buff[1][0];
        flag=1;
      }else
      {
        ptr1 = &spi1_buff[0][0];
        ptr2 = &spi2_buff[0][0];
        flag=0;
      }
/*
    if(ptr2 == &spi2_buff[0][0])
      {
        ptr2 = &spi2_buff[1][0];
      }else
      {
        ptr2 = &spi2_buff[0][0];
      }
*/
      for(i=0;i<num_cells;i++)
      {
        if(i>14)
        {
            k = load_buff[num_cells-i-1];
            t = load_buff[num_cells-i+9];
            //Serial.print(t, HEX);
        }
        *ptr1 = spi_1.transfer(k);		//Выгружаем и сравниваем значения из регистров
        ptr1++;
        *ptr2 = spi_2.transfer(t);		//Выгружаем и сравниваем значения из регистров
        ptr2++;
      }

      digitalWrite(nss1, 1);
      digitalWrite(nss2, 1);
      delay(1);
      digitalWrite(nss1, 0);
      digitalWrite(nss2, 0);
     
     for(i=0;i<num_cells;i++)
      {
        if(spi1_buff[0][i]!=spi1_buff[1][i])
        {
          flags|= sensors_flag;
          if(spi1_buff[0][i]>spi1_buff[1][i])
          {
            k = spi1_buff[0][i]&(~spi1_buff[1][i]);   //Получение битов разницы
          }else
          {
            k = spi1_buff[1][i]&(~spi1_buff[0][i]);   //Получение битов разницы
          }

          sens_delta[i] = k;

          if(flag)            //В зависимости от того, какой буфер использовался
          {                   //разница является либо установкой, либо убытием
            sens_change = 0;
          }else
          {
            sens_change = 1;
          }
          }
        /**/
        if(spi2_buff[0][i]!=spi2_buff[1][i])
          {flags|= sensors_flag;}
        
       ptr1--;
       ptr2--;  
      }

      if(flags&sensors_flag)
      {
        j=0;
        for(i=0;i<25;i++)
        {
          sens_buff[j] = ptr1[i];
          j++;
          //Serial.print(ptr1[i], HEX);
        }
        //Serial.println("");
        for(i=0;i<25;i++)
        {
          sens_buff[j] = ptr2[i];
          j++;
        }

        xEventGroupSetBits(main_event_group, sensors_flag);
        
        Serial.println("Сигнал с датчиков ");   
      }

      flags&=~sensors_flag;
    vTaskDelay(100);    
  }

}

void card_task( void * parameter)
{
  static char buff[20], i,j;
  String card_val;
  
  Serial1.setRxBufferSize(20);
  Serial1.begin(9600, SERIAL_8N1, 18, 17);

  RFID_dat = xQueueCreate(20,5);
  
  for(;;)
  {
    if(Serial1.available())
    {
      i=0;
      vTaskDelay(100); 
      xEventGroupSetBits(main_event_group, RFID_flag);
      Serial.println("Данные карты: ");
      while(Serial1.available())
      {
        j = Serial1.read();//
       //Serial.print(atoi((const char*)&j));   :33008B904E64
       Serial.print((const char)j);
        buff[i] = j;
        card_val+=j;
        i++;
      }
        xQueueSendToBack(RFID_dat, &buff[0], 100);
        Serial.println("");
        card_val = "";      
      }
      vTaskDelay(1);  
  }
}

void rs232_task(void *plParametrs)
{
  char serial_buff[50];
  unsigned int i,j;
  static int tme=0;
  String str;
    
  Serial2.setRxBufferSize(50);
  Serial2.begin(115200, SERIAL_8N1, 16, 15);
  Serial2.setTimeout(2);

  for(;;)
  { 
    j = Serial2.available();
    if(j)
    {
      Serial2.readBytes(serial_buff, j); 
      str = serial_buff;    
      if(deserializeJson(rs_232.reseive, str) == DeserializationError::Ok)
        rs_232.rx_flag = true;
      
      j=0;
    }
    
    if(rs_232.tx_flag)
    {
      serializeJson(rs_232.transmit, str);
      Serial2.print(str);
      rs_232.tx_flag = false;
    }
    delay(10);
  }
}






















