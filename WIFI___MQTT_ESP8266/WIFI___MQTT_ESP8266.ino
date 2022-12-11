
/////////////////////////////////////////////////////////////
//////////////////////// AUTOPLANT //////////////////////////
//////////////// Proyecto integrador 2 2022 /////////////////
/////////////////////////////////////////////////////////////
///////// Nicolas Nemmer - Juan Pablo Peyroulou /////////////
///// Prof. Emiliano Espíndola - Prof. Juan Pedro Silva /////
/////////////////////////////////////////////////////////////
///////////////////// Universidad ORT ///////////////////////
/////////////////////////////////////////////////////////////

/*========= BIBLIOTECAS =========*/
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "DHT.h"          // Biblioteca para trabajar con DHT 11 (Sensor de temperatura y humedad)
#include <Adafruit_NeoPixel.h>

/*========= CONSTANTES =========*/

// Credenciales de la redWiFi
//const char* ssid = "HUAWEI-IoT";
//const char* password = "ORTWiFiIoT";

const char* ssid     = "iPhone de Juan Pablo";
const char* password = "jp123456";


const char* mqtt_server = "demo.thingsboard.io";
const char* token = "11QHcVqS0n3q8OvO22k1";


// -----DRFINITIONS-----------------
#define LED1 14        // Luces en pin D5
#define FAN1 15        //Ventilador 1 en Pin D8
#define FAN2 13        //Ventilador 2 en Pin D7
#define FAN3 12        //Ventilador 3 en Pin D6
#define Bomba1 5       //Bomba en pin D1
#define NUMPIXELS 16   //Numero de leds Luces

Adafruit_NeoPixel pixels(NUMPIXELS, LED1, NEO_GRB + NEO_KHZ800);


// Connection objects
WiFiClient espClient;
PubSubClient client(espClient);


//---------------VARIABLES--------------------------
unsigned long lastMsg = 0;
int msgPeriod = 2000;       // Actualizar los datos cada 2 segundos
float humidity = 0;
float temperature = 0;
long humedad_terrestre = 0;
long valorTanque = 0;
boolean estadoTanque = false;
boolean led_state = false;

//buffer sizes and messages
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];
char msg2[MSG_BUFFER_SIZE];

//json document to store incoming messages
DynamicJsonDocument incoming_message(256);

//Led state
boolean estado = false;


//--------------Initiates WiFi Connection------------------
void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}



//----------------------------------FUNCION CALLBACK------------------------------------------

