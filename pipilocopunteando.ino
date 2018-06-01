

///////////////////////////////RFRID

// Wireless RFID Door Lock Using NodeMCU
// Created by LUIS SANTOS & RICARDO VEIGA
// 7th of June, 2017


#include <Wire.h>
//#include "SSD1306.h"
#include <MFRC522.h>
#include <SPI.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>


#define RST_PIN 15 // RST-PIN for RC522 - RFID - SPI - Module GPIO15 
#define SS_PIN  2  // SDA-PIN for RC522 - RFID - SPI - Module GPIO2
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance

String nueva= "";
char ssid[] = "MENDEZ WIFI";     // your network SSID (name)
char password[] = "79539360"; // your network key
const char* mqtt_server = "broker.hivemq.com";          //Direccion/dominio del servidor MQTT.
const int mqttPort = 1883;
const char* mqttUser = "ValdInt";

WiFiClient espClient;
PubSubClient client(espClient);
//------------------------------------------------------------------------------------

void usr(){
  Serial.println("USR");
  digitalWrite(5,HIGH);
  digitalWrite(0,HIGH);
  digitalWrite(16,LOW); 
  delay (2000);
  digitalWrite(0,LOW);      
  digitalWrite(5,LOW);
  }
//------------------------------------------------------------------------------------

void incorrecta(){
     Serial.println("Incorrecta");
      digitalWrite(16,LOW);
      digitalWrite(4,HIGH);
      delay (2000);
      digitalWrite(4,LOW); 
} 
//------------------------------------------------------------------------------------
char* leerNueva(){
     Serial.println("Creador");
     delay(2000);
     char* nueva=readCard();
     while(nueva == "null"){
        nueva=readCard();
        Serial.print(".");
        delay(500);
     }

    Serial.println("Creada nuevo USR");
    Serial.println(nueva);
    delay(2000);
    return nueva;
}          
//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------

void reconnect() {                                                    //Funcion que en caso de que se pierda la conexion al servidor MQTT reintenta la vinculacion
 while (!client.connected()) 
 {
     Serial.println("ESPERANDO MQTT CONEXION");
     if (client.connect("ESP8266Client")){
         Serial.println("connected");
         client.publish("RegisRes","avalible");
         client.publish("Validate","avalible"); 
         client.subscribe("registrar");
         client.subscribe("ValidateRes");
     } 
     else{
         Serial.print(client.state());
         delay(500)
         ;
     }
 }
}
//------------------------------------------------------------------------------------
 void callback(char* topic, byte* payload, unsigned int length) {      //Funcion que se ejecuta cada que llega un mensaje y lo que hace es imprimir su contenido
  Serial.print("Message arrived [");
   Serial.print(topic);                                                               
   Serial.print("] ");
   char paychar[64];
   for (int i = 0; i < length; i++) {
     Serial.print((char)payload[i]);
     paychar[i]=((char)payload[i]);
   }
   paychar[length]='\0';
    if(strcmp(topic, "registrar") == 0){
      Serial.println("ha llegado algo de registrar");
      delay(10000);
      Serial.println(paychar);
      char * val=paychar;
      if(paychar != "null"){
         Serial.println(paychar);
         char* nueva =leerNueva();
         if (!client.connected()) {
           reconnect();
         }
         client.loop();
         client.publish("RegisRes",nueva); 
      }
    }
    if(strcmp(topic, "ValidateRes") == 0){
      Serial.println("ha llegado algo de ValidateRes");
      Serial.println(paychar);
      delay(10000);
      char * val=paychar;
      if(paychar == "TRUE"){
         Serial.println(paychar);
         usr();
      }else if(paychar == "FALSE"){
         Serial.println(paychar);
         incorrecta();
      }
    }
    
}
//----------------------------------------------------------------------------------------
char* readCard(){
  if ( ! mfrc522.PICC_IsNewCardPresent()) {   
    delay(50);
     digitalWrite(16,LOW);
    return "null";
  }
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {   
    delay(50);
    digitalWrite(16,LOW);
    return "null";
  }
////-------------------------------------------------RFID----------------------------------------------
  // Shows the card ID on the serial console
 String content= "";
  for (byte i = 0; i < mfrc522.uid.size; i++){
     content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : ","));
     content.concat(String(mfrc522.uid.uidByte[i]));
  }
  Serial.println();
  content.toUpperCase();
  Serial.println("Cart read:" + content);
  digitalWrite(16,HIGH);
  delay(100);
  char copy [20];
  content.toCharArray(copy, 20);
  return copy;
}



//------------------------------------------------------------------------------------

void validar(char* nuevo){
   if (!client.connected()) {
   reconnect();
 }
 client.loop();
  Serial.println(" client.publish(Validate,nuevo);");
  client.publish("Validate",nuevo);
  delay(5000);
}

//------------------------------------------------------------------------------------
void setup() {

  pinMode(5, OUTPUT);
  digitalWrite(5,LOW);
  pinMode(0, OUTPUT);
  digitalWrite(0,LOW);  
  pinMode(4, OUTPUT);
  digitalWrite(4,LOW);
  pinMode(16, OUTPUT);
  digitalWrite(16,HIGH);  
  Serial.begin(115200);    // Initialize serial communications
  SPI.begin();           // Init SPI bus
  mfrc522.PCD_Init();    // Init MFRC522
  delay(3000);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  // attempt to connect to Wifi network:
  Serial.println("--");
  Serial.print("Connecting Wifi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  client.setServer(mqtt_server, mqttPort);
  client.setCallback(callback);   
  delay(100);
  if (!client.connected()) {
     reconnect();
   }
   client.loop();
}


void loop() {

  int authorized_flag = 0;
  if (!client.connected()) {
       reconnect();
     }
     client.loop();

  char* act=readCard();         
  if(act == "null"){
    Serial.println("Nada");
    digitalWrite(16,HIGH);   
  }
  else{
    validar(act);
  }
}
    


