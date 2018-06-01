#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include "DHT.h"
#include <UniversalTelegramBot.h>

#define DHTPIN 4 //D2
#define DHTTYPE DHT11

const char* ssid = "MENDEZ WIFI";
const char* password = "79539360";
const char* server = "IoTHubESP8266.azure-devices.net";
const char* deviceID = "ESP8266-Sensores";
const char* hubUser = "IoTHubESP8266.azure-devices.net/ESP8266-Sensores";
const char* sasKey = "SharedAccessSignature sr=IoTHubESP8266.azure-devices.net%2Fdevices%2FESP8266-Sensores&sig=%2FYRHtYZ3zo92J1g8962co9%2FyqcA59lGmoqTdfmVnCWg%3D&se=1559284558";
const char* topic = "devices/ESP8266-Sensores/messages/events/";

#define BOTtoken "543265089:AAH7ss6IwBDzqGpfKZiF76J-InVJpXrOx4E"
const char* mqtt_server = "broker.hivemq.com";          //Direccion/dominio del servidor MQTT.
const int mqttPort = 1883;
const char* mqttUser = "Azure"; 

WiFiClientSecure espClient;
PubSubClient client(espClient); 
DHT dht(DHTPIN, DHTTYPE);
UniversalTelegramBot botAll(BOTtoken, espClient);

long lastMsg = 0;
byte pir = 5; //D1
byte relay = 14; //D5
int valuePir;
int magnetPin = 0; //A0
int ledPin = 13;
int gasPin = 0; //D3
int limitGas;
float GAUSS_PER_STEP = 2.713;

float rawValue = 0.0;
float value = 0.0;
float zeroLevel = 537.0;
int statusAlam = 0;

void setup_wifi(){
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
void reconnectMQTT() {                                                    //Funcion que en caso de que se pierda la conexion al servidor MQTT reintenta la vinculacion
 while (!client.connected()) {
  if (client.connect("ESP8266Client")) {
         Serial.println("connected");
         client.subscribe("Azure2");
     } 
     else {
         Serial.print(client.state());
         delay(5000);
     }
   }
}
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
    if(strcmp(topic, "Azure2") == 0){
      if(strcmp(paychar, "status") == 0){
           Serial.println(paychar);
           String chat_id = String(botAll.messages[botAll.getUpdates(botAll.last_message_received + 1)].chat_id);
           String msgS = "[{\"sensor\":\"Humedad\",\"value\":\"" + String(dht.readHumidity()) +"\"}," + 
                   "{\"sensor\":\"Temperatura\",\"value\":\"" + String(dht.readTemperature()) +"\"}," +
                   "{\"sensor\":\"Alarma\",\"value\":\"" + String(statusAlam) +"\"}]";
           botAll.sendMessage(chat_id, msgS, "");
        }else if(strcmp(topic, "apagarAlarma") == 0){
            Serial.println("apagando alarma");
            statusAlam = 0;
            digitalWrite(relay, HIGH);
        }
    }
}
void MQTT(){
  client.disconnect();
  client.setServer(mqtt_server, mqttPort);   //Vinculacion con datos del servidor mqtt.
  client.setCallback(callback); 
  if (!client.connected()) {
   reconnectMQTT();
 }
 
}


void reconnect(){
  while(!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    
    // Intenta conectar
    if (client.connect(deviceID, hubUser, sasKey))
      Serial.println("connected");
    else{
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void message(){
  long now = millis();
  String msgS = "[{\"sensor\":\"Humedad\",\"value\":\"" + String(dht.readHumidity()) +"\"}," + 
                 "{\"sensor\":\"Temperatura\",\"value\":\"" + String(dht.readTemperature()) +"\"}," +
                 "{\"sensor\":\"Alarma\",\"value\":\"" + String(statusAlam) +"\"}]";
  char msg[msgS.length()+1];
  
  msgS.toCharArray(msg, msgS.length()+1);
  
  if (now - lastMsg > 5000){
    lastMsg = now;
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish(topic, msg);
  }
}

void setup(){
  pinMode(pir, INPUT);
  pinMode(relay, OUTPUT);
  digitalWrite(relay, LOW);
  pinMode (magnetPin, INPUT);
  dht.begin();
  pinMode(gasPin, INPUT);
  Serial.begin(115200);

  setup_wifi();
  client.setServer(server, 8883);
}
void alarma(){
    Serial.println("ON");
    statusAlam = 1;
    digitalWrite(relay, LOW);
}
void loop(){
  if(!client.connected())
    reconnect();
    
  valuePir = digitalRead(pir);
  rawValue = analogRead (magnetPin) - zeroLevel;
  limitGas= digitalRead(gasPin);
  // Obtiene la Humedad
  float hum = dht.readHumidity();
  // Obtiene la Temperatura en Celsius
  float temp = dht.readTemperature();
  
  // Reading positive relative to the South Pole, the North Pole negative
  value = rawValue * GAUSS_PER_STEP;
  Serial.println (value);
  Serial.println ("TEMP: ");
  Serial.println (temp);
  String chat_id = String(botAll.messages[botAll.getUpdates(botAll.last_message_received + 1)].chat_id);
  if(  valuePir == HIGH){
    alarma();
    botAll.sendMessage(chat_id, "El sensor de movimiento ha detectado algo ", "");
  }else if(value < 700){
    alarma();
    botAll.sendMessage(chat_id, "Se ha abierto la _____ ", "");
  }else if(temp > 35){
    alarma();
    String msg="La temperatura ha subido a ";
    msg+=temp;
    msg+="°C\n";
    botAll.sendMessage(chat_id,msg, "");
  }else  if( limitGas == LOW){
    alarma();
    botAll.sendMessage(chat_id, "Se ha detectado una fuga de gas", "");
  }
 
  message();
  MQTT();
  delay(1000);
}
