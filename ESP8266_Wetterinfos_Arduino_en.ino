#include <SoftwareSerial.h>
#include <ArduinoJson.h>

#define SSID "SSID" // SSID
#define PASS "AP_Passwort" // AP-password
#define LOCATIONID "2911298" // location id for Hamburg, Germany
#define DST_IP "api.openweathermap.org" //api.openweathermap.org
SoftwareSerial esp8266Serial(10, 11); // ESP8266 connection

void setup()
{
  
  // output on Serial Monitor
  Serial.begin(9600);
  
  // ESP8266 connection
  esp8266Serial.begin(9600);
  esp8266Serial.setTimeout(5000);
  
  Serial.println("Sending reset to ESP8266 (AT+RST)");
  
  esp8266Serial.println("AT+RST"); // reset ESP8266
  delay(1000);
  
  /*if(esp8266Serial.find("ready")) {
    Serial.println("ESP8266 module ready");
  }else{
    Serial.println("ESP8266 module error");
    while(1);
  }
  delay(1000);*/
  
  // try to connect to wifi AP
  boolean connected=false;
  for(int i=0;i<5;i++){
    if(connectWiFi()){
      connected = true;
        break;
    }
  }
  if (!connected){
    while(1);
  }
  delay(5000);
  Serial.println("Setting ESP8266 to single connection mode (AT+CIPMUX=0)");
  esp8266Serial.println("AT+CIPMUX=0"); // set to single connection mode
}
void loop()
{
  String cmd = "AT+CIPSTART=\"TCP\",\"";
  cmd += DST_IP;
  cmd += "\",80";
  
  Serial.print("Establishing TCP-connection on port 80 with: ");
  Serial.println(DST_IP);
  Serial.print(" (");
  Serial.print(cmd);
  Serial.println(")");
    
  esp8266Serial.println(cmd);
  
  if(esp8266Serial.find("Error")) return;
  cmd = "GET /data/2.5/weather?id=";
  cmd += LOCATIONID;
  cmd += " HTTP/1.0\r\nHost: api.openweathermap.org\r\n\r\n";
  
  Serial.print("Sending data length (AT+CIPSEND=");
  Serial.print(cmd.length());
  Serial.println(")");
  
  esp8266Serial.print("AT+CIPSEND=");
  esp8266Serial.println(cmd.length());
  if(esp8266Serial.find(">")){
    Serial.println("Ready to send HTTP-Request");
  }else{
    esp8266Serial.println("AT+CIPCLOSE");
    Serial.println("Connection timeout. Sent AT+CIPCLOSE");
    delay(1000);
    return;
  }
  Serial.println("Sending HTTP-Request: >");
  Serial.println(cmd);
  
  
  esp8266Serial.print(cmd);
  unsigned int i = 0; //timeout counter
  int n = 1; // char counter
  char json[100]="{";
  while (!esp8266Serial.find("\"main\":{")){} // find the part we are interested in.
  while (i<60000) {
    if(esp8266Serial.available()) {
      char c = esp8266Serial.read();
      json[n]=c;
      if(c=='}') break;
      n++;
      i=0;
    }
    i++;
  }
  Serial.println("Received web data (JSON-formatted): ");
  Serial.println(json);
  
  StaticJsonBuffer<100> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(json);
  
  double temp = root["temp"];
  int humidity = root["humidity"];
  int pressure = root["pressure"];
  temp -= 273.15; // from kelvin to degree celsius
  
  Serial.print("Parsing JSON data");
  Serial.println();
  
  Serial.print("Temperature: ");
  Serial.println(temp);
  Serial.print("Pressure: ");
  Serial.println(pressure);
  Serial.print("Humidity: ");
  Serial.println(humidity);
  Serial.println();
  
  delay(300000);

}
     
boolean connectWiFi()
{
  esp8266Serial.println("AT+CWMODE=1");
  String cmd="AT+CWJAP=\"";
  cmd+=SSID;
  cmd+="\",\"";
  cmd+=PASS;
  cmd+="\"";
  esp8266Serial.println(cmd);
  Serial.print("Connecting to SSID: ");
  Serial.print(SSID);
  Serial.println("(AT+CWJAP=<SSID>,<PASSWORD>)");
  
  delay(2000);
  if(esp8266Serial.find("OK")){
    Serial.println("Connection established");
    return true;
  }else{
    Serial.println("Can not connect to AP");
    return false;
  }
}
