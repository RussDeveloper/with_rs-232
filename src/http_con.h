#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>

//IP =  95.31.39.61:51112 - ип сервера стекломаш

//Your Domain name with URL path or IP address with path
String servergetToken("https://api.steklm.ru:51112/api/auth/getToken");
//String servergetCardList("https://api.steklm.ru:51112/api/card/ ");
String servergetCardList("https://api.steklm.ru:51112/api/Card?mac=3C-84-27-20-F5-B4");
//String servergetCardList("https://api.steklm.ru:51112/api/card/getCard");

String getToken_req("{\"Login\": \"3C-84-27-20-F5-B4\",\"Password\": \"ChkalovTest\"}"); //3C:84:27:20:F5:B4
String body_req("{\"Login\": \"3C-84-27-20-F5-B4\",\"Password\": \"ChkalovTest\"}"); //3C:84:27:20:F5:B4
String getToken_req1("{\n\"Login\": \"test\",\n\"Password\": \"test\"\n}");
byte server1[] = { 192, 168, 0, 26 };

String token, r_login;
JsonDocument card_list;

bool get_token(void);
JsonDocument get_card_list(void);
JsonDocument set_cust_action(String);

String http_request(String, String, String);

void http_master()
{
  static unsigned long tmr = 0, t_val=0;
  String val = "" ,header = "", data = "";

  if(tmr>100)
  {
    get_token();
    /**/
    if(token.length()>0)
    {
      card_list = get_card_list();
      
      if(!card_list.isNull())
      {
        if(serializeJson(card_list, val))              //Получение списка карт
          {
            Serial.println("Список карт: ");
            Serial.println(val);
            File u_list = SPIFFS.open("/list_users.txt");
            data = u_list.readString();
            if(val != data)                           //В случае несовпадения полученного и сохраненного списка
            {
              u_list.close();
              SPIFFS.remove("/list_users.txt");
              u_list = SPIFFS.open("/list_users.txt");
              u_list.write((const uint8_t*)val.c_str(), val.length());    //Перезаписываем содержимое файла
              u_list.close();
            }
          }

      }else
      Serial.println("Запрос на карты не верен");
    }
    card_list.clear();
    tmr=0;
  }
  
  tmr++;
}

bool get_token(void)
{
        WiFiClientSecure *STA_client = new  WiFiClientSecure; 
        bool flag=false;
        //JsonDocument doc;
        JsonDocument doc;
        const bool comment = false;
        String tmp;
        
    
    if(STA_client)
    {
          STA_client->setInsecure();
          
          HTTPClient https;
          tmp = https.begin(*STA_client, servergetToken);  //HTTP
          if(comment)
          {
          Serial.println("[HTTP] begin...\n");
          Serial.println(tmp);
      
          Serial.print("[HTTP] POST...\n");
          }
          https.addHeader("Content-Type", "application/json");
          int httpCode = https.POST( getToken_req);
          vTaskDelay(10);
      
          // httpCode will be negative on error
          if (httpCode > 0) {
            // HTTP header has been send and Server response header has been handled
            if(comment)
            Serial.printf("[HTTP] GET... code: %d\n", httpCode);
      
            // file found at server
            if (httpCode == HTTP_CODE_OK) {
              String payload = https.getString();
              Serial.print(payload);
              if(deserializeJson(doc, payload) == DeserializationError::Ok)
              {
                const char* p =   doc["token"];
                token = p;
                p = doc["login"];
                r_login = p;

                if(comment)
                Serial.println("Токен:");
               /* Serial.println(token);

                Serial.println("Логин:");
                Serial.println(r_login);
                */
               delete STA_client;
                return true;
              }else
              {
                if(comment)
                Serial.print("Десериализация отъехала...");
              }
              
              flag=true;
            }
          } else {
            if(comment)
            {
            Serial.printf("[HTTP] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
            Serial.println(httpCode);
            }
          }      
          https.end();
          doc.clear();
    }
  
    delete STA_client;

    return flag;

}



String http_request(String url,       //URL адрес
                    String body,      //Тело запроса
                    String type       //Тип запроса
                    )
{
          WiFiClientSecure *STA_client = new  WiFiClientSecure; 
        bool flag=false;
        String responce = "";
    
    if(STA_client)
    {
          STA_client->setInsecure();
          
          HTTPClient https;
      
          Serial.print("[HTTP] begin...\n");
          https.begin(*STA_client, url);  //HTTP
      
          Serial.print("[HTTP] POST...\n");
          https.addHeader("Content-Type", "application/json");
          int httpCode = 0;
          if(type == "POST")
          httpCode = https.POST(body);
          if(type == "GET")
          httpCode = https.GET();
      
          // httpCode will be negative on error
          if (httpCode > 0) {
            // HTTP header has been send and Server response header has been handled
            Serial.printf("[HTTP] GET... code: %d\n", httpCode);
      
            // file found at server
            if (httpCode == HTTP_CODE_OK) {
              responce = https.getString();
              
            }
          } else {
            Serial.printf("[HTTP] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
          }      
          https.end();
          return responce;
    }
  
    delete STA_client;
          return responce;    
}

JsonDocument get_card_list(void)
{
          WiFiClientSecure *STA_client = new  WiFiClientSecure; 
        bool flag=false;
        //JsonDocument doc;
        String str, tmp;
        JsonDocument doc;
        int i,j;
    
    if(STA_client)
    {
          STA_client->setInsecure();
          
          HTTPClient *https = new HTTPClient;

          //https.setTimeout(100);
          tmp = https->begin(*STA_client, servergetCardList);

          Serial.println("[HTTP] begin...\n");
          Serial.println(tmp);  //HTTP

          https->setAuthorization(token.c_str());
          https->setAuthorizationType("Bearer");    

          int httpCode = https->sendRequest("GET", str);
      
          // httpCode will be negative on error
          if (httpCode > 0) {
            // HTTP header has been send and Server response header has been handled
            Serial.printf("[HTTP] GET... code: %d\n", httpCode);
      
            // file found at server
            if (httpCode == HTTP_CODE_OK) {
              String payload = https->getString();
              Serial.print("Поступившие данные: ");
              Serial.println(payload);

              
              if(deserializeJson(doc, payload) == DeserializationError::Ok)
              {
                Serial.println("Количество элементов в массиве:");
                j=doc.size();
                Serial.println(j);
                /*
                for(i=0;i<j;i++)
                {
                  Serial.println(doc.operator[](i));
                }*/
                flag=true;
              }else
                Serial.print(httpCode);
            }
          } else {
            Serial.printf("[HTTP] GET... failed, error: %s\n", https->errorToString(httpCode).c_str());
          }      
          https->end();
          delete https;
          doc.clear();
    }

    STA_client->stop();
    delete STA_client;

    if(flag==false)
    doc.clear();
    return doc;

}
