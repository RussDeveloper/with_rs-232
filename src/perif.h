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
#define mosi1       11

#define nss2				38
#define leds2				2
#define l_sh2				47
#define sck2				36
#define miso2				37
#define mosi2       35
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
String card_val;

xQueueHandle RFID_dat;
//Указатели на валидные массивы сенсоров
char *sensors_ptr1, *sensors_ptr2;
//Приемные буферы
char    spi1_buff[2][110],      //Буферы временного хранения для организации сравнения показаний от предыдущего считывания
        spi2_buff[2][110],      //Буферы временного хранения для организации сравнения показаний от предыдущего считывания
        sens_buff[50],          //Данные сенсоров после фиксации изменения
        sens_delta[50],         //Битовая маска разницы
        
        load_buff[5*num_nodules],
        card_buff[10],
        *valid_sensors_buff[2]
                        ;
uint32_t sens_change;            //знак разницы - приход - 1, уход - 0

unsigned int flags;             //Переменная глобальных флагов

byte decode(char);
char * bin_dec(uint32_t);

void spi_task( void * parameter) 
{
    SPIClass  spi_1(HSPI), 
            spi_2(FSPI);
    char i,j,           //Переменные циклов
    buff[2][110],       
    num_bloks[2],       //Переменные автоматического определения количества блоков
    flag,               //Флаг, показывающий в какую часть двумерного массива в данный момент произошла загрузка
    k,t, fl;                //Временные переменные

  static char *ptr1, *ptr2, num_cells = num_nodules*5;

  pinMode(sck1, OUTPUT);
  for(i=0;i<6;i++)
  {
    digitalWrite(sck1, 1);
    delay(1);
    digitalWrite(sck1, 0);
    delay(1);
  }

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

    load_buff[10] = 0x50;
    load_buff[0] = 0x0e;
    load_buff[1] = 0x02;
    load_buff[2] = 0x83;
    load_buff[3] = 0x13;

  delay(100);
  
  if(0)
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
    }else{
      num_bloks[0] = 5;
      num_bloks[1] = 5;
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
    int tt;

  load_buff[0]++;

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

      for(i=0;i<num_cells;i++)
      {
        if(i>(num_cells-((num_bloks[0]*2)-1)))    //Начиная с 15-го байта передаем байты для нагрузки
        {
            k = load_buff[num_cells-i-1];
            t = load_buff[num_cells-i+9];
        }
        else{k=1; t=1;}
        
        ptr1[i] = spi_1.transfer(k);
        ptr2[i] = spi_2.transfer(t);   
      }

      digitalWrite(nss1, 1);            //Выгрузка значений на пины
      digitalWrite(nss2, 1);
      delay(1);
      digitalWrite(nss1, 0);
      digitalWrite(nss2, 0);

     sens_change = 0;

     for(i=0;i<num_cells;i++)
      {
        k=0;
        if(spi1_buff[0][i]!=spi1_buff[1][i])
        {
          flags|= sensors_flag;

          if(spi1_buff[0][i]>spi1_buff[1][i])           //Если данные в нулевом банке больше
          {
            k = spi1_buff[0][i]&(~spi1_buff[1][i]);     //Получение битов разницы
             if(flag==0)                                //В зависимости от того, какой буфер использовался
              sens_change|= 1<<(i/5);                   //разница является либо установкой, либо убытием           
          }else
          {
            k = spi1_buff[1][i]&(~spi1_buff[0][i]);     //Получение битов разницы

            if(flag)                                    //В зависимости от того, какой буфер использовался
              sens_change|= 1<<(i/5);                   //разница является либо установкой, либо убытием
          }
        }
          sens_delta[i] |= k;
          k=0;
        if(spi2_buff[0][i]!=spi2_buff[1][i])
        {
          flags|= sensors_flag;

          if(spi2_buff[0][i]>spi2_buff[1][i])
          {
            k = spi2_buff[0][i]&(~spi2_buff[1][i]);   //Получение битов разницы
            if(flag==0)                                  //В зависимости от того, какой буфер использовался
            sens_change|= 1<<((i/5)+5);               //разница является либо установкой, либо убытием
          }else
          {
            k = spi2_buff[1][i]&(~spi2_buff[0][i]);   //Получение битов разницы

            if(flag)                                  //В зависимости от того, какой буфер использовался
            sens_change|= 1<<((i/5)+5);               //разница является либо установкой, либо убытием
          }
        }
          sens_delta[i+num_cells] |= k;
          /*
          if(flag)            //В зависимости от того, какой буфер использовался
          {                   //разница является либо установкой, либо убытием
            sens_change = 1;
          }
          */

        /*
       ptr1--;
       ptr2--; */ 
      }

      //digitalWrite(RED, 0);
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
        //digitalWrite(RED, 1);
        xEventGroupSetBits(main_event_group, sensors_flag);
        
        //Serial.println("Сигнал с датчиков ");   
      }

      flags&=~sensors_flag;
    vTaskDelay(100);    
  

}
}

