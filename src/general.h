
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
extern JsonDocument card_list, action;
       JsonDocument card_list_flash;

extern char load_buff[5*num_nodules];
extern String card_val;
//extern int box_action(JsonDocument action);
//extern int box_action(String act);

//char user_nums[100][10];

String user;
uint32_t  lock_timer;
bool lock,
      record            //Флаг режима добавления инструмента
      ;

typedef struct
{
  String id;
  String mask;
  String tool_light;
  String shelf_light;
}tool_form;

//tool_form tool[50];

JsonDocument tool,
              tool_list, 
              user_list,     //JSON в котором записываются все пользователи
              users          //JSON в котором записываются все инструменты 
            ;                //взятые каждым пользователем

bool dell_item(JsonDocument *, String );
int add_tool(JsonDocument);
int del_tool(String);
int add_user(String);
int del_user(String);
String convert_to(String, bool);      //Функция приведения информации в формат для передачи на сервер
JsonDocument get_tools();
JsonDocument get_tool_list();         //JSON с ID-шниками инструментов
String get_tool();                    //если произошлои зменение датчиков - выдает ID инструмента
JsonDocument get_users();             //Возвращает json с ключами, которые являются номерами карт
JsonDocument get_users_list();        //JSON с ID-шниками инструментов
JsonDocument _get_users();
JsonDocument  read_card_list();
void card_list_compare(JsonDocument);
bool chek_item(JsonDocument, String);

void del(int val)
{while(val--);}

