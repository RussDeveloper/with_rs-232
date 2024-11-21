/*
  ESP32 WiFi AP Mode
  http:://www.electronicwings.com
*/
#include "WiFi.h"
#include "esp_spiffs.h"
#include "SPIFFS.h"
#include "AT_func.h"
#include "general.h"
//#include "AsyncElegantOTA.h"
//#include "STA_func.h"
//#include "globals.h"
/**/
EventGroupHandle_t main_event_group;

int x;
const char* assid = "ESP32 WiFI AP";
const char* asecret = "00000000";

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

void setup() 
{
  init_all();
/*
  digitalWrite(GREEN, 1);
  delay(200);
  digitalWrite(GREEN, 0);
*/
    //Создаем задачу, которая будет выполняться на ядре 1 с наивысшим приоритетом (1)
  xTaskCreatePinnedToCore( AT_Task,    // Функция задачи. 
                          "Task1",    // Имя задачи. 
                          10000,     // Размер стека 
                          NULL,       // Параметры задачи 
                          1,          // Приоритет 
                          &Task1,     // Дескриптор задачи для отслеживания 
                          0);         // Указываем номер ядра для этой задачи 
 /* */   
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

void loop()
{
  if(x>500)
  {
    x=0;
    Serial.println("Размер кучи, байт: ");
    Serial.println(heap_caps_get_free_size(MALLOC_CAP_8BIT));
  }
  //x++;
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
  
  Serial.println("Networks_scan :");  
   networks_scan();
   
  Serial.println(""); 
  Serial.println("Creating Accesspoint");
  WiFi.softAP(assid,asecret,4,0,5);
  Serial.print("IP address:\t");
  Serial.println(WiFi.softAPIP());
  Serial.println("Wait 100 ms for AP_START…");
  wifi_ssid = "0";
  wifi_parol = "0";
  
  delay(100);
  

  server.begin();
 
}

