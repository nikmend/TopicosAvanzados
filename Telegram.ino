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
#include <Print.h>
#include <MFRC522.h>
#include <SdFat.h>

// Initialize Wifi connection to the router
char ssid[] = "MENDEZ WIFI";     // your network SSID (name)
char password[] = "79539360"; // your network key
const char* mqtt_server = "broker.hivemq.com";          //Direccion/dominio del servidor MQTT.
const int mqttPort = 1883;
const char* mqttUser = "telegram"; 

// Initialize Telegram BOT
#define BOTtoken "543265089:AAH7ss6IwBDzqGpfKZiF76J-InVJpXrOx4E"  // your Bot Token (Get from Botfather)
const int cardSize = 4;
WiFiClientSecure clientTel;
UniversalTelegramBot bot(BOTtoken, clientTel);
WiFiClient espClient;
PubSubClient client(espClient);
byte readCard[cardSize] = {1,2,3,4};
int Bot_mtbs = 1000; //mean time between scan messages
long Bot_lasttime;   //last time messages' scan has been done
bool Start = false;

const int ledPin = 16;
int ledStatus = 0;
#define SD_SS_PIN    4
SdFat sd;
char cardFile[] = "RFID_S2.txt";
char cardTempFile[] = "cardsTemp.txt";
//------------------------------------------------------------------------------------
void PrintCard(byte printCard[cardSize])
{
  int index;

  Serial.print("Card - ");
  for(index = 0; index < 4; index++)
  {
    if (index > 0)
    {
      Serial.print(",");
    }
    Serial.print(printCard[index]);
  }
  Serial.println(" ");
}

//------------------------------------------------------------------------------------
boolean findCard()
{
  byte currentCard[cardSize];
  char text[10];
  char c1;
  int  index;
  int  value;

  Serial.print("find ");
  PrintCard(readCard);

  // open input file
  ifstream readStr(cardFile);

  // check for open error
  if (!readStr.is_open())
  {
    Serial.println("open errorr");
    return false;
  }

  index = 0;
  // read until input fails
  while (!readStr.eof()) 
  {
    readStr >> value >> c1; 

    if (readStr.fail()) 
    {
      break;
    }

    currentCard[index] = value;
    
    index++;
    if (index > 3)
    {
      Serial.print("file read ");
      PrintCard(currentCard);
      if ((memcmp(currentCard, readCard, 4)) == 0)
      {
        return true;
      } 
      index = 0;
    }
  }

  return false;
}

//------------------------------------------------------------------------------------
void addCard(){
  int index;
  SdFile writeFile;
  Serial.print("add ");
  PrintCard(readCard);
  if (writeFile.open(cardFile, O_RDWR | O_CREAT | O_AT_END))
  { 
    for(index = 0; index < 4; index++)
    {
      writeFile.print(readCard[index]); 
      writeFile.print(",");
    }
    writeFile.close();
  }
  return;
}

//------------------------------------------------------------------------------------

void reconnect() {                                                    //Funcion que en caso de que se pierda la conexion al servidor MQTT reintenta la vinculacion
 while (!client.connected()) 
 {
     Serial.print("ESPERANDO MQTT CONEXION");
     if (client.connect("ESP8266Client")) 
     {
         Serial.println("connected");
         client.publish("registrar", "avalible");
         client.publish("ValidateRes","avalible");
         client.subscribe("RegisRes");
         client.subscribe("Validate");
     } 
     else 
     {
         Serial.print(client.state());
     }
 }
}

//------------------------------------------------------------------------------------

void registrar(){
   if (!client.connected()) {
   reconnect();
 }
 client.loop();
  Serial.println(" client.publish(registrar,nuevo);");
  client.publish("registrar","nuevo");
}
//------------------------------------------------------------------------------------
void abrir(){
   if (!client.connected()) {
   reconnect();
 }
  client.loop();
  Serial.println(" client.publish(abrir);");
  client.publish("abrirP","ok");
}
//------------------------------------------------------------------------------------
void statusAll(){
   if (!client.connected()) {
   reconnect();
 }
  client.loop();
  Serial.println(" client.publish(status);");
  client.publish("Azure2","status");
}
//------------------------------------------------------------------------------------

