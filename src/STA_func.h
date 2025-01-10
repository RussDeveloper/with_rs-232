///#include "globals.h"
#include <WiFi.h>
#include <EEPROM.h>
#include "FS.h"
#include "SPIFFS.h"
#include "http_con.h"
#include "globals.h"

String  wifi_ssid,      //Set Your SSID
        wifi_parol,     //Set Your Passwor
        parol_serv,
        login_serv,
        token_serv
        ;

WiFiServer STA_Server(80);
//WiFiClient STA_client;

 struct tm timeinfo;
extern TaskHandle_t Task2;

extern EventGroupHandle_t main_event_group;

#define  RED 5
#define  GREEN 6

#define FORMAT_SPIFFS_IF_FAILED true
// Not sure if NetworkClientSecure checks the validity date of the certificate.
// Setting clock just to be sure...192.168.0.7
void setClock() {
  configTime(0, 0, "pool.ntp.org");
  //configTime(0, 0, "192.168.0.7");

  Serial.print(F("Waiting for NTP time sync: "));
  time_t nowSecs = time(nullptr);
  while (nowSecs < 8 * 3600 * 2) {
    delay(500);
    Serial.print(F("."));
    yield();
    nowSecs = time(nullptr);
  }
    Serial.println();
 
  gmtime_r(&nowSecs, &timeinfo);
  Serial.print(F("Current time: "));
  Serial.print(asctime(&timeinfo));
}

byte wifi_reader()
{
   int ssid_len, 
              parol_len, 
              p_serv_len,
              l_serv_len,
              t_serv_len,
              i,j;

  char buff[50];
      
  EEPROM.begin(256);

  if(EEPROM.read(0)>0)
  {
    EEPROM.end();
    Serial.println("EEPROM is empty");
    return 0;
  }

  ssid_len =    EEPROM.read(1);
  parol_len =   EEPROM.read(2);
  l_serv_len =  EEPROM.read(3);
  p_serv_len =  EEPROM.read(4);
  t_serv_len =  EEPROM.read(5);

  j=6;
  for(i=0;i<ssid_len;i++)           //Логин сети
  {
    buff[i] = EEPROM.read(j);
    j++;
  }
  buff[i]=0;
  wifi_ssid = String(buff);

  for(i=0;i<parol_len;i++)           //Пароль сети
  {
    buff[i] = EEPROM.read(j);
    j++;
  }
  buff[i]=0;
 wifi_parol = String(buff);

  for(i=0;i<l_serv_len;i++)           //Логин сервера
  {
    buff[i] = EEPROM.read(j);
    j++;
  }
  buff[i]=0;
  login_serv = String(buff);

  for(i=0;i<p_serv_len;i++)           //Пароль сервера
  {
    buff[i] = EEPROM.read(j);
    j++;
  }
  buff[i]=0;
  parol_serv = String(buff);

  for(i=0;i<t_serv_len;i++)           //Токен сервера
  {
    buff[i] = EEPROM.read(j);
    j++;
  }
  buff[i]=0;
  token_serv = String(buff);



  Serial.println("Логин :");
  Serial.println(wifi_ssid);
    Serial.println("");
  Serial.println("Пароль:");
  Serial.println(wifi_parol);
    Serial.println("");
   /*   
  Serial.println("Логин сервера:");
  Serial.println(login_serv);
    //Serial.println("");
  Serial.println("Пароль сервера:");
  Serial.println(parol_serv);
    //Serial.println("");
  Serial.println("Токен сервера:");
  Serial.println(token_serv);
    //Serial.println("");*/
  /*
  for(i=0;i<50;i++)
  {
    Serial.println(EEPROM.read(i));
  }*/
  EEPROM.end();
  
  return 1;
}

