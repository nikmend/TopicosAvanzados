/*******************************************************************
 *  An example of bot that receives commands and turns on and off  *
 *  an LED.                                                        *
 *                                                                 *
 *  written by Giacarlo Bacchio (Gianbacchio on Github)            *
 *  adapted by Brian Lough                                         *
 *******************************************************************/
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <PubSubClient.h>
// Initialize Wifi connection to the router
char ssid[] = "ZTE";     // your network SSID (name)
char password[] = "aa11bb22"; // your network key
const char* mqtt_server = "192.168.137.1";          //Direccion/dominio del servidor MQTT.
const int mqttPort = 1883;
const char* mqttUser = "telegram"; 

// Initialize Telegram BOT
#define BOTtoken "543265089:AAH7ss6IwBDzqGpfKZiF76J-InVJpXrOx4E"  // your Bot Token (Get from Botfather)

WiFiClientSecure clientTel;
UniversalTelegramBot bot(BOTtoken, clientTel);
WiFiClient espClient;
PubSubClient client(espClient);

int Bot_mtbs = 1000; //mean time between scan messages
long Bot_lasttime;   //last time messages' scan has been done
bool Start = false;

const int ledPin = 16;
int ledStatus = 0;
void registrar(){
   if (!client.connected()) {
   reconnect();
 }
 client.loop();
  Serial.println(" client.publish(registrar,nuevo);");
  client.publish("registrar","nuevo");
}
void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i=0; i<numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;

    String from_name = bot.messages[i].from_name;
    if (from_name == "") from_name = "Guest";

    if (text == "/ledon") {
      digitalWrite(ledPin, HIGH);   // turn the LED on (HIGH is the voltage level)
      ledStatus = 1;
      bot.sendMessage(chat_id, "Led is ON", "");
    }

    if (text == "/ledoff") {
      ledStatus = 0;
      digitalWrite(ledPin, LOW);    // turn the LED off (LOW is the voltage level)
      bot.sendMessage(chat_id, "Led is OFF", "");
    }
  if (text == "/registry") {
      registrar();
      bot.sendMessage(chat_id, "Nuevo registro", "");
    }

    if (text == "/status") {
      if(ledStatus){
        bot.sendMessage(chat_id, "Led is ON", "");
      } else {
        bot.sendMessage(chat_id, "Led is OFF", "");
      }
    }

    if (text == "/start") {
      String welcome = "Welcome to Universal Arduino Telegram Bot library, " + from_name + ".\n";
      welcome += "This is Flash Led Bot example.\n\n";
      welcome += "/ledon : to switch the Led ON\n";
      welcome += "/ledoff : to switch the Led OFF\n";
      welcome += "/registry : para registrar un nuevo RFID\n";
      
      welcome += "/status : Returns current status of LED\n";
      bot.sendMessage(chat_id, welcome, "Markdown");
    }
  }
}


void setup() {
  Serial.begin(115200);

  // Set WiFi to station mode and disconnect from an AP if it was Previously
  // connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  // attempt to connect to Wifi network:
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
  client.setServer(mqtt_server, mqttPort);                            //Vinculacion con datos del servidor.
  client.setCallback(callback); 
  pinMode(ledPin, OUTPUT); // initialize digital ledPin as an output.
  delay(10);
  digitalWrite(ledPin, LOW); // initialize pin as off
}


void callback(char* topic, byte* payload, unsigned int length) {      //Funcion que se ejecuta cada que llega un mensaje y lo que hace es imprimir su contenido


Serial.print("Message arrived [");
String chat_id = String(bot.messages[bot.getUpdates(bot.last_message_received + 1)].chat_id);
 Serial.print(topic);                                                               
 Serial.print("] ");
 char paychar[64];
 for (int i = 0; i < length; i++) {
   Serial.print((char)payload[i]);
   paychar[i]=((char)payload[i]);
 }
 paychar[length]='\0';
 Serial.print("==");
 Serial.print(topic);
 Serial.print("==");
  Serial.print("RegisRes");
 Serial.print("==");
 Serial.println(strcmp(topic, "RegisRes"));
  if(strcmp(topic, "RegisRes") == 0){
    Serial.println("ha llegado algo de registry");
    Serial.println(paychar);
    if(paychar != "null"){
       Serial.println(paychar);
      bot.sendMessage(chat_id, "added ", "");
      bot.sendMessage(chat_id, paychar, "");
    }else{
      bot.sendMessage(chat_id, "nothing added ", "");
    }
    
  }
}


void reconnect() {                                                    //Funcion que en caso de que se pierda la conexion al servidor MQTT reintenta la vinculacion
 while (!client.connected()) 
 {
     Serial.print("ESPERANDO MQTT CONEXION");
     if (client.connect("ESP8266Client")) 
     {
         Serial.println("connected");
         client.publish("registrar", "avalible");
         client.subscribe("RegisRes");
     } 
     else 
     {
         Serial.print(client.state());
         delay(5000);
     }
 }
}

void loop() {
  if (millis() > Bot_lasttime + Bot_mtbs)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while(numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    if (!client.connected()) {
       reconnect();
     }
     client.loop();
    Bot_lasttime = millis();
  }
}
