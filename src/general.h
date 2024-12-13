
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
extern char load_buff[5*num_nodules];
extern String card_val;

//char user_nums[100][10];

String users_num[100];
uint32_t lock_flag, lock_timer;
bool lock;

typedef struct
{
  String id;
  String mask;
  String tool_light;
  String shelf_light;
}tool_form;

//tool_form tool[50];

JsonDocument tool;

int add_tool(JsonDocument);
int del_tool(JsonDocument);
int add_user(String);
int del_user(String);
JsonDocument  read_card_list();
void card_list_compare(JsonDocument);

void del(int val)
{while(val--);}

void general_task( void * pvParameters)
{
  int list_size,i,j,k;
  char *p;
  String list_str, list_card;
  JsonDocument t_doc;
  extern JsonDocument card_list;
  //JsonArray u_arr = card_list.as<JsonArray>();

  //Запрос данных из файлов
  File t_list = SPIFFS.open("/tool_list.txt");
  
  // Номер в списке - "0005694052"

 /* 
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
      */
      //Данные из файла переведены в массив строк
  for(;;)
  {
    if(xEventGroupGetBits(main_event_group)&wifi_flag)    //Если WiFi подключен
    {
    }

    if(xEventGroupGetBits(main_event_group)&RFID_flag)    //Если RFID_flag считан
    {
      serializeJsonPretty(card_list, list_card);
      if((list_card.indexOf(card_val)!=-1))//&&(lock_timer==0))             //фиксация совпадения карты
      {
        //card_list.clear();
        lock_timer = 60*100;                              //При совпадении карты ящик открыт минуту
        Serial.println("Доступ открыт");
      }
        else{
          Serial.println("Нет доступа");
          Serial.println(list_card);
          Serial.println(card_val);
        }
      xEventGroupClearBits(main_event_group, RFID_flag);
      
      Serial.println("card_list.as<int>()");
      Serial.println(card_list.as<String>());
    }

    if(xEventGroupGetBits(main_event_group)&sensors_flag)    //Если сенсоры изменились
    {
      digitalWrite(RED,HIGH);
      delay(50);
      digitalWrite(RED,LOW);
      xEventGroupClearBits(main_event_group, sensors_flag);
      for(i=0;i<50;i++)
      {
        /*
          Serial.print(sens_delta[i], HEX);
          sens_delta[i]=0;
          delay(1);
          */
        if(sens_delta[i])
        {
          if(sens_change)
          {
          Serial.print("Номер модуля: ");
          Serial.println(i/5);
          
          Serial.print("прибытие/убытие: ");
          Serial.println(sens_change, HEX);
          

          Serial.print("Маска датчиков: ");
          for(j=i;j<(i+4);j++)
            Serial.print(sens_delta[j], HEX);
          Serial.println("");
          sens_delta[i]=0;
          }
        }
      }  
      //Serial.println("");   
    }
/**/
    i=0;
 /*
    if(RFID_dat)                            //Если очередь создана
      i=uxQueueMessagesWaiting(RFID_dat);
         
    if(i)                                   //Если приложили карту к RFID считывателю
    {
      JsonArray tmp = doc.as<JsonArray>();  //Получить список сохраненных номеров
      i=0;

      char *p;
      /*
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
    */
    String msg;
    if(rs_232.rx_flag)                  //Сообщение от планшета
    {
      //deserializeJson(rs_232.reseive, msg);
      //msg.begin(rs_232.reseive["command"]);
      //if(rs_232.reseive["command"].is<int>())
      //{
        const char *ptr = rs_232.reseive["command"];
        if(ptr){
          msg += ptr;
          Serial.println("general_task");
          Serial.println(msg);
          //delete ptr;
            if(msg =="start")  
            {
              
            }
            if(msg =="end")  
            {
                rs_232.transmit["command"] = "ok";
                rs_232.transmit["id"] = "1234";
                rs_232.tx_flag = true;
            }
            if(msg =="ok")  
            {
              
            } 
            if(msg =="light")  
            {
              
            }

            msg.clear();
        }
        else{ Serial.println("Json опять мозги ебет");}
        //}
      //if( ==)
      rs_232.rx_flag = 0;
    }
    delay(10);
    while(lock_timer--){}
  }
}

