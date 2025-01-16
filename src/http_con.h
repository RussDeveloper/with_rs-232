#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>

//IP =  95.31.39.61:51112 - ип сервера стекломаш

//Your Domain name with URL path or IP address with path
String servergetToken("https://api.steklm.ru:51112/api/auth/getToken");
//String servergetCardList("https://api.steklm.ru:51112/api/card/ ");
String servergetCardList("https://api.steklm.ru:51112/api/Card?mac=3C-84-27-20-F5-B4");
//String user_action("http://192.168.0.26:1322/api/log/");
String user_action("https://api.steklm.ru:51112/api/log/");
//String servergetCardList("https://api.steklm.ru:51112/api/card/getCard");

String getToken_req("{\"Login\": \"3C-84-27-20-F5-B4\",\"Password\": \"ChkalovTest\"}"); //3C:84:27:20:F5:B4
//String body_req("{\"Login\": \"3C-84-27-20-F5-B4\",\"Password\": \"ChkalovTest\"}"); //3C:84:27:20:F5:C8
//String getToken_req1("{\n\"Login\": \"test\",\n\"Password\": \"test\"\n}");
byte server1[] = { 192, 168, 0, 26 };

String token, r_login, s_action;
JsonDocument card_list,       //Список карт 
              action,
              doc;

extern String  wifi_ssid,      //Set Your SSID
        wifi_parol,     //Set Your Passwor
        parol_serv,
        login_serv,
        token_serv
        ;

bool get_token(void);
JsonDocument get_card_list(void);
JsonDocument set_cust_action(String);
int box_action(JsonDocument);
int box_action_s(String);
extern void card_list_compare(JsonDocument);

String http_request(String, String, String);
void card_list_compare_old(JsonDocument);


