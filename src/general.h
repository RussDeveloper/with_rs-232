
#include "SPIFFS.h"
#include "perif.h"
#include <ArduinoJson.h>

//Преамбулы посылок для передачи компьютеру

#define change_sensors		  0x30		//оповещение об изменении сигналов на сенсорах
#define sensors_condition	  0x31		//Передача сигналов с сенсоров
#define RFID_event			    0x40		//передача данных считанной карты
#define empty_event		    	0x20		//Пустая посылка для проверки наличия подключения

//Преамбулы для приема команд от компьютера

#define  RED        5
#define  GREEN      6

extern EventGroupHandle_t main_event_group;
extern com_drive rs_232;
extern JsonDocument card_list;

//char user_nums[100][10];

String users_num[100];

typedef struct
{
  String name;
  String mask;
  String tool_light;
  String shelf_light;
}tool_form;


void general_task( void * pvParameters)
{
  int list_size,i,j,k;
  char *p;
  String list_str, list_card;

  //Запрос данных из файла
  File file = SPIFFS.open("/list_users.txt");
    if(!file){
         Serial.println("Ошибка отрытия файла со списком карт");
        while(1){}
              }
    /*
    p = (char *)malloc(1000);
    if(p==0)
    {
      Serial.println("general task: В куче нет памяти");
      while(1){}
    }          
      */
       list_size = file.size();

      
      for(i=0;i<list_size;i++)            //Перегрузка в оперативу
      {
        //p[i] = file.read();
        //p++;
        list_str += file.read();
      }

      JsonDocument doc;
      DeserializationError err = deserializeJson(doc, list_str);

      bool ser_flag = true;
      if(err == DeserializationError::Ok)
      {ser_flag = false;}
      else if(err == DeserializationError::EmptyInput)
      {Serial.println("Ошибка десериализации list_str: EmptyInput");}
      else if(err == DeserializationError::IncompleteInput)
      {Serial.println("Ошибка десериализации list_str: IncompleteInput");}
      else if(err == DeserializationError::InvalidInput)
      {Serial.println("Ошибка десериализации list_str: InvalidInput");}
      else if(err == DeserializationError::NoMemory)
      {Serial.println("Ошибка десериализации list_str: NoMemory");}
      else if(err == DeserializationError::TooDeep)
      {Serial.println("Ошибка десериализации list_str: TooDeep");}
      //while(ser_flag){}
      //Данные из файла получены...

      j = doc.size();

      //Данные из файла переведены в массив строк
  for(;;)
  {
    if(xEventGroupGetBits(main_event_group)&wifi_flag)    //Если WiFi подключен
    {

    }

    if(xEventGroupGetBits(main_event_group)&sensors_flag)    //Если сенсоры изменились
    {
      digitalWrite(RED,HIGH);
      delay(50);
      digitalWrite(RED,LOW);
      xEventGroupClearBits(main_event_group, sensors_flag);
    }
/**/
    i=0;
    if(RFID_dat)                            //Если очередь создана
      i=uxQueueMessagesWaiting(RFID_dat);
    if(i)                                   //Если приложили карту к RFID считывателю
    {
      JsonArray tmp = doc.as<JsonArray>();  //Получить список сохраненных номеров
      i=0;

      char *p;
      /**/
      xQueueReceive(RFID_dat, p, 100);
      String str_rfid(p);
      bool rfid_flag = false;

      for(JsonVariant v : tmp)              //Проверить, находится ли новый номер в списке на допуск
      {
        users_num[i] = v.as<String>();
        if(users_num[i] == str_rfid)
          {rfid_flag = true; break;}
        i++;
      }
     
      if(rfid_flag)                 //Карта пользователя есть в списке
      {
        JsonDocument resp;
        resp["card"] = str_rfid.c_str();
        resp["acces"] = "true";
        String resp_str;                  //Строка с JSON на отправку на планшет
        deserializeJson(resp, resp_str);
      }
    

    }
    String msg;
    if(rs_232.rx_flag)                  //Сообщение от планшета
    {
      //serializeJson(rs_232.reseive, msg);
      //msg.begin(rs_232.reseive["command"]);
    }
    delay(1);
  }
}





