int add_tool(JsonDocument t)
{
  String str;
  bool seach = false;

    File root = SPIFFS.open("/tools");
    File file = root.openNextFile();

    str = t["id"].as<String>(); 
    str+= ".txt";

    while(file)                     //Поверка на наличие уже созданных файлов
    {
      //str=file.name();

      if(str==file.name())
      {
        seach=true;
        Serial.println("Инструмент с таким номеров уже существует...");
        file.close();
        root.close();
        return 1;
      }
      //str.clear();
      file.close();
      file = root.openNextFile();
    }
      str.clear();
      file.close();
      root.close(); 

      str+= "/tools/";
      str+=t["id"].as<String>();
      str+= ".txt";
      file = SPIFFS.open(str.c_str(), "w", true);

      if(!file)
      {
        Serial.println("Не удалось создать файл");
        file.close();
        return 2;
      }

      serializeJson(t, file);
      file.close();
      return 0;
}

int add_tool1(JsonDocument t)
{
  File t_list1  = SPIFFS.open("/tool_list.txt");

      if(!t_list1){
         Serial.println("В функции \"add_tool\" не открылся файл \"tool_list.txt \"");
         t_list1.close();
         return 1;
              }

    DeserializationError err = deserializeJson(tool, t_list1);

    if((err!= DeserializationError::Ok)&&(err!= DeserializationError::EmptyInput))
    {
      Serial.println("В функции \"add_tool\" ошибка десериализации");
      t_list1.close();
      return 2;
    }

  if(tool[t["id"]].is<int>())
  {
     Serial.println("В функции \"add_tool\" инструмент с таким id уже существует");
     t_list1.close();
      return 3;
  }

  tool["id"] = t["id"];
  tool["id"]["mask"] = t["mask"];
  tool["id"]["tool_light"] = t["tool_light"];
  tool["id"]["shelf_light"] = t["shelf_light"];

  t_list1.close();
  SPIFFS.remove("/tool_list.txt");
  
  t_list1  = SPIFFS.open("/tool_list.txt", "w", true);

  serializeJsonPretty(tool, t_list1);
  t_list1.close();
}

int del_tool(String _id)
{
      File root = SPIFFS.open("/tools");
    File file = root.openNextFile();
    bool err = false;
    String str("/tools/");
    _id+=".txt";

    while(file)                     //Поверка на наличие уже созданных файлов
    {
      if(_id==file.name())
      {
        str+=_id;
        if(!SPIFFS.remove(str))
        {
          Serial.println("Удаление инструмента не получилось");
          err = true;
        }
        file.close();
        root.close();
        return 0;
      }
      //str.clear();
      file.close();
      file = root.openNextFile();
    }
    if(err)
      return 2;
    Serial.println("Такого инструмента нет в базе");
    return 1;
}

int add_user(String _id)
{
    String str;
  bool seach = false;

    File root = SPIFFS.open("/users");
    File file = root.openNextFile();

    _id+= ".txt";

    while(file)                     //Поверка на наличие уже созданных файлов
    {
      if(_id==file.name())
      {
        seach=true;
        Serial.println("Пользователь с таким ID уже существует...");
        file.close();
        root.close();
        return 1;
      }
      //str.clear();
      file.close();
      file = root.openNextFile();
    }
      str.clear();
      file.close();
      root.close(); 

      str+= "/users/";
      str+=_id;
      file = SPIFFS.open(str.c_str(), "w", true);

      if(!file)
      {
        Serial.println("Не удалось создать файл");
        file.close();
        return 2;
      }
      file.close();
      return 0;
}

