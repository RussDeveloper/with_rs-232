/*
  ESP32 WiFi AP Mode
  http:://www.electronicwings.com
*/
#include "WiFi.h"
#include "esp_spiffs.h"
#include "SPIFFS.h"
#include "AT_func.h"
#include "general.h"
#include "brige.h"
//#include "globals.h"
/**/
EventGroupHandle_t main_event_group;

int x;
const char* assid = "ESP32 WiFI AP";
const char* asecret = "00000000";

void read_file(const char*);

TaskHandle_t  Task1 = NULL,
              Task2 = NULL,
              Task3 = NULL,
              Task4 = NULL,
              Task5 = NULL,              
              Task6 = NULL              
              ;

#define  RED 5
#define  GREEN 6

void init_all();
void listDir(fs::FS &fs, const char *dirname, uint8_t levels);

JsonDocument t1, t2;
extern  struct tm timeinfo;

void setup() 
{
  init_all();
  String str;  
  char p1[10];
  sens_buff[0] = 0x10;
  sens_buff[2] = 0x20;  



  JsonArray msk = t1["mask"].to<JsonArray>();
  for(i=0;i<50;i++)
  {
    sprintf(p1,"%u", sens_buff[i]);   //"%u" - десятичное число без знака
                                      //"%х - шестнадцатеричное число без знака"
    msk.add(p1);
}
  t1["tool_light"] = "1";
  t1["shelf_light"] = "2";
/*
  msk = t1["id"].to<JsonArray>();
  msk.add("321");
  msk.add("321");
  msk.add("321");
*/
  t2["id"] = t1;

    //Создаем задачу, которая будет выполняться на ядре 1 с наивысшим приоритетом (1)
  /*  */ 
  xTaskCreatePinnedToCore( AT_Task,    // Функция задачи. 
                          "Task1",    // Имя задачи. 
                          10000,     // Размер стека 
                          NULL,       // Параметры задачи 
                          1,          // Приоритет 
                          &Task1,     // Дескриптор задачи для отслеживания 
                          1);         // Указываем номер ядра для этой задачи 
    
  xTaskCreatePinnedToCore(spi_task,    // Функция задачи. 
                          "Task2",    // Имя задачи. 
                          10000,     // Размер стека 
                          NULL,       // Параметры задачи 
                          1,          // Приоритет 
                          &Task2,     // Дескриптор задачи для отслеживания 
                          0);         // Указываем номер ядра для этой задачи 
   
  xTaskCreatePinnedToCore( card_task,    // Функция задачи. 
                          "card_task",    // Имя задачи. 
                          10000,     // Размер стека 
                          NULL,       // Параметры задачи 
                          1,          // Приоритет 
                          &Task3,     // Дескриптор задачи для отслеживания 
                          0);         // Указываем номер ядра для этой задачи 
 /*
   xTaskCreatePinnedToCore(STA_Task,    // Функция задачи. 
                          "STA_Task",    // Имя задачи. 
                          10000,     // Размер стека 
                          NULL,       // Параметры задачи 
                          2,          // Приоритет 
                          &Task4,     // Дескриптор задачи для отслеживания 
                          1);         // Указываем номер ядра для этой задачи 
   */                
   xTaskCreatePinnedToCore(general_task,    // Функция задачи. 
                          "general_task",         // Имя задачи. 
                          10000,                  // Размер стека 
                          NULL,                   // Параметры задачи 
                          2,                      // Приоритет 
                          &Task5,                 // Дескриптор задачи для отслеживания 
                          0);                     // Указываем номер ядра для этой задачи 
                   
   xTaskCreatePinnedToCore(rs232_task,            // Функция задачи. 
                          "rs232_task",            // Имя задачи. 
                          10000,                  // Размер стека 
                          NULL,                   // Параметры задачи 
                          2,                      // Приоритет 
                          &Task6,                 // Дескриптор задачи для отслеживания 
                          0);                     // Указываем номер ядра для этой задачи 
              
  }   

  JsonDocument j_doc;

