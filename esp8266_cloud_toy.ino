/* Cloud Toy
 *
 * This device subscribes to a MQTT topic and receives commands to activate its LEDs.
 */

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WebSocketsClient.h>
extern "C" {
#include "user_interface.h"
}

const char* ssid = "";
const char* wifi_password = "";
const char* wifi_hostname = "CloudToy";

const char* mqtt_server = "";
unsigned int mqtt_port = 1883;
const char* mqtt_user = "cloudtoy";
const char* mqtt_password = "";

WiFiClient espClient;
PubSubClient client(espClient);

// LED states
bool red_on, blue_on, green_on;
uint8_t red_pin = D8;
uint8_t blue_pin = D7;
uint8_t green_pin = D6;


void delay_flash_led(unsigned int duration) {
  unsigned int ledFlashDelay = 100;
  for(unsigned int i = 0; (i * ledFlashDelay) < duration; i++) {
    digitalWrite(blue_pin, i % 2 ? LOW : HIGH);
    delay(ledFlashDelay);
  }
  // End state is led off.
  digitalWrite(blue_pin, LOW);
}


void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client", mqtt_user, mqtt_password)) {
      Serial.println("connected");
      // Once connected, resubscribe
      client.subscribe("cloudtoy/#");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay_flash_led(5000);
    }
  }
}


void callback(char* topic, byte* payload, unsigned int length) {
  payload[length] = 0;
  Serial.printf("Message arrived [%s] ", topic);
  for (int i = 0; i < length; i++) {
    Serial.print((const char)payload[i]);
  }
  Serial.println();

  String topic_s(topic);
  String msg((const char *)payload);
  if (topic_s.indexOf("cloudtoy/led") != -1) {
    Serial.println("Setting LED state");
    blue_on = red_on = green_on = false;
    if (msg.indexOf("off") != -1) {
      // A led control message including "off" and a colour has undefined behaviour
      Serial.println("All LEDs off");
    }
    if (msg.indexOf("red") != -1) {
      Serial.println("Red LED on");
      red_on = true;
    }
    if (msg.indexOf("blue") != -1) {
      Serial.println("Blue LED on");
      blue_on = true;
    }
    if (msg.indexOf("green") != -1) {
      Serial.println("Green LED on");
      green_on = true;
    }
  } else {
    Serial.printf("Unknown topic: %s\n", topic);
  }
}


void setup() {
  pinMode(blue_pin, OUTPUT);
  pinMode(green_pin, OUTPUT);
  pinMode(red_pin, OUTPUT);
  digitalWrite(blue_pin, LOW);
  digitalWrite(green_pin, LOW);
  digitalWrite(red_pin, LOW);

  Serial.begin(115200);
  delay(10);

  // Connect to Wifi.
  wifi_station_set_hostname((char *)wifi_hostname);
  Serial.printf("\n\n\nConnecting to %s\n", ssid);
  WiFi.begin(ssid, wifi_password);
  while (WiFi.status() != WL_CONNECTED) {
    delay_flash_led(500);
    Serial.print(">");
  }
  Serial.println(" connected");

  // Configure MQTT client.
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  digitalWrite(red_pin, red_on ? HIGH: LOW);
  pinMode(red_pin, red_on ? OUTPUT: INPUT);
  digitalWrite(blue_pin, blue_on ? HIGH: LOW);
  pinMode(blue_pin, blue_on ? OUTPUT: INPUT);
  digitalWrite(green_pin, green_on ? HIGH: LOW);
  pinMode(green_pin, green_on ? OUTPUT: INPUT);
}