int del_user(String _id)
{
      File root = SPIFFS.open("/users");
    File file = root.openNextFile();
    bool err = false;
    String str("/users/");
    _id+=".txt";

    while(file)                     //Поверка на наличие уже созданных файлов
    {
      if(_id==file.name())
      {
        str+=_id;
        if(!SPIFFS.remove(str))
        {
          Serial.println("Удаление Пользователя с таким id  не получилось");
          err = true;
        }
        file.close();
        root.close();
        return 0;
      }
      file.close();
      file = root.openNextFile();
    }
    if(err)
      return 2;
    Serial.println("Пользователя с таким id нет в базе");
    return 1;
}

JsonDocument  read_card_list()
{
  String str;
  JsonDocument doc;
  JsonArray arr = doc.as<JsonArray>();
  int i,j;

  File root = SPIFFS.open("/users");
  File file = root.openNextFile();

  while(file)
  {
    str = file.name();
 
    i = str.indexOf(".txt");
    str.remove(i);
     //Serial.println(str);  
     doc.add(str);
    file = root.openNextFile();
  }
  //Serial.println(doc.size());
  return doc;
}



void card_list_compare(JsonDocument t)
{
  File root = SPIFFS.open("/users");
  int i=0,j;
    bool flag = true;
  JsonDocument tmp;     //Список ID, хранящихся во флеши
  String str;

  if(!root)
  {
    Serial.println("Сравнение не выполнено, папка не открылась");
    root.close();
    return;
  }

      File file = root.openNextFile();

  while(file)                                     //получение списка карт
  {
    str = file.name();
    i = str.length();
    if(i>4)
      {i-=4;}
      else{
        Serial.println("Ошибка в имени пользователя");
      }
      str.remove(i);    //Получение id пользователя из имени файла
    tmp.add(str);
    file.close();
    file = root.openNextFile();
  }

  Serial.println("Список пользователей в базе: " + tmp.as<String>());
  Serial.println("Принятый писок пользователей: " + t.as<String>());
  
  flag=true;
  for(i=0;i<t.size();i++)
  {
    for(j=0;j<tmp.size();j++)
    {
           if(t[i].as<String>()==tmp[j].as<String>()) //Если полученный номер совпадает хотя бы 
          {                                           //с одним имеющимся - продолжать поиск
            flag = false;
          }
    }      
      if(flag)                                      //Если Данного ID нет в списке сохраненных
      {                                             // - добавить
        Serial.println("Добавить пользователя с ID - "+t[i].as<String>());
        add_user(t[i].as<String>());
      }
        flag=true;
    
  }
      
  flag=true;
  for(i=0;i<tmp.size();i++)
  {
    for(j=0;j<t.size();j++)
    {
           if(t[j].as<String>()==tmp[i].as<String>()) //Если имеющийся номер совпадает хотя бы 
          {                                           //с одним полученным - продолжать поиск
            flag = false;
          }
     }     
      if(flag)                                      //Если Данного ID нет в списке сохраненных
      {                                             // - добавить
        Serial.println("Удалить пользователя с ID - "+tmp[i].as<String>());
        del_user(tmp[i].as<String>());
      }
        flag=true;
  }
  /*    
    if(flag)                                      //Если размеры совпадают - сравнить номера
    {
      for(i=0;i<t.size();i++)
      {
        flag=false;
        for(j=0;j<tmp.size();j++)
        {
          if(t[i].as<String>()==tmp[j].as<String>()) //Если полученный номер совпадает хотя бы 
          {                                           //с одним имеющимся - продолжать поиск
            flag = true;
            break;
          }
          }      
      } 


      Serial.println("Списки не совпали");
    }else{
          Serial.println("Списки совпали");  
          }
*/          
  Serial.println("Сравнение выполнено");
}

//Сериализация - из JSON в строку, функцию
//Десериализация - из строки в JSON



















