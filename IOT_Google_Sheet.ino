#include <ESP8266WiFi.h>
#include <MFRC522.h>
#include <Servo.h>
#include <HTTPSRedirect.h>
#include <SPI.h>
#include <PubSubClient.h>
#include <WiFiManager.h>
#define SS_PIN 2
#define RST_PIN 0
#define LED 5 //D1
#define LED2 16 //D0
int analogPin = A0;
int val;
#define UPDATE_INTERVAL_HOUR  (0)
#define UPDATE_INTERVAL_MIN   (0)
#define UPDATE_INTERVAL_SEC   (30)
#define UPDATE_INTERVAL_MS    ( ((UPDATE_INTERVAL_HOUR*60*60) + (UPDATE_INTERVAL_MIN * 60) + UPDATE_INTERVAL_SEC ) * 1000 )


MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;  
MFRC522::StatusCode status;
Servo myServo;
String Name = "Rathanan";


const char* ssid     = "JumboPlus_DormIoT";
const char* password = "rathanan027";
String GAS_ID = "AKfycbzY5xx97OtgRWZG0XDeasfcChxKmKlXFq0VGJfuoVGmEDDleL_S78Zu--VHEefD2Oi8Fg";
const char* host = "script.google.com"; 
const char* mqtt_server = "broker.netpie.io";
const int mqtt_port = 1883;
const char* mqtt_Client = "34e8cb13-350a-46a6-859d-0d330d36959b";
const char* mqtt_username = "j7PNENLxxzHeuorSd8WxmSWVgCJMb6ge";
const char* mqtt_password = "TQFv3PcyVRbVD6NWvvbJYiMp3JFrkadm";


WiFiClient espClient;
PubSubClient client(espClient);

char msg[50];
long lastMsg = 0;
int value = 0;

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection…");
    if (client.connect(mqtt_Client, mqtt_username, mqtt_password)) {
      Serial.println();
      Serial.println("connected");
      client.subscribe("@msg/LED");
    }
    else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println("try again in 5 seconds");
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  String message;
  String tpc;
  for (int i = 0; i < length; i++) {
    message = message + (char)payload[i];
  }
  Serial.println(message);
  if(String(topic) == "@msg/LED") {
    if (message == "ON"){
      digitalWrite(LED,1);
      client.publish("@shadow/data/update", "{\"data\" : {\"LED\" : \"ON\"}}");
      Serial.println("LED ON");
    }
    else if (message == "OFF") {
      digitalWrite(LED,0);
      client.publish("@shadow/data/update", "{\"data\" : {\"LED\" : \"OFF\"}}");
      Serial.println("LED OFF");
    }
    else if (message == "OPEN"){
      myServo.write(180);
      client.publish("@shadow/data/update", "{\"data\" : {\"DOOR\" : \"OPEN\"}}");
      Serial.println("DOOR OPEN");
    }
    else if (message == "CLOSE") {
      myServo.write(45);
      client.publish("@shadow/data/update", "{\"data\" : {\"DOOR\" : \"CLOSE\"}}");
      Serial.println("DOOR CLOSE");
    }
    }
  }

void update_google_sheet()
{
    Serial.print("connecting to ");
    Serial.println(host);
  
       WiFiClientSecure client;
    const int httpPort = 443; 
    
    client.setInsecure(); 
    
    if (!client.connect(host, httpPort)) { //works!
      Serial.println("connection failed");
      return;
    }
       
    String url = "/macros/s/" + GAS_ID + "/exec?Name=";
   
    url += String(Name);
    

    
    Serial.print("Requesting URL: ");
    Serial.println(url);
  

    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" + 
                 "Connection: close\r\n\r\n");
  
    Serial.println();
    Serial.println("closing connection");
    Serial.println();  
}

void setup()
{
  pinMode(LED,OUTPUT);
  pinMode(LED2,OUTPUT);
  Serial.begin(115200);
  Serial.println('\n');
  SPI.begin();
  myServo.attach(4);
  myServo.write(45);
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  Serial.println();

  WiFi.begin(ssid, password );
  Serial.print("Connecting to ");
  Serial.print(ssid);

  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println('\n');
  Serial.println("WiFi Connected!");
  Serial.println(WiFi.localIP());
  mfrc522.PCD_Init();
  Serial.println("Put your card to the reader...");

}

unsigned long time_ms;
unsigned long time_1000_ms_buf;
unsigned long time_sheet_update_buf;
unsigned long time_dif;

void loop()
{  
  val = analogRead(analogPin);


  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if (val < 100) {
    digitalWrite(LED2, HIGH);
    Serial.println("LED ON");
    Serial.println();
  }
  else {
    digitalWrite(LED2, LOW);
    Serial.println("LED OFF");
    Serial.println();
  }
  delay(500);
  
  time_ms = millis();
  time_dif = time_ms - time_1000_ms_buf;

  if ( time_dif >= 1000 ) 
  {
    time_1000_ms_buf = time_ms;

    if(! mfrc522.PICC_IsNewCardPresent())
    {
      return;
    }
    if(! mfrc522.PICC_ReadCardSerial())
    {
      return;
    }
    Serial.print("UID tag :");
    String content= "";
    byte letter;
    for (byte i =0; i < mfrc522.uid.size; i++)
    {
      Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? "0" : " ");
      Serial.print(mfrc522.uid.uidByte[i], HEX);
      content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : " "));
      content.concat(String(mfrc522.uid.uidByte[i], HEX));
    }
    Serial.println();
    Serial.print("Message : ");
    content.toUpperCase();
    if (content.substring(1) == "020A 92 85")
    {
      Serial.println("Authorized access");
      Serial.println();
      delay(500);

    }

    else {
      Serial.println("Access denied");
      delay(100);
      myServo.write(180);
      delay(3000);
      myServo.write(45);
      update_google_sheet();
      delay(2000);

    }
  }


}
void onAutoConnWifi(String chk){
   WiFiManager wifiManager;
   if(chk=="reset"){
    wifiManager.resetSettings();
    Serial.println("reset wifi");
   }else{
    wifiManager.autoConnect("LED_Status"); // ชื่อ Access point เริ่มต้น
    Serial.println(WiFi.localIP());
   }
}
 