//This method is called whenever a MQTT message arrives. We must be prepared for any type of incoming message.
//We are subscribed to RPC Calls: v1/devices/me/rpc/request/+
void callback(char* topic, byte* payload, unsigned int length) {

  //log to console
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  //convert topic to string to parse
  String _topic = String(topic);

  if (_topic.startsWith("v1/devices/me/rpc/request/")) {
    //We are in a request, check request number
    String _number = _topic.substring(26);

    //Read JSON Object
    deserializeJson(incoming_message, payload);
    String metodo = incoming_message["method"];

    //----------------------CHECK STATUS--------------------------------------------
    if (metodo == "checkStatus") { //Check device status. Expects a response to the same topic number with status=true.

      char outTopic[128];
      ("v1/devices/me/rpc/response/" + _number).toCharArray(outTopic, 128);

      DynamicJsonDocument resp(256);
      resp["status"] = true;
      char buffer[256];
      serializeJson(resp, buffer);
      client.publish(outTopic, buffer);
    }

    //----------------------SET LED STATUS------------------------------------------
    if (metodo == "setLedStatus") { //Set led status and update attribute value via MQTT

      boolean estado = incoming_message["params"];

      if (estado) {
         for(int i=0; i<NUMPIXELS; i++) { // For each pixel...
            pixels.setPixelColor(i, pixels.Color(255,255, 100));
         }
         pixels.show(); 
      } else {
       pixels.clear();
       pixels.show(); 
      }

      //Attribute update
      DynamicJsonDocument resp(256);
      resp["LED1"] = estado;
      char buffer[256];
      serializeJson(resp, buffer);
      client.publish("v1/devices/me/attributes", buffer);
      Serial.print("Publish message [attribute]: ");
      Serial.println(buffer);
    }


    //-------------------------SET ExOk STATUS----------------------------------------
    if (metodo == "setExOkStatus") { //Set led status and update attribute value via MQTT

      boolean estado = incoming_message["params"];

      if (!estado) {
        digitalWrite(FAN1, LOW);    //Apagar Extractor
      } else {
        digitalWrite(FAN1, HIGH);   //Prender Extractor
      }

      //Attribute update
      DynamicJsonDocument resp(256);
      resp["EXOK"] = estado;
      char buffer[256];
      serializeJson(resp, buffer);
      client.publish("v1/devices/me/attributes", buffer);
      Serial.print("Publish message [attribute]: ");
      Serial.println(buffer);
    }
    
     //-------------------------SET Ex2 STATUS----------------------------------------
    if (metodo == "setEx2Status") { //Set led status and update attribute value via MQTT

      boolean estado = incoming_message["params"];
      Serial.print(estado);

      if (!estado) {
        digitalWrite(FAN2, LOW);    //Apagar Extractor
      } else {
        digitalWrite(FAN2, HIGH);   //Prender Extractor
      }

      //Attribute update
      DynamicJsonDocument resp(256);
      resp["EX2"] = estado;
      char buffer[256];
      serializeJson(resp, buffer);
      client.publish("v1/devices/me/attributes", buffer);
      Serial.print("Publish message [attribute]: ");
      Serial.println(buffer);
    } 
    
    //-------------------------SET FAN1 STATUS----------------------------------------
    if (metodo == "setFan1Status") { //Set led status and update attribute value via MQTT

      boolean estado = incoming_message["params"];

      if (!estado) {
        digitalWrite(FAN3, LOW);   //Apagar Extractor
      } else {
        digitalWrite(FAN3, HIGH);  //Prender Extractor
      }

      //Attribute update
      DynamicJsonDocument resp(256);
      resp["FAN1"] = estado;
      char buffer[256];
      serializeJson(resp, buffer);
      client.publish("v1/devices/me/attributes", buffer);
      Serial.print("Publish message [attribute]: ");
      Serial.println(buffer);
    }


      //-----------------------------SET BOMBA1 STATUS--------------------------------------
      if (metodo == "setBomba1Status") { //Set led status and update attribute value via MQTT

      boolean estado = incoming_message["params"];

      if (estado) {
        digitalWrite(Bomba1, LOW); //turn OFF bomba
      } else {
        digitalWrite(Bomba1, HIGH); //turn ON bomba
      }

      //Attribute update
      DynamicJsonDocument resp(256);
      resp["BOMBA1"] = estado;
      char buffer[256];
      serializeJson(resp, buffer);
      client.publish("v1/devices/me/attributes", buffer);
      Serial.print("Publish message [attribute]: ");
      Serial.println(buffer);
    }
    
  }//FIN TOPIC START WITH 

}//FIN CALBACK


//-----------FUNCION RECONNECT--------------------------------
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect - client.connect(DEVICE_ID, TOKEN, TOKEN)
    if (client.connect("NODEMCU Nuevo", token, token )) {
      Serial.println("connected");
      // Once connected, subscribe to rpc topic
      client.subscribe("v1/devices/me/rpc/request/+");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
//---------------------------------------------------------------


void setup() {
  
//--------PINMODES-------
  pinMode(LED1, OUTPUT);     //Inicializar LED1 como salida
  pinMode(FAN1, OUTPUT);     //Inicializar FAN1 como salida
  pinMode(FAN2, OUTPUT);     //Inicializar FAN2 como salida
  pinMode(FAN3, OUTPUT);     //Inicializar FAN3 como salida
  pinMode(Bomba1, OUTPUT);   //Inicializar Bomba1 como salida
  
//--------SERIAL-----------
  Serial.begin(115200);      //Inicializar conexión Serie para utilizar el Monitor

//--------WIFI START--------
  setup_wifi();                          // Establecer la conexión WiFi
  client.setServer(mqtt_server, 1883);   // Establecer los datos para la conexión MQTT
  client.setCallback(callback);          // Establecer la función del callback para la llegada de mensajes en tópicos suscriptos

//---------LED------------
 pixels.begin();             //Inicializar funciones luces
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  
  if (now - lastMsg > 2000) {
    lastMsg = now;
    Serial.print("-");

    
  }//cierre if(now - lastMsg > 2000)

}//Fin del loop