void loop()
{
  int i[5],j,k[5],t,f;


  if(x>2000)
  {
    x=0;
    Serial.println("Размер кучи, байт: ");
    Serial.println(heap_caps_get_free_size(MALLOC_CAP_8BIT));
  }
  //x++;
  /**/
  if(Serial.available())
  {
    f = Serial.read();
    //Serial.println(t, HEX);   
  }
    switch (f)
    {
      case '1':{
                j_doc = get_users();
                JsonArray _ar = j_doc.as<JsonArray>();  
                for(int a=0;a<user_list.size();a++)
                {
                  Serial.println(user_list[a].as<String>());
                }   
                }break;

      case '2':{
                serializeJson(user_list, Serial);  
                Serial.println("");
                }break;

      case '3':{
                serializeJson(users, Serial);  
                Serial.println("");
                }break;

    }

    switch (t)
    {
    case '1':{
      for(int r=0;r<10;r++)
        load_buff[r] = 0x0f;
    }break;

    case '2':{
      for(int r=0;r<10;r++)
        load_buff[r] = 0x00;
    }break;
    
    case '3':{
      for(int r=10;r<20;r++)
        load_buff[r] = 0xf0;
    }break;

    case '4':{
      for(int r=10;r<20;r++)
        load_buff[r] = 0x00;
    }break;
    case '5':{
      String str;
            str+=timeinfo.tm_mday;
            str+=".";
            str+=timeinfo.tm_mon;
            str+=".";
            str+=timeinfo.tm_year+1900;

        Serial.print(F("Current date: "));
        Serial.print(str);
        str.clear();
    }break; 

    case '6':{
      Serial.println(token);
    }break;
    
    case '7':{
      Serial.println(j_doc["main1"].add("p1"));
      Serial.println(j_doc["main1"].add("p2"));
      //serializeJson(j_doc, Serial);
    }break;
    case '8':{
      j_doc["main1"].clear();
      //serializeJson(j_doc, Serial);
    }break;
    case '9':{
      JsonArray arr = j_doc["main1"];
     Serial.println(j_doc["p2"].is<int>());
     Serial.println(j_doc["main1"]["p2"].is<int>());
      //serializeJson(j_doc, Serial);
    }break;
    case '0':{
              j_doc["main1"].remove(String("p2"));
    }break;
    }
    
    if((f>='0')&&(f<='9'))
    {
      serializeJson(j_doc, Serial);
      Serial.println("");
      delay(100);
    }
    t=0;
    switch (j)
    {
    case '1':{
      Serial.print("add_tool(t);  ");
      Serial.println(add_tool(t1));
            Serial.print("add_user(129);  ");
      Serial.println(add_user(String("129")));
      }
      break;
    case '2':{serializeJson(get_users(), Serial);}//read_file("/list_users.txt");}
      break;
    case '3':{serializeJson(get_tools(), Serial);}
      break;
    case '4':{
      Serial.println("Маска датчиков");
      for(int f=0;f<50;f++)
        Serial.print(sens_delta[f], HEX);
        Serial.println("");
      }
      break;
    case '5':{
      Serial.println("Просмотр t2");
      serializeJson(t2, Serial);
      Serial.println("t2.nesting()");
      Serial.println(t2.nesting());
      }
      break;
    case '6':{
      listDir(SPIFFS, "/tools", 0);
      }
      break;
    case '7':{
      listDir(SPIFFS, "/users", 0);
      }
      break;
    case '8':{
      t1.clear();
      t1 = get_tool_list();
      JsonArray arr = t1.as<JsonArray>();
        Serial.println("arr.size()");
        Serial.println(arr.size());
        for(int r=0;r<arr.size(); r++)
        {
          Serial.println(arr[r].as<String>());
        }
        //Serial.println(get_users().as<String>().indexOf("124"));
      }
      break;
    case '9':{
        //serializeJson(tool_list, Serial);

        load_buff[5] = 0xff;
      }
      break;
    case '0':{
      //Serial.print(analogRead(3), DEC);
      load_buff[5] = 0x00;
      }
      break;
  }
    j=0;  
  delay(1);
}

void init_all()
{
  Serial.begin(115200);
  pinMode(2, OUTPUT);
  pinMode(7,INPUT_PULLUP);
  pinMode(RED, OUTPUT);
  pinMode(GREEN ,OUTPUT);

  main_event_group = xEventGroupCreate();       //флаги глобальных событий

  if(!SPIFFS.begin(true)){
        Serial.println("An Error has occurred while mounting SPIFFS");
        return;
  }
  
  WiFi.mode(WIFI_AP_STA);
  /**/
  
  Serial.println("Networks_scan :");  
  networks_scan();
   
  Serial.println(""); 
  Serial.println("Creating Accesspoint");
  WiFi.softAP(assid,asecret,4,0,5);
  Serial.print("IP address:\t");
  Serial.println(WiFi.softAPIP());
  Serial.println("Wait 100 ms for AP_START…");
  //wifi_ssid = "0";
  //wifi_parol = "0";
  
  delay(100);
  

  server.begin();
 
}

void read_file(const char* path)
{
  int i=0;

  File file = SPIFFS.open(path);

  while(i<5)
  {
    if(!file)
    {
      file = SPIFFS.open(path);
      Serial.print("Open :");
      Serial.println(path); 
      delay(500);  
    }else{i=5;}
  }
  
  Serial.println(path); 

  while(file.available())
    Serial.write(file.read());

  file.close();
}

void listDir(fs::FS &fs, const char *dirname, uint8_t levels) {
  Serial.printf("Listing directory: %s\r\n", dirname);

  File root = fs.open(dirname);
  if (!root) {
    Serial.println("- failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    Serial.println(" - not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if (levels) {
        listDir(fs, file.path(), levels - 1);
      }
    } else {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("\tSIZE: ");
      Serial.println(file.size());
    }
    file.close();
    file = root.openNextFile();
  }
  root.close();
  file.close();
}









