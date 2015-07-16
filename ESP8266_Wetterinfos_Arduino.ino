#include <SoftwareSerial.h>
#include <ArduinoJson.h>

#define SSID "SSID" // Hier die SSID eingeben
#define PASS "PasswortAccessPoint" // Passwort vom Access Point
#define LOCATIONID "2911298" // Location ID für Hamburg, Deutschland
#define DST_IP "api.openweathermap.org" //api.openweathermap.org
SoftwareSerial esp8266Serial(10, 11); // ESP8266 Verbindung

void setup()
{
  
  // Ausgabe auf den seriellen Monitor
  Serial.begin(9600);
  
  // ESP8266 Verbindung
  esp8266Serial.begin(9600);
  esp8266Serial.setTimeout(5000);
  
  Serial.println("Sending reset to ESP8266 (AT+RST)");
  
  esp8266Serial.println("AT+RST"); // Reset ESP8266
  delay(1000);
   
  // Versuch Verbindungsaufbau AP
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
  Serial.println("Setze ESP8266 auf single connection mode (AT+CIPMUX=0)");
  esp8266Serial.println("AT+CIPMUX=0"); // Single Connecton Mode
}
void loop()
{
  String cmd = "AT+CIPSTART=\"TCP\",\"";
  cmd += DST_IP;
  cmd += "\",80";
  
  Serial.print("Stelle TCP-Verbindung auf Port 80 her: ");
  Serial.println(DST_IP);
  Serial.print(" (");
  Serial.print(cmd);
  Serial.println(")");
    
  esp8266Serial.println(cmd);
  
  if(esp8266Serial.find("Error")) return;
  cmd = "GET /data/2.5/weather?id=";
  cmd += LOCATIONID;
  cmd += " HTTP/1.0\r\nHost: api.openweathermap.org\r\n\r\n";
  
  Serial.print("Sende Datenlänge (AT+CIPSEND=");
  Serial.print(cmd.length());
  Serial.println(")");
  
  esp8266Serial.print("AT+CIPSEND=");
  esp8266Serial.println(cmd.length());
  if(esp8266Serial.find(">")){
    Serial.println("Bereit HTTP-Request zu senden");
  }else{
    esp8266Serial.println("AT+CIPCLOSE");
    Serial.println("Vernindungs-Timeout. Sende AT+CIPCLOSE");
    delay(1000);
    return;
  }
  Serial.println("Sende HTTP-Request: >");
  Serial.println(cmd);
  
  
  esp8266Serial.print(cmd);
  unsigned int i = 0; //Timeout Zähler
  int n = 1; // Character Zähler
  char json[100]="{";
  while (!esp8266Serial.find("\"main\":{")){}
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
  Serial.println("Erhaltene Web-Daten (JSON-formatted): ");
  Serial.println(json);
  
  StaticJsonBuffer<100> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(json);
  
  double temp = root["temp"];
  int humidity = root["humidity"];
  int pressure = root["pressure"];
  temp -= 273.15; // Umrechnung Kelvin in Grad Celsius
  
  Serial.print("Parsing JSON data");
  Serial.println();
  
  Serial.print("Temperatur: ");
  Serial.println(temp);
  Serial.print("Druck: ");
  Serial.println(pressure);
  Serial.print("Feuchtigkeit: ");
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
  Serial.print("Verbindung mit SSID: ");
  Serial.print(SSID);
  Serial.println("(AT+CWJAP=<SSID>,<PASSWORD>)");
  
  delay(2000);
  if(esp8266Serial.find("OK")){
    Serial.println("Verbindung hergestellt");
    return true;
  }else{
    Serial.println("Kann nicht mit dem Access Point verbinden");
    return false;
  }
}
