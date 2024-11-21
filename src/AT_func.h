/*
 *

void AT_Task( void *);
 */

#include "WiFi.h"
#include "esp_spiffs.h"
#include "SPIFFS.h"
#include "STA_func.h"
//#include "globals.h"
//TaskHandle_t Task2;

String ssid_buff[50];
char ssid_num;

WiFiClient client;

extern String  wifi_ssid,     //Из файла STA_func
               wifi_parol, 
               parol_serv,
               login_serv
               ;
extern TaskHandle_t Task4;

char hat_mass[2500];
byte *p;        //Указатель для шапки HTML
unsigned int hat_size;   //Длина массива для шапки HTML

WiFiServer server(80); /* Instance of WiFiServer with port number 80 */
String request;
#define  RED 5
#define  GREEN 6
//#define  BLUE


 char i=0,j=0,k=0;

 extern byte wifi_reader();

void networks_scan()                 //Функция сканирования сетей
{
     // i = 
     int st=WiFi.scanNetworks();

     for(j=0;j<st;j++)
    {
      ssid_buff[j] = WiFi.SSID(j);
      vTaskDelay(1);
    }
    ssid_num = st;
}

void page_send(void)
{
  char x,y,z;
  unsigned int m,n;

    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connection: close");
    client.println();

    for(m=0;m<hat_size;m++)
    {
      client.print(hat_mass[m]);
      if(m%500==0)
        vTaskDelay(1);                      //Чтобы WDT собака не кусала
    }

    for(y=0;y<ssid_num;y++)                  //Вывод списка доступных сетей
    {
      client.print("<option value=");
      client.write(0x22);
      client.print(y+1);
      client.write(0x22);
      client.print(">");   
      client.print(ssid_buff[y]); 
      client.println("</option>");        
    }

    client.println("</select>");
    client.println("<br>");
    client.println("<br>");

    vTaskDelay(1);                      //Чтобы WDT собака не кусала
    
    client.println("<label for=\"psw\"><b>Введите пароль</b></label>");
    client.println("<input type=\"password\" placeholder=\" Ведите пароль\" name=\"psw\" required>");
    
    client.println("<label for=\"login\"><b>Введите логин для входа на сервер</b></label>");
    client.println("<input type=\"text\" placeholder=\" логин\" name=\"login\" required>");
    
    client.println("<label for=\"parol\"><b>Введите пароль для входа на сервер</b></label>");
    client.println("<input type=\"text\" placeholder=\" пароль\" name=\"parol\" required>");
    
    client.println("<br>");
    client.println("<br>");  
    
    client.println("<label for=\"mac\"><b>MAC-адрес устройства: ");
    client.println(WiFi.macAddress());
    
    client.println("</b></label>");
    
    client.println("<br>");
    client.println("<br>");    

    client.println("<div class=\"clearfix\">");
    client.println("<button type='submit' class=\"sendbtn\">Передать</button>");
    client.println("</div>");
    client.println("</div>");
    client.println("</form>");
    client.println("</body>");
    client.println("</html>");
}

bool response_handler(String str)
{
  char wifi_name,k,buff[20];
  String wifi_net;//,wifi_parol,parol_serv,login_serv;
  int i,j;
  
  i=str.indexOf("box_field=");
  if(i==-1)
  {return false;}
  else
  {
    
    i+=sizeof("box_field=");
    i--;
    j=str.indexOf("&psw=");        //Поле с номером wifi сети начинается с "box_field=" и заканчивается"&psw="
    for(;i<j;i++)
    {
        wifi_net += str[i];        //Добавлять в строку символы,
    }
  }
   /**/
  i=str.indexOf("&psw=");
  if(i==-1)
  {return false;}
  else
  {
    wifi_parol = "";
    i+=sizeof("&psw=");
    i--;
    j=str.indexOf("&login=");        //Поле с паролем wifi сети начинается с "&psw=" и заканчивается "login="
    //j--;
    for(;i<j;i++)
    {
        wifi_parol += str[i];       //Добавлять в строку символы, пока не попадется &
    }
    Serial.println(wifi_parol);
  }
  
  i=str.indexOf("&login=");
  if(i==-1)
  {return false;}
  else
  {
    login_serv = "";
    i+=sizeof("&login=");
    i--;
    j=str.indexOf("&parol=");        //Поле с логином сервера начинается с ""login="" и заканчивается"&parol="
   // j--;
    for(;i<j;i++)
    {
        login_serv += str[i];     //Добавлять в строку символы, пока не попадется &
    }
  }
 
  j=str.indexOf("&parol=");        //Поле с паролем сервера начинается с "&parol="
  if(j==-1)
  {return false;}
  else
  {
    parol_serv = "";
    j+=sizeof("&parol=");
    j--;
    for(i=0;i<5;i++)
    {
      if(str[j]==' ')
      {
        i=6;
      }else
      {
        parol_serv += str[j];     //Добавлять в строку символы, пока не попадется &
      }
      j++;
    }
  }

  wifi_ssid = ssid_buff[wifi_net.toInt()-1];

  Serial.println(wifi_parol);

  return true;
}

void request_handler(String *str)
{
  if((str->indexOf("GET / HTTP/1.1")!=-1))
  {
    Serial.println("(str->indexOf(\"GET / HTTP/1.1\")!=-1)");
  }
    page_send();  
    
    Serial.println("page_send");  

}

void AT_Task( void *pvParameters )
{
  static int tme;
  unsigned int m,n;
  char *ptr;
    
    File file = SPIFFS.open("/hat.html");
    if(!file){
         Serial.println("There was an error opening the file for writing");
         return;
              }
              
       hat_size = file.size();

      for(m=0;m<hat_size;m++)            //Перегрузка в оперативу
      {
        hat_mass[m] = file.read();

      }

/*     client = server.available();*/
      if(wifi_reader())
      {
        xTaskCreatePinnedToCore(STA_Task,   // Функция задачи. 
                                "STA_Task", // Имя задачи. 
                                10000,      // Размер стека 
                                NULL,       // Параметры задачи 
                                1,          // Приоритет 
                                &Task4,     // Дескриптор задачи для отслеживания 
                                1);         // Указываем номер ядра для этой задачи    

       
      }
   
 
 for(;;)///////////////////////////////////////////////////////////////////////////////////
       {
  /**/
    client = server.available();
  if(client)
  {
    if(client.connected())
      Serial.println("Client connected ");
      
      while (client.connected())
      {
        if (client.available())
        {
          char c = client.read();
          request += c;
          if (c == '\n')
          {
            Serial.println("HTML request: ");
            Serial.print(request);
            //request_handler(&request);
            page_send();
            if(response_handler(request))
            {
              Serial.println("Прием данных выполнен");
              /**/
              if(Task4==NULL)
               //if(eTaskGetState(Task4)!=eReady)
                xTaskCreatePinnedToCore(STA_Task,   // Функция задачи. 
                                        "STA_Task", // Имя задачи. 
                                        10000,      // Размер стека 
                                        NULL,       // Параметры задачи 
                                        1,          // Приоритет 
                                        &Task4,     // Дескриптор задачи для отслеживания 
                                        1);         // Указываем номер ядра для этой задачи 
                                        
            }
            break;
            }
          }else
          {
            //
          }  
         vTaskDelay(1);                    
        }
      Serial.println("Client disconnected ");        
      request="";
      client.stop();
  }else
  {
    if(tme>1000)
    {
      networks_scan();
      tme=0;
      Serial.println("Сканирование сетей");   
    }
    //tme++;
  }
     vTaskDelay(1); 
    }
}