/*
SERIAL_8E1    :020056E26462
SERIAL_8O1    :020056E26462
SERIAL_8N2    :020056E26462
*/
void card_task( void * parameter)
{
  static char buff[20], i,j,k;
  char mass[20];
  uint32_t a,b,c;

  
  Serial1.setRxBufferSize(20);
  Serial1.begin(9600, SERIAL_8N1, 18, 17);

  RFID_dat = xQueueCreate(20,5);
  
  for(;;)
  {
    if(Serial1.available())
    {
      i=0;
      vTaskDelay(100); 

      //Serial.println("Данные карты: ");
      card_val.clear();
      a=0;
      while(Serial1.available())        //Прием
      {
        j = Serial1.read();
        //card_val+=j;    
        /**/    
        if(i>0)
          buff[i-1] = decode(j);        //Декодирование
        i++;
      }

      for(j=0;j<i-1;j+=2)             //Получение номера в формате HEX
      {
        mass[j/2]=((buff[j]&0x0f)<<4)|(buff[j+1]&0x0f);
      }
       
      a = mass[2]<<16;
      a |= mass[3]<<8;
      a |= mass[4];
      char *p = bin_dec(a);
      card_val+=p;
        //xQueueSendToBack(RFID_dat, &buff[0], 100);
        xEventGroupSetBits(main_event_group, RFID_flag);
    }
      vTaskDelay(5);  
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
      delay(5);
      Serial2.readBytes(serial_buff, j); 
      str = serial_buff;    
      if(deserializeJson(rs_232.reseive, str) == DeserializationError::Ok)
        {rs_232.rx_flag = true;
          Serial.println("DeserializationError::Ok");
          Serial.println(rs_232.reseive.as<String>());          
          xEventGroupSetBits(main_event_group, rs232_flag);
        }
        j=0;
    }
    
    if(rs_232.tx_flag)
    {
      serializeJson(rs_232.transmit, str);
      Serial2.print(str);
      rs_232.tx_flag = false;
      Serial.print("tx_flag");
    }
    delay(10);
  }
}

byte decode(char val)
{
  if((val>=0x30)&&(val<=0x39))
    return val&0x0f;

  if(val=='a')
    return 0xa ;
  if(val=='b')
    return 0xb ;
  if(val=='c')
    return 0xc ;
  if(val=='d')
    return 0xd ;
  if(val=='e')
    return 0xe ;
  if(val=='f')
    return 0xf;

  if(val=='A')
    return 0xa ;
  if(val=='B')
    return 0xb ;
  if(val=='C')
    return 0xc ;
  if(val=='D')
    return 0xd ;
  if(val=='E')
    return 0xe ;
  if(val=='F')
    return 0xf;
  return 0;
}

char * bin_dec(uint32_t val)
{
  const uint32_t bb[] = {1000000000, 100000000, 10000000, 
                      1000000, 100000, 10000, 1000, 100,
                       10, 1};
  static char mass[11];
  char* p=NULL;
  int i,j=0;
  bool flag = true;
  mass[10]=0;

   for(i=0;i<10;i++)
  {
    if(val/bb[i])
    {
      flag=true;
      mass[i] = val/bb[i];
      val%=bb[i];
      j++;
    }else{mass[i]=0;}

    if(flag)
      {
        mass[i] |= 0x30;
      }
  }
    if(j)
      p = &mass[0];
    return p;  
}

















