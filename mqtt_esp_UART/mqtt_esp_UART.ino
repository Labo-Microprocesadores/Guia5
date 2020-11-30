/**************************************************************************
 * Communication using Wi-Fi with MQTT protocol and UART
 * Grupo 2 - Labo. de Micros
 **************************************************************************/



//Para probarlo: en una terminal suscribirse a "holaTopic". En otra suscribirse a "myAnswerTopic": mosquitto_sub -v -t "myAnswerTopic"
//Desde otra terminal publicar al topic "myTopic": mosquitto_pub -t 'myTopic' -m 'holahola'
//usar el serial monitor en 115200

#include <ESP8266WiFi.h>
#include <PubSubClient.h> 

unsigned long  lastMillis;


#define  CLOUD_MALE  0
#define  CLOUD_SANTI 1

#define CLOUD CLOUD_MALE

#if CLOUD==CLOUD_MALE
char ssid[] = "Airport Extreme";
char password[] = "malena1996";
//IPAddress MqttServer(192,168,0,9);
IPAddress MqttServer(34,121,212,49);
const unsigned int MqttPort=1883; 
const char MqttUser[]="admin";
const char MqttPassword[]="mnb";
const char MqttClientID[]="";

#elif CLOUD==CLOUD_SANTI
char ssid[] = ""; //COMPLETAR
char password[] = " "; //COMPLETAR
IPAddress MqttServer(34,121,212,49);
const unsigned int MqttPort=1883; 
const char MqttUser[]="admin";
const char MqttPassword[]="mnb";
const char MqttClientID[]="";
#endif


#define DEBUG_OFF  0
#define DEBUG_ON   1
#define debug DEBUG_ON
#define debug_message(fmt)
/*\
  do {              \
    if (debug)          \
       Serial.print(fmt);     \
  } while (0)
  */


#define serial_message(fmt)  
/*\
  do {              \
      if (debug)          \
      Serial.print(">>> "); \
        Serial.print(fmt);     \
      if (debug)          \
        Serial.print(" <<<<\n"); \
  } while (0)*/


// Create Cloud sockets
WiFiClient wclient;
PubSubClient client(wclient);

void setup_gpios(void);
void setup_wifi(void);    
void setup_mqtt(void);
void reconnect(void); 
void publish_init_state(void);
void callback(char* topic, byte* payload, unsigned int length);
void ParseTopic(char* topic, byte* payload, unsigned int length);

#define Board_LED         D0  
#define External_LED      D1
#define Board_LED_OFF     1
#define Board_LED_ON      0
#define External_LED_OFF  0
#define External_LED_ON   1
#define SERIAL_TERM  "\n"

void setup_gpios(void);
void setup_wifi(void);    
void setup_mqtt(void);
void reconnect(void); 
void publish_init_state(void);
void callback(char* topic, byte* payload, unsigned int length);
void ParseTopic(char* topic, byte* payload, unsigned int length);

void setup() 
{
  //Serial.begin(115200);
  Serial.begin(9600, SERIAL_8N1);
  Serial.swap();  
  Serial1.begin(115200);
  Serial1.setDebugOutput(true);
  setup_gpios();         // initialize used GPIOS
  setup_wifi();          // initialize WIFI an connect to network
  setup_mqtt();          // initialize mqtt server
}

void loop() 
{
  if (!client.connected()) 
  {
      reconnect();    
  }
  client.loop();  //This should be called regularly to allow the client to process incoming messages and maintain its connection to the server
  byte serin = 0;
  if (Serial.available()>0)
  {
    serin=Serial.read();
    client.publish("fromUART","hello from UART",false);
  }
  if (2000 < (millis()-lastMillis)) 
  {
    lastMillis = millis(); 
    if(debug)
    {
    client.publish("holaTopic","holaaaa",false);
    //Serial.write(3);
    }
  }
}


void setup_gpios(void )
{
  pinMode(Board_LED, OUTPUT);
  digitalWrite(Board_LED, Board_LED_OFF);
  pinMode(External_LED, OUTPUT);
  digitalWrite(External_LED,External_LED_OFF);
}


void setup_mqtt(void) 
{
 client.setServer(MqttServer, MqttPort);
 client.setCallback(callback);
}

void setup_wifi(void) 
{
  /*
  debug_message("\n\n");
  debug_message("Connecting to ");
  debug_message(ssid);
  debug_message("\n");
  */
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
   // debug_message(".");
  }
  /*
 debug_message("\nWiFi connected");  
 debug_message("IP address: ");
 debug_message(WiFi.localIP());
 debug_message("\n\n");
 */
 digitalWrite(External_LED,External_LED_ON); // WIFI is OK
}


void reconnect() 
{
 // Loop until we're reconnected
 while (!client.connected()) 
 {
 debug_message("Attempting MQTT connection...");
 // Attempt to connect
 if (client.connect(MqttClientID,MqttUser,MqttPassword)) 
 {
  //debug_message("connected \r\n");
  // ... and subscribe to topic
  client.subscribe("myTopic"); 
  client.subscribe("play"); 
  client.subscribe("pause"); 
  client.subscribe("level");
  client.subscribe("player");  
  client.subscribe("start");
  client.subscribe("end");    
 } 
 else 
 {
  delay(3000);
  }
 }
}


void callback(char* topic, byte* payload, unsigned int length) 
{
  for (int i=0;i<length;i++) 
  {
   // debug_message((char)payload[i]);
  }
  //debug_message("\r\n");
  client.publish("myAnswerTopic","chau",false); //answers
  Serial.write(3);
  ParseTopic(topic,payload,length);
}

void ParseTopic(char* topic, byte* payload, unsigned int length)
{
  if(!strcmp(topic,"level"))
  {
    //Serial.write(L);
    Serial.write((char)payload[i]);
  }
  else if(!strcmp(topic,"play"))
  {
    Serial.write(P);
  }
  else if(!strcmp(topic,"pause"))
  {
    Serial.write(A);
  }
  else if(!strcmp(topic,"player"))
  {

  }
  else if(!strcmp(topic,"start"))
  {
    Serial.write(S);
  }
  else if(!strcmp(topic,"end"))
  {
    Serial.write(E);
  }
}