void http_master()
{
  static unsigned long tmr = 5000, t_val=0;
  String val = "" ,header = "", data = "";
  int i,j,k,t;
  //Сериализация - из JSON в файл/строку, функцию
  //Десериализация - из файла/строки в JSON
  if(tmr>5000)
  {
    if(get_token())
    {
      if(token.length()>0)
      {
        card_list.clear();
        card_list = get_card_list();
        card_list_compare(card_list);
      }
    }
    tmr=0;
    /**/
    }
    if(token.length()>0)
    {
      /*
      if(!action.isNull())
      {
        Serial.println("Запрос начат. Код результата: ");
        Serial.println(box_action(action));
        serializeJson(action, Serial);
        action.clear();
      }*/
      if(!s_action.isEmpty())
      {
        Serial.println("Запрос начат. Код результата: ");
        Serial.println(box_action_s(s_action), HEX);
        s_action.clear();
      }
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
          //"{\"Login\": \"3C-84-27-20-F5-B4\",\"Password\": \"ChkalovTest\"}"
          if((login_serv.isEmpty())||(parol_serv.isEmpty()))
            Serial.println("Логин/Пароль сервера отсутствует");
          String str_getToken("{\"Login\": \"");
          str_getToken+=login_serv ;
          str_getToken+="\" ,\"Password\": \"";
          str_getToken+=parol_serv;
          str_getToken+="\"}";
          Serial.print("str_getToken: ");
          Serial.println(str_getToken);
          Serial.print("parol_serv: ");
          Serial.println(parol_serv);
          //int httpCode = https.POST( getToken_req);
          int httpCode = https.POST(parol_serv);
          vTaskDelay(10);
      
          // httpCode will be negative on error
          if (httpCode > 0) {
            // HTTP header has been send and Server response header has been handled
            if(comment)
            Serial.printf("[HTTP] GET... code: %d\n", httpCode);
      
            // file found at server
            if (httpCode == HTTP_CODE_OK) {
              String payload = https.getString();
              if(comment)
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
               STA_client->stop();
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
            STA_client->stop();
    delete  STA_client;

    return flag;

}

String http_request(String url,       //URL адрес
                    String body,      //Тело запроса
                    String type       //Тип запроса
                    )
{
          WiFiClientSecure *STA_client = new  WiFiClientSecure; 
        bool flag=false, log = false;
        String responce = "";
    
    if(STA_client)
    {
          STA_client->setInsecure();
          
          HTTPClient https;
          if(log)
          Serial.print("[HTTP] begin...\n");
          https.begin(*STA_client, url);  //HTTP

          if(log)
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
            if(log)
            Serial.printf("[HTTP] GET... code: %d\n", httpCode);
      
            // file found at server
            if (httpCode == HTTP_CODE_OK) {
              responce = https.getString();
              
            }
          } else {
            Serial.printf("[HTTP] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
          }      
          https.end();
          STA_client->stop();
          return responce;
    }
    STA_client->stop();
    delete STA_client;
    return responce;    
}

JsonDocument get_card_list(void)
{
    WiFiClientSecure *STA_client = new  WiFiClientSecure; 
    bool flag=false, log = false;
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
          if(log)
          Serial.println("[HTTP] begin...\n");
          if(log)
          Serial.println(tmp);  //HTTP

          https->setAuthorization(token.c_str());
          https->setAuthorizationType("Bearer");    

          int httpCode = https->sendRequest("GET", str);
      
          // httpCode will be negative on error
          if (httpCode > 0) {
            // HTTP header has been send and Server response header has been handled
            if(log)
            Serial.printf("[HTTP] GET... code: %d\n", httpCode);
      
            // file found at server
            if (httpCode == HTTP_CODE_OK) {
              String payload = https->getString();
              if(log){
              Serial.print("Поступившие данные: ");
              Serial.println(payload);}
              
              if(deserializeJson(doc, payload) == DeserializationError::Ok)
              {
                if(log){
                Serial.println("Количество элементов в массиве:");
                j=doc.size();
                Serial.println(j);}
                flag=true;
              }else
              { if(log)
                Serial.print(httpCode);}
            }
          } else {
            if(log)
            Serial.printf("[HTTP] GET... failed, error: %s\n", https->errorToString(httpCode).c_str());
          }      
          https->end();
          delete https;
          //doc.clear();
    }

    STA_client->stop();
    delete STA_client;

    if(flag==false)
      doc.clear();
    return doc;
}

int box_action(JsonDocument act)
{
            WiFiClientSecure *STA_client = new  WiFiClientSecure; 
        bool tmp;
        int httpCode;

    if(STA_client)
    {
          STA_client->setInsecure();
          
          HTTPClient *https = new HTTPClient;

          //https.setTimeout(100);
          tmp = https->begin(*STA_client, user_action);

          Serial.println("[HTTP] begin...\n");
          Serial.println(tmp);  //HTTP

          if(token.isEmpty())
          {
            Serial.println("Токен не получен");
           https->end();
          delete https;
          STA_client->stop();
          delete STA_client;            
          }

          https->setAuthorization(token.c_str());
          https->setAuthorizationType("Bearer");    
          String st = act.as<String>();
          httpCode = https->sendRequest("POST", st);
      
          // httpCode will be negative on error
          if (httpCode > 0) {
            // HTTP header has been send and Server response header has been handled
            Serial.printf("[HTTP] GET... code: %d\n", httpCode);

          //doc.clear();
          }else{
            Serial.println("HTTP not response, code:");
            Serial.println(httpCode);
            }

           https->end();
          delete https;

    STA_client->stop();
    delete STA_client;
}
    if(httpCode == HTTP_CODE_OK)
      return 0;
      else return 1;
}

int box_action_s(String  act)
{
            WiFiClientSecure *STA_client = new  WiFiClientSecure; 
        bool tmp;
        int httpCode;

    if(STA_client)
    {
          STA_client->setInsecure();
          
          HTTPClient *https = new HTTPClient;

          //https.setTimeout(100);
          tmp = https->begin(*STA_client, user_action);

          Serial.println("[HTTP] begin...\n");
          Serial.println(tmp, DEC);  //HTTP

          if(token.isEmpty())
          {
            Serial.println("Токен не получен");
           https->end();
          delete https;
          STA_client->stop();
          delete STA_client;            
          }

          https->setAuthorization(token.c_str());
          https->setAuthorizationType("Bearer");   
          https->addHeader("Content-Type", "application/json"); 
          //String st = act.as<String>();
          //httpCode = https->sendRequest("POST", st);
          httpCode = https->sendRequest("POST", act);
      
          // httpCode will be negative on error
          if (httpCode > 0) {
            // HTTP header has been send and Server response header has been handled
            Serial.printf("[HTTP] GET... code: %d\n", httpCode);

          //doc.clear();
          }else{
            Serial.println("HTTP not response, code:");
            Serial.println(httpCode, DEC);
            }

           https->end();
          delete https;

    STA_client->stop();
    delete STA_client;
}
    if(httpCode == HTTP_CODE_OK)
      return 0;
      else return 1;
}

void card_list_compare_old(JsonDocument doc1)
{
  String val;
  int i;

        if(!doc1.isNull())
      {
        i = serializeJson(doc1, val);

        if(i)              //Получение списка карт
          {
           File u_list = SPIFFS.open("/list_users.txt");
            DeserializationError error = deserializeJson(doc, u_list);
            if(error!= DeserializationError::Ok)
              Serial.println("Ошибка десериализации файла из флешь памяти");
            if(doc.as<String>()!=doc1.as<String>())  //Если присланный список и сохраненный не совпадают - перезаписать
            {
              u_list.close();
              SPIFFS.remove("/list_users.txt");
              u_list = SPIFFS.open("/list_users.txt","w",true);
              serializeJson(doc1, u_list);
              u_list.close();
              u_list = SPIFFS.open("/list_users.txt");
              doc.clear();
              deserializeJson(doc, u_list);
              Serial.println("Сохраненный список : ");
              Serial.println(doc.as<String>());              
            }else
              Serial.println("Списки совпадают");
      }else
      Serial.println("Запрос на карты не верен");
    }

}