byte wifi_saver()
{
  static int tme;
  byte i,j,k;
  String str = "";

  if(digitalRead(7))
  {
    tme=0;
  }else{
    tme++;
    if(tme%100)
      Serial.print("Wait");
  }

  if(tme>100)        //Действия при удержании кнопки
  {
    //Сохранение данных подключения wifi во флеши
    EEPROM.begin(512);

    EEPROM.write(0,0);
    EEPROM.write(1,wifi_ssid.length());
    EEPROM.write(2,wifi_parol.length());
    EEPROM.write(3,login_serv.length());
    EEPROM.write(4,parol_serv.length());
    EEPROM.write(5,token_serv.length());

    j=6;
    for(i=0;i<wifi_ssid.length();i++)     //логин
    {
      EEPROM.put(j,wifi_ssid[i]);
      j++;
    }
    
    for(i=0;i<wifi_parol.length();i++)    //Пароль
    {
      EEPROM.put(j,wifi_parol[i]);
      j++;
    }

    for(i=0;i<login_serv.length();i++)    //логин сервера
    {
      EEPROM.put(j,login_serv[i]);
      j++;
    }
    
    for(i=0;i<parol_serv.length();i++)    //пароль сервера
    {
      EEPROM.put(j,parol_serv[i]);
      j++;
    }

    for(i=0;i<token_serv.length();i++)    //токен сервера
    {
      EEPROM.put(j,token_serv[i]);
      j++;
    }

    EEPROM.commit();
    
    while(digitalRead(7)==0)      //Ждем отпускания кнопки
    {
      digitalWrite(RED, HIGH);
      digitalWrite(GREEN, HIGH);
      vTaskDelay(5);
    }
      digitalWrite(RED, LOW);
      digitalWrite(GREEN, LOW);
      tme=0;
      /*
      j=wifi_ssid.length()+wifi_parol.length()+5;
      
      Serial.println("wifi_saver_lenght:");
              vTaskDelay(1);
      Serial.println(j);
        vTaskDelay(1);
      Serial.println("wifi_saver_acton ");
              vTaskDelay(1);
      for(i=3;i<50;i++)
      {
        EEPROM.get(i,k);
        str+=k;
      }
      str+=" ";
      str+=0x00;
      Serial.println(str);*/
      wifi_reader();
  }

  i = EEPROM.read(0);
      EEPROM.end();
           
           
  if(i)
    return 0;
  return 1;
}

void STA_Task( void *pvParameters )
{
  int i;
  WiFi.config(IPAddress(192 ,168,9,117),IPAddress(192,168,0,1),IPAddress(255,255,0,0),IPAddress(192,168,0,7));

  if(wifi_ssid.isEmpty())
    wifi_reader();

    Serial.println("SSID:");
    Serial.println(wifi_ssid);
    Serial.println("wifi_pass:");
    Serial.println(wifi_parol);
 
  while(WiFi.status() != WL_CONNECTED) 
  {
    WiFi.begin(wifi_ssid, wifi_parol);
    Serial.print(".");
    vTaskDelay(3000);
    }
    //setClock();

  Serial.println("STA_Task запущена");
  FS _fs(SPIFFS);
/*
  File root = _fs.open("/");
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
    file = root.openNextFile();
  }
*/

  //String srt = SPIFFS.
 // u_list.close();
  
  for(;;)
  {          
    wifi_saver();

    if(wifi_ssid!=0)
    {
      if(WiFi.status() == WL_CONNECTED) 
       { 
          xEventGroupSetBits(main_event_group, wifi_flag);
          digitalWrite(GREEN, HIGH); 

          http_master();           
       }else
        {
         digitalWrite(GREEN, LOW); 
         xEventGroupClearBits(main_event_group, wifi_flag);
          WiFi.disconnect();
          vTaskDelay(100);
          Serial.println("wifi_ssid");
          Serial.println(wifi_ssid);
          Serial.println("wifi_parol");
          Serial.println(wifi_parol);
          WiFi.begin(wifi_ssid, wifi_parol);
           vTaskDelay(2000);
         }
     }     
      vTaskDelay(10);
  }
}
