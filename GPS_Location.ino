#include <SoftwareSerial.h>
#include <NMEA0183.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#ifndef STASSID
#define STASSID "********"   //Wifi SSID
#define STAPSK  "********"   //Wifi Password
#endif

SoftwareSerial ss(2,0);      //(RX,TX)
NMEA0183 nmea;

void setup()
{
  pinMode(2, OUTPUT);
  Serial.begin(9600);
  Serial.println("Testing");
  WiFi.begin(STASSID, STAPSK); 
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }
  ss.begin(9600);
}
void loop()
{
  delay(100);
  if ((WiFi.status() == WL_CONNECTED))   //Wifi successfully connected
  {
    while (ss.available() > 0)           //GPS>0 start running
    {
      char c = ss.read();                //Every row of GPS's data //GPS每一行讀進來的數據
      if (nmea.update(c))                //Update latest data //更新現在最新一行的數據
      {
        if(nmea.getFields()==12)                 //NMEA格式的數據中，我只要GNRMC這行，找到它的Fields==12，這樣每次就只會輸出GNRMC的資料
        {                                        //I just want GNRMC this line, and its Fields=12
          const char* str  = nmea.getSentence(); //Change NMEA to string //NMEA得到的數據不是字串，所以把它改成字串，方便之後webservice作業
          Serial.print(nmea.getSentence());
          Serial.println();
          
          HTTPClient http;
          //The following sample is using 'POST' to call webservice
          //以下為 用http post呼叫webservice的範例 也可以自由選擇要用什麼方式去呼叫webservice
          http.begin("http://***.***.**.**/WebService/WebService.asmx/insert_gpslocation");   //Your IP Adress and path //用http post的方式呼叫webservice，直接指定到要執行的位置
          http.addHeader("Content-Type", "application/x-www-form-urlencoded");                //http post 它的Content-Type為application/x-www-form-urlencoded (選擇用什麼方式呼叫就寫它規定的Content-Type)
          String a1=String("GNRMC=IM-235-TD001");          //http post執行要求的語法
          int httpCode = http.POST(a1+str);                //http.POST(真正呼叫webservice要它處理的值)(vice.asmx/insert_gpslocation?GNRMC=IM-235-TD001$GNRMC,,,,,)
          String payload = http.getString();               //webservice處理完給的回應
          Serial.println(httpCode);                        //Http code 200 is successul //Http執行結果後回傳的代碼，可以找錯誤，ex:200成功
          Serial.println(payload);                         //webservice execution result //回傳webservice執行後的結果(內容描述)
          if(httpCode==200)
          { delay(100); }
          http.end();
          delay(30000);                                    //Late 30sec to reupdate the value //這一次的http呼叫結束，準備進行下一次呼叫重新傳值
        }  
      }
    }
  }
}
