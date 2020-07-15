

///************************************
 * Include Libraries
 ****************************************/
#include <WiFi.h>
#include <PubSubClient.h>
/****************************************
 * Define Constants
 ****************************************/
#define WIFISSID "INFINITUM9904_2.4" // Put your WifiSSID here
#define PASSWORD "s7AiHCX6UG" // Put your wifi password here
#define TOKEN "BBFF-utZo239dIY5aBUgPX2G4AOFRrPSvIs" // Put your Ubidots' TOKEN
#define MQTT_CLIENT_NAME "luisAngelReye" // MQTT client Name, please enter your own 8-12 alphanumeric character ASCII string; 
                                                        //it should be a random and unique ascii string and different from all other devices

#define VARIABLE_LABEL_1"var_led"// Assing the variable label
#define VARIABLE_LABEL_2"var_voltaje"
#define DEVICE_LABEL "esp32-wroom-32" // Assig the device label

#define pot 14
#define led 26 // Set the GPIO16 as RELAY

char mqttBroker[]  = "industrial.api.ubidots.com";
char payload[100];
char topic[150];
char topicSubscribe[100];
char str_volt[10];
/****************************************
 * Auxiliar Functions
 ****************************************/
WiFiClient ubidots;
PubSubClient client(ubidots);

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");
    
    // Attemp to connect
    if (client.connect(MQTT_CLIENT_NAME, TOKEN, "")) {
      Serial.println("Connected");
      client.subscribe(topicSubscribe);
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 2 seconds");
      // Wait 2 seconds before retrying
      delay(2000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  char p[length + 1];
  memcpy(p, payload, length);
  p[length] = NULL;
  String message(p);
  if (message == "0") {
    digitalWrite(led, LOW);
  } else {
    digitalWrite(led, HIGH);
  }
  
  Serial.write(payload, length);
  Serial.println(topic);
}

/****************************************
 * Main Functions
 ****************************************/
void setup() {
  Serial.begin(115200);
  WiFi.begin(WIFISSID, PASSWORD);
  // Assign the pin as OUTPUT
  pinMode(pot, INPUT); 
  pinMode(led, OUTPUT);

  Serial.println();
  Serial.print("Wait for WiFi...");
  
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  
  Serial.println("");
  Serial.println("WiFi Connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  client.setServer(mqttBroker, 1883);
  client.setCallback(callback);

  sprintf(topicSubscribe, "/v1.6/devices/%s/%s/lv", DEVICE_LABEL, VARIABLE_LABEL_1);
  
  client.subscribe(topicSubscribe);
}

void loop() {
  if (!client.connected()) {
    client.subscribe(topicSubscribe);
    reconnect();
       
  }
  
  sprintf(topic, "%s%s", "/v1.6/devices/", DEVICE_LABEL);
  sprintf(payload, "%s", ""); // Cleans the payload
  sprintf(payload, "{\"%s\":", VARIABLE_LABEL_2); // Adds the variable label
  
  float volt = analogRead(pot); 
  Serial.print("valor del potenciometro");Serial.println(volt);
  /* 4 is mininum width, 2 is precision; float value is copied onto str_sensor*/
  dtostrf(volt, 4, 2, str_volt);
  
  sprintf(payload, "%s {\"value\": %s}}", payload, str_volt); // Adds the value
  Serial.println("Publishing data to Ubidots Cloud");
  client.publish(topic, payload);
  client.loop();
  delay(1000);

}