void general_task( void * pvParameters)
{
  int list_size,i,j,k;
  char *p, mass[50];
  String list_str, list_card, msg;
  JsonDocument doc, t_doc, tools;
  extern JsonDocument card_list;
  lock_timer = 0;
  //JsonArray u_arr = card_list.as<JsonArray>();

  //Запрос данных из файлов
  //File t_list = SPIFFS.open("/tool_list.txt");
  
  tool_list = get_tools();        //список инструментов
  user_list = get_users_list();   //Список пользователей

  users = get_users();          

  Serial.println("//список инструментов");

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

  for(;;)
  {
    if(xEventGroupGetBits(main_event_group)&RFID_flag)    //Если RFID_flag считан
    {
      serializeJsonPretty(card_list, list_card);
      if((list_card.indexOf(card_val)!=-1))//&&(lock_timer==0))             //фиксация совпадения карты
      {
        //card_list.clear();
        lock_timer = 60*100;                              //При совпадении карты ящик открыт минуту
        user = card_val;
        Serial.print("Доступ открыт на минуту для пользователя с ID :");
        Serial.println(card_val);
        set_locks(0xfffff);
      }
        else{
              Serial.println("Нет доступа");
              //Serial.println(list_card);
              //Serial.println(card_val);
            }
      xEventGroupClearBits(main_event_group, RFID_flag);
      
      //Serial.println("card_list.as<int>()");
      //Serial.println(card_list.as<String>());
      card_val.clear();
    }

    if(xEventGroupGetBits(main_event_group)&sensors_flag)    //Если сенсоры изменились
    {
      digitalWrite(RED,HIGH);
      delay(50);
      digitalWrite(RED,LOW);
      //xEventGroupClearBits(main_event_group, sensors_flag);
      //Работа только когда не происходит операция добавления инструмента в базу
      if(record == false)
      {
      JsonDocument list = get_tool_list();
      tools = get_tools();

      JsonDocument ms_doc;
      byte match, num_bit;
      int vl;

      for(j=0;j<list.size();j++)
      {
        
        ms_doc = tools[list[j]];
        /**/
        JsonArray msk = ms_doc["mask"].as<JsonArray>();
        //String msk = t_doc["mask"].as<String>(); 

         num_bit=0;

         for(i=0;i<50;i++)
         {
          k = msk[i].as<int>(); 
          if(k)
            num_bit++;        
         }
          for(i=0;i<50;i++)
         {
          k = msk[i].as<int>();
          //Serial.print(k);          
          if(k>0)
          {
            match=0;
            if((k&sens_delta[i])==k)
            {
              match|=0x1;
              if(num_bit)
                num_bit--;
            }else
            {
              match|=0x2;
            }
          }
        }
             if((match&0x1)&&(num_bit==0))//((match&0x1)&&(match&0x2==0))
        {
          /**/
          String id =ms_doc["id"].as<String>();
          Serial.print("Инструмент: ");
          Serial.println(id);

          if(sens_change>0)
          {
            Serial.println("Был установлен");
            s_action = convert_to(id, true);
           }
          if(sens_change==0)
          {
            Serial.println("Был взят");
            s_action = convert_to(id, false);       
          }
          Serial.println(s_action);

          if(sens_change>0)     //Если "Был установлен"
          {
            bool fl1=false;
           for(int a=0;a<user_list.size();a++)                    //Поиск пользователя, который брал и удаление из его списка
           {/**/
            JsonArray _arr = users[user_list[a].as<String>()];
            for(int q=0;q<_arr.size();q++)
            {
              if(_arr[q].as<String>()==id)
              {
                _arr.remove(q);
                fl1=true;
              }
            }
           }
            if(fl1)
              Serial.println("Инструмент с ID " +id+" вернуть не удалось");
          }else
          {
              users[user].add(id);
          }
          }  
        }

      for(i=0;i<50;i++)
        sens_delta[i] = 0;
      }
      /*
            for(i=0;i<50;i++)
            {
              if(sens_delta[i])
              {
                for(j=0;j<list.size();j++)
                {
                  JsonArray mask = t_doc[list[j]]["mask"].as<JsonArray>();
                    if((mask[i]&sens_delta[i])==sens_delta[i])
                    {

                    }
                }
              }
            }
      */      
      /*
      for(i=0;i<50;i++)
      {
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
        
      }  */
     
    }
    if(xEventGroupGetBits(main_event_group)&wifi_flag)    //Если WiFi подключен
    {
      if(xEventGroupGetBits(main_event_group)&sensors_flag)
      {

        for(int a=0;a<users.size();a++)
        {
          if(user_list[users[a]].size()>0)
          {
            
          }
        }
      }

    }else{
      if(card_list.isNull())                              //Если выключен - берем список из флеши
      {
        card_list=read_card_list();
        Serial.println("Работа в автономном режиме. Список крат:   ");
        Serial.println(card_list.as<String>());
      }
    }

    if(xEventGroupGetBits(main_event_group)&rs232_flag)              //Сообщение от планшета
      {
          if(!rs_232.reseive.isNull()){
            msg += rs_232.reseive["command"].as<String>();
            //serializeJson(rs_232.reseive, Serial);
            //delete ptr;
              if(msg =="start")                   //При добавлении инструмента - сохраняется битовая маска до добавления
              {
                record = true;
                for(i=0;i<50;i++)
                  {
                    mass[i] = sens_buff[i];
                    sens_delta[i] = 0;
                  }

                rs_232.rx_flag=false;
                rs_232.reseive.clear();
              }
              if(msg =="end")  
              {
                doc.clear();
                doc["id"] = rs_232.reseive["value"];
                //JsonDocument doc2;
                //doc2[rs_232.reseive["value"]] = t_doc;
                
                //Serial.println(doc["id"].as<String>());

                //JsonArray arr = doc["mask"].as<JsonArray>();
                int mod=0;
                for(i=0;i<50;i++)
                {
                  //arr.add((unsigned char)sens_delta[i]);
                  doc["mask"].add((unsigned char)sens_delta[i]);
                  if((sens_delta>0)&&(mod==0))
                    mod = i/5;
                }
                doc["cell"] = mod;
                serializeJson(doc, Serial);//////////////////////////
                if(add_tool(doc)==0)
                {
                  rs_232.transmit["command"] = "ok";
                  String val =  rs_232.reseive["value"];
                  rs_232.transmit["id"] = val;
                  rs_232.tx_flag = true;}
                  else{
                  rs_232.transmit["command"] = "false";
                  String val =  rs_232.reseive["value"];
                  rs_232.transmit["id"] = val;
                  rs_232.tx_flag = true;
                  }
                  record = false;  
              }
              if(msg =="delete")  
              {
                if(del_tool(rs_232.reseive["value"]))
                {
                  rs_232.transmit.clear();
                  rs_232.transmit["command"] = "delete";
                  rs_232.transmit["value"] = "ok";
                  rs_232.tx_flag = true;
                }

              }
              if(msg =="light")  
                {
                  
                }

              msg.clear();
          }
          else{ Serial.println("Json опять мозги ебет");}

          xEventGroupClearBits(main_event_group, rs232_flag);
        rs_232.rx_flag = 0;
      }
    
    xEventGroupClearBits(main_event_group, sensors_flag);
    delay(10);
    if(lock_timer==0)
      user.clear();

    if(lock_timer<(59*100))
      set_locks(0);
    if(lock_timer)
    {
      if(lock_timer%100==0)
      {
        Serial.println("До закрытия осталось :");
        Serial.println(lock_timer/100, DEC);
        Serial.println("секунд");
      }
      lock_timer--;
    }
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
    //Serial.print("serializeJson(t, Serial); :");
    serializeJson(t, Serial);

    while(file)                     //Поверка на наличие уже созданных файлов
    {
      //str=file.name();

      if(str==file.name())
      {
        seach=true;
        Serial.println("Инструмент с таким номеров уже существует...");
        Serial.println(str);
        file.close();
        root.close();
        record = false;
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
      record = false;
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
  return 0;
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
  //JsonArray arr = doc.as<JsonArray>();
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
  root.close();
  file.close();
  //Serial.println(doc.size());
  return doc;
}

void card_list_compare(JsonDocument t)
{
  File root = SPIFFS.open("/users");
  int i=0,j;
    bool flag = true, log = false;
  //JsonDocument tmp;     //Список ID, хранящихся во флеши
  String str;

  if(!root)
  {
    Serial.println("Сравнение не выполнено, папка не открылась");
    root.close();
    return;
  }

      File file = root.openNextFile();

  if(file)                      
    card_list_flash.clear();

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
    //tmp.add(str);
    card_list_flash.add(str);
    file.close();
    file = root.openNextFile();
  }
    if(log)
    {
      //Serial.println("Список пользователей в базе: " + tmp.as<String>());
      Serial.println("Список пользователей в базе: " + card_list_flash.as<String>());
      Serial.println("Принятый писок пользователей: " + t.as<String>());
    }
  flag=true;
  for(i=0;i<t.size();i++)
  {
 //   for(j=0;j<tmp.size();j++)
    for(j=0;j<card_list_flash.size();j++)
    {
           //if(t[i].as<String>()==tmp[j].as<String>()) //Если полученный номер совпадает хотя бы 
           if(t[i].as<String>()==card_list_flash[j].as<String>()) //Если полученный номер совпадает хотя бы 
          {                                           //с одним имеющимся - продолжать поиск
            flag = false;
          }
    }      
      if(flag)                                      //Если Данного ID нет в списке сохраненных
      { if(log)                                            // - добавить
        Serial.println("Добавить пользователя с ID - "+t[i].as<String>());
        add_user(t[i].as<String>());
      }
        flag=true;
  }
      
  flag=true;
  for(i=0;i<card_list_flash.size();i++)
  {
    for(j=0;j<t.size();j++)
    {
           //if(t[j].as<String>()==tmp[i].as<String>()) //Если имеющийся номер совпадает хотя бы 
           if(t[j].as<String>()==card_list_flash[i].as<String>()) //Если имеющийся номер совпадает хотя бы 
          {                                           //с одним полученным - продолжать поиск
            flag = false;
          }
     }     
      if(flag)                                      //Если Данного ID нет в списке сохраненных
      {                                             // - добавить
        Serial.println("Удалить пользователя с ID - "+card_list_flash[i].as<String>());//tmp[i].as<String>());
        del_user(card_list_flash[i].as<String>());///(tmp[i].as<String>());
      }
        flag=true;
  }
        
  Serial.println("Сравнение выполнено");
}

JsonDocument get_tools()
{
  JsonDocument doc, temp;
  String str;
  int i,j;

      File root = SPIFFS.open("/tools");
       File file = root.openNextFile();

      while(file)                     //Поверка на наличие уже созданных файлов
    {
      DeserializationError err = deserializeJson(temp, file);
      if(err!= DeserializationError::Ok)
        Serial.println("При чтении списка инструментов произошла ошибка десериализации");
/**/
        JsonArray arr = temp["mask"].as<JsonArray>();
        j=0;
        for(int r=0;r<50;r++)
        {
           i = arr[r];
           if(i)
           {
            j|=0x1;
            if((i&sens_buff[r])!=i)
            {
              j|=0x2;
            }
          }
        }
        if(j==0x1)
        {
          temp["isHere"] = "true";
        }else{
          temp["isHere"] = "false";
        }
        str = file.name();
        int t = str.indexOf(".txt");
        str.remove(t);
      doc[str] = temp;
      //Serial.println(str);
      temp.clear();
      file.close();
      file = root.openNextFile();
    }
  return doc;
}
JsonDocument get_tool_list()
{
  JsonDocument doc;
  String str;

      File root = SPIFFS.open("/tools");
       File file = root.openNextFile();

      while(file)                     //Поверка на наличие уже созданных файлов
    {
      str = file.name();
      int t = str.indexOf(".txt");
      str.remove(t);
      doc.add(str);
      file.close();
      file = root.openNextFile();
    }
  return doc;
}

JsonDocument _get_users()
{
  JsonDocument doc, temp;
  String str;
  int i,j;

      File root = SPIFFS.open("/users");
       File file = root.openNextFile();

       if(root)
      {
      while(file)                     //Поверка на наличие уже созданных файлов
      {
        str = file.name();
        i = str.indexOf(".txt");
        str.remove(i);
        doc.add(str);        
        temp.clear();
        file.close();
        file = root.openNextFile();
      }
      }else
      {
        Serial.println("В функции get_users() не удалось открыть папку /users");
      }
  return doc;
}
JsonDocument get_users()
{
  JsonDocument doc, temp;
  String str;
  int i,j;

    File root = SPIFFS.open("/users");
    File file = root.openNextFile();

      if(root)
    {
      while(file)                     //Поверка на наличие уже созданных файлов
      {
        str = file.name();
        i = str.indexOf(".txt");
        str.remove(i);
        doc[str] = nullptr;        
        temp.clear();
        file.close();
        file = root.openNextFile();
      }
    }else
    {
      Serial.println("В функции get_users() не удалось открыть папку /users");
    }
    root.close();
  return doc;
}

bool chek_item(JsonDocument dc, String str)
{
  return true;
}

String get_tool()
{
  String str;
  return str;
}
//Сериализация - из JSON в строку, функцию
//Десериализация - из строки в JSON

String convert_to(String id, bool here)
{
        String str;
        if(user.isEmpty())
          return str;
        str = "{\"Card\": \"";
        str+= user;
        str+="\", \"idFilling\": ";
        str+= id;
        str+=", \"Date\": \"";
        str+=timeinfo.tm_year+1900;
        str+="-"; 
        if(timeinfo.tm_mon<10)
          str+="0";       
        str+=timeinfo.tm_mon; 
        str+="-";  
        if(timeinfo.tm_mday<10)
          str+="0";       
        str+=timeinfo.tm_mday;
        str+="T";
        if(timeinfo.tm_hour<10)
          str+="0";       
        str+=timeinfo.tm_hour;
        str+=":";
        if(timeinfo.tm_min<10)
          str+="0";          
        str+=timeinfo.tm_min;
        str+=":";
        if(timeinfo.tm_sec<10)
          str+="0";          
        str+=timeinfo.tm_sec;
        str+="\", \"isHere\": ";

        if(here)
        {str+= "true}";}else{str+= "false}";}

        return str;
}

bool dell_item(JsonDocument *p, String &str)
{
  JsonDocument _doc;
  JsonArray _arr = p->as<JsonArray>();
  bool fl1=false;
    
    for(int q=0;q<_arr.size();q++)
    {
      if(_arr[q].as<String>()==str)
      {
        _arr.remove(q);
        fl1=true;
      }
    }
    return fl1;
}

JsonDocument get_users_list()
{
  JsonDocument doc;
  String str;

      File root = SPIFFS.open("/users");
       File file = root.openNextFile();

      while(file)                     //Поверка на наличие уже созданных файлов
    {
      str = file.name();
      int t = str.indexOf(".txt");
      str.remove(t);
      doc.add(str);
      file.close();
      file = root.openNextFile();
    }
  return doc;

}
