void apagarAlarma(){
   if (!client.connected()) {
   reconnect();
    }
     client.publish("Azure2","apagarAlarma");
}
//------------------------------------------------------------------------------------

void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i=0; i<numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;
    Serial.println(text);
    String from_name = bot.messages[i].from_name;
    if (from_name == "") from_name = "Guest";

    if (text == "/apagarAlarma") {
      apagarAlarma();
      bot.sendMessage(chat_id, "Apagando alarma", "");
    }
    if (text == "/registrar") {
      registrar();
      bot.sendMessage(chat_id, "Nuevo registro", "");
    }
    if (text == "/abrir") {
      abrir();
      bot.sendMessage(chat_id, "Abriendo puerta", "");
    }
    if (text == "/status") {
      statusAll();
      bot.sendMessage(chat_id, "Datos del sensor:", "");
     }

    if (text == "/start") {
      String welcome = "Bienvenido, " + from_name + ".\n";
      welcome += "Proyecto de Topicos.\n\n";
      welcome += "/registrar : Para registrar un nuevo RFID\n";
      welcome += "/apagarAlarma : Para apagar la alarma\n";
      welcome += "/abrir : Desbloquea la puerta\n";
      welcome += "/status : Retorna el balance de los sensores\n";
      bot.sendMessage(chat_id, welcome, "");
    }
   
  }
}

//------------------------------------------------------------------------------------

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
  if (!sd.begin(SD_SS_PIN, SPI_HALF_SPEED)){
    
    Serial.println("ERROR eN SD");   
  }
}
//------------------------------------------------------------------------------------

void Guardar(char* array){
  Serial.println("Guardar");
  Serial.println(array);
  char *strings[10];
  char *ptr = NULL;

    byte index = 0;
    ptr = strtok(array, ", ");  // takes a list of delimiters
    while(ptr != NULL)
    {
        strings[index] = ptr;
      
        index++;
        ptr = strtok(NULL, ", ");  // takes a list of delimiters
    }
    //Serial.println(index);
// print the tokens
    
    for(int n = 0; n < index; n++)
   { 
    String val=strings[n];
   readCard[n]=val.toInt();
    Serial.println(readCard[n]);
    Serial.println(strings[n]);
   }
   if (!findCard()){
    Serial.println("NO existe, creando");
    addCard();
   }else{
    Serial.println("Ya existe");
   }

}
//------------------------------------------------------------------------------------

boolean validarID(char* array){
  Serial.println("Validacion");
  char *strings[10];
  char *ptr = NULL;
  byte index = 0;
  ptr = strtok(array, ", ");  // takes a list of delimiters
  while(ptr != NULL){
      strings[index] = ptr; 
      index++;
      ptr = strtok(NULL, ", ");  // takes a list of delimiters
  }
  for(int n = 0; n < index; n++){ 
    String val=strings[n];
    readCard[n]=val.toInt();
   }
   return findCard();

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
    if(strcmp(topic, "RegisRes") == 0){
      Serial.println("ha llegado algo de registry");
      Serial.println(paychar);
      char * val=paychar;
      if(paychar != "null"){
         Serial.println(paychar);
         Guardar(paychar);
        bot.sendMessage(chat_id, "added ", "");
        bot.sendMessage(chat_id, val, "");
      }else{
        bot.sendMessage(chat_id, "nothing added ", "");
      }
    }
    else   if(strcmp(topic, "Validate") == 0){
      char * val=paychar;
      if(paychar != "null"){
         boolean validID=validarID(paychar);
         Serial.println(validID);
         if (!client.connected()) {
           reconnect();
         }
         client.loop();
         if(validID){
         client.publish("ValidateRes","TRUE"); 
          Serial.println("ES valido");
          bot.sendMessage(chat_id, "Validado ", "");
          bot.sendMessage(chat_id, val, "");
        }else{
          client.publish("ValidateRes","FALSE"); 
          Serial.println("NO es valido");
          bot.sendMessage(chat_id, "Invalido ", "");
        }
      }
    }
}

//------------------------------------------------------------------------------------


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
  Serial.println(".");
}
