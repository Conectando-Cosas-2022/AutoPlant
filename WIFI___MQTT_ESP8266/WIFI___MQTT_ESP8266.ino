

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// Update these with values suitable for your network.
const char* ssid = "HUAWEI-IoT";
const char* password = "ORTWiFiIoT";



const char* mqtt_server = "demo.thingsboard.io";
const char* token = "xi8BtsVO8eB9reDsBwCW";

// Connection objects
WiFiClient espClient;
PubSubClient client(espClient);

//timestamp of the last telemetry update
unsigned long lastMsg = 0;

//buffer sizes and messages
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];
char msg2[MSG_BUFFER_SIZE];

//json document to store incoming messages
DynamicJsonDocument incoming_message(256);

//Fake telemetry
int value = 0;

//Led state
boolean estado = false;

//Initiates WiFi Connection
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
    
    if (metodo=="checkStatus") {  //Check device status. Expects a response to the same topic number with status=true.
      
      char outTopic[128];
      ("v1/devices/me/rpc/response/"+_number).toCharArray(outTopic,128);
      
      DynamicJsonDocument resp(256);
      resp["status"] = true;
      char buffer[256];
      serializeJson(resp, buffer);
      client.publish(outTopic, buffer);
    }
    
    if (metodo=="setLedStatus") {  //Set led status and update attribute value via MQTT
      
      boolean estado = incoming_message["params"];

      if (estado) {
        digitalWrite(LED_BUILTIN,LOW); //turn on led
      } else {
        digitalWrite(LED_BUILTIN,HIGH); //turn off led
      }

      //Attribute update
      DynamicJsonDocument resp(256);
      resp["estado"] = estado;
      char buffer[256];
      serializeJson(resp, buffer);
      client.publish("v1/devices/me/attributes", buffer);
      Serial.print("Publish message [attribute]: ");
      Serial.println(buffer);
    }
  }
}

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

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

   unsigned long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    value = random(100);

    //Send value as telemetry
    DynamicJsonDocument resp(256);
    resp["temperatura"] = value;
    resp["humedad"] = value*10;
    char buffer[256];
    serializeJson(resp, buffer);
    client.publish("v1/devices/me/telemetry", buffer);
    
    Serial.print("Publish message [telemetry]: ");
    Serial.println(buffer);
    
  }
  
}
