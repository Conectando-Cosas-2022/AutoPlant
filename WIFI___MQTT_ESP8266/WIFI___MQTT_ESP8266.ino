

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "DHT.h"          // Biblioteca para trabajar con DHT 11 (Sensor de temperatura y humedad)
#include <Adafruit_NeoPixel.h>


// Update these with values suitable for your network.
//const char* ssid = "HUAWEI-IoT";
//const char* password = "ORTWiFiIoT";

const char* ssid     = "iPhone de Juan Pablo";
const char* password = "jp123456";


const char* mqtt_server = "demo.thingsboard.io";
const char* token = "11QHcVqS0n3q8OvO22k1";


// -----DRFINITIONS-----------------
//#define DHTTYPE DHT11 // DHT 11
//#define DHT_PIN 2   // Conexión en PIN D4
#define LED1 14  // pin D5
#define FAN1 15 //Pin D8
#define FAN2 13 // D7
#define FAN3 12 // D6
#define Bomba1 5 // D1
//#define hterrestre A0 // pin A0 humedad terrestre
//#define pinTanque A0
//#define valorReserva 500
#define NUMPIXELS 16

Adafruit_NeoPixel pixels(NUMPIXELS, LED1, NEO_GRB + NEO_KHZ800);


// Connection objects
WiFiClient espClient;
PubSubClient client(espClient);

// Objetos de Sensores o Actuadores
//DHT dht(DHT_PIN, DHTTYPE);

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

////Fake telemetry
//int value = 0;

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


//-----------------ON OF LED--------------------------------------
void onLed(){
  for(int i=0; i<NUMPIXELS; i++) { // For each pixel...
    pixels.setPixelColor(i, pixels.Color(255,255, 100));
  }
  pixels.show(); 
}

void offLed(){
  pixels.clear();
}

//------------------------------------------------------------



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
        //digitalWrite(LED1, HIGH); //turn on led
      } else {
       pixels.clear();
       pixels.show(); 
       
       // digitalWrite(LED1, LOW); //turn off led
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
        digitalWrite(FAN1, LOW); //turn on led
      } else {
        digitalWrite(FAN1, HIGH); //turn off led
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
        digitalWrite(FAN2, LOW); //turn on led
        Serial.println("prender");
      } else {
        digitalWrite(FAN2, HIGH); //turn off led
        Serial.println("apagar");
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
        digitalWrite(FAN3, LOW); //turn on led
      } else {
        digitalWrite(FAN3, HIGH); //turn off led
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
        digitalWrite(Bomba1, LOW); //turn on bomba
      } else {
        digitalWrite(Bomba1, HIGH); //turn off bomba
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


//    //-----------------------------SET MANUAL STATUS--------------------------------------
//      if (metodo == "setControlManualStatus") { //Set led status and update attribute value via MQTT
//
//      boolean estado = incoming_message["params"];
//
//      //Attribute update
//      DynamicJsonDocument resp(256);
//      resp["MANUAL"] = estado;
//      char buffer[256];
//      serializeJson(resp, buffer);
//      client.publish("v1/devices/me/attributes", buffer);
//      Serial.print("Publish message [attribute]: ");
//      Serial.println(buffer);
//    }
//    //------------------------------------------------------------------
    
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
  pinMode(LED1, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  pinMode(FAN1, OUTPUT);
  pinMode(FAN2, OUTPUT);
  pinMode(FAN3, OUTPUT);
  pinMode(Bomba1, OUTPUT);
  
//--------SERIAL-----------
  Serial.begin(115200);

//--------WIFI START--------
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  
//--------DHT SENSOR--------
  //pinMode(DHT_PIN, INPUT);            // Inicializar el DHT como entrada
  //dht.begin();                        // Iniciar el sensor DHT

//---------LED------------
 pixels.begin();
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
//   //------LECTURAS DE SENSORES--------
//    
//  //temperature = dht.readTemperature();  
//  //humidity = dht.readHumidity();        
//    temperature= 30;
//    humidity= 50;
//    humedad_terrestre = analogRead(hterrestre);
//    humedad_terrestre = (1024 - humedad_terrestre)/5;
//    valorTanque = analogRead(pinTanque);
//    if(valorTanque>valorReserva){
//      estadoTanque = false;
//    }else{
//      estadoTanque = true;
//    }
//
//    //------REPORTE DE ATRIBUTOS--------
//     DynamicJsonDocument resp(256);
//      resp["Tanque1"] = estadoTanque;
//      char buffer[256];
//      serializeJson(resp, buffer);
//      client.publish("v1/devices/me/attributes", buffer);
//      Serial.print("Publish message [attribute]: ");
//      Serial.println(buffer);
//      
   //------PUBLICACION TELEMETRIA-------
   //if (!isnan(temperature) && !isnan(humidity) && !isnan(humedad_terrestre)) {
  //    DynamicJsonDocument resp(256);
  //    resp["TETE"] = 3;
  //    //resp["humedad"] = humidity;
  //    //resp["humedad terrestre"] = humedad_terrestre;
  //    char buffer[256];
  //    serializeJson(resp, buffer);
  //    client.publish("v1/devices/me/telemetry", buffer);

  //    Serial.print("Publish message [telemetry]: ");
  //    Serial.println(buffer);
  //  //
  //  //------------------------------------

  }//cierre if(now - lastMsg > 2000)



}//Fin del loop
