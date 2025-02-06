// DeskPal
// I was bored and i did this, that's all. I think i could have used another way to get the time more precisely, but im good with what i have right now
// Made by https://github.com/brnomt
// Use this however you want to, i don't care a lot really

#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <HTTPClient.h>
#include <time.h>

// OLED screen configuration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Network and MQTT configuration
const char* ssid = "<YOUR_SSID_GOES_HERE>";          // Your SSID
const char* password = "<YOUR_PASSWORD_GOES_HERE>";  // Your password
const char* mqtt_server = "<YOUR_MQTT_SERVER_IP_GOES_HERE>"; // MQTT broker IP

WiFiClient espClient;
PubSubClient client(espClient);

// URLs for GET requests
//I used a HTTP IN node in Node-RED, so it act as a trigger.
const char* URL_PHRASE = "<YOUR_URL_GOES_HERE>";
const char* URL_SPOTIFY = "<YOUR_URL_GOES_HERE>";

// Variables for clock (updated via MQTT on "time/timeserver")
String timeStr = "";

// Variables for face and animation
unsigned long lastFaceChangeTime = 0;
unsigned long lastMessageTime = 0;
int currentFace = 0; // Values from 1 to 4
bool isShowingMessage = false;
String currentMessageType = "";  // "motiv" or "spotify"
unsigned long lastMoveTime = 0;
int offsetY = 0;  // Offset for vertical face movement

// Button variables (assuming button on GPIO2)
#define BUTTON_PIN 1
int lastButtonState = HIGH;
unsigned long lastPressTime = 0;
const unsigned long debounceDelay = 50;      // ms for debounce
bool singleClickPending = false;
const unsigned long clickThreshold = 300;    // ms to differentiate single vs double click

// ------------------------------------------------------------------
// Function to send a GET request without waiting for a response
void sendHttpTrigger(const char* host, uint16_t port, const char* path) {
  WiFiClient client;
  if (client.connect(host, port)) {
    client.print("GET ");
    client.print(path);
    client.print(" HTTP/1.1\r\nHost: ");
    client.print(host);
    client.print("\r\nConnection: close\r\n\r\n");
    client.flush();
    client.stop();
    Serial.print("GET sent to ");
    Serial.print(path);
    Serial.println(" without waiting for response.");
  } else {
    Serial.println("Error connecting to server for GET request.");
  }
}

// ------------------------------------------------------------------
// Functions to trigger requests based on touch type
void sendSingleClickRequest() {
  Serial.println("Sending GET request for motivational phrase...");
  sendHttpTrigger("<YOUR_MQTT_SERVER_IP_GOES_HERE>", 1881, "/motiv");
}

void sendDoubleClickRequest() {
  Serial.println("Sending GET request for Spotify Now Playing...");
  sendHttpTrigger("<YOUR_MQTT_SERVER_IP_GOES_HERE>", 1881, "/trigger");
}

// ------------------------------------------------------------------
// MQTT callback function: updates the display based on topic
void callback(char* topic, byte* payload, unsigned int length) {
  const size_t bufferSize = 128;
  char message[bufferSize];
  size_t copyLength = (length < bufferSize - 1) ? length : bufferSize - 1;
  memcpy(message, payload, copyLength);
  message[copyLength] = '\0';

  Serial.print("Message received on [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.println(message);

  if (strcmp(topic, "time/timeserver") == 0) {
    timeStr = String(message);
  }
  else if (strcmp(topic, "spotify/nowplaying") == 0) {
    currentMessageType = "spotify";
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println(message);
    display.display();
    lastMessageTime = millis();
    isShowingMessage = true;
  }
  else if (strcmp(topic, "motiv") == 0) {
    currentMessageType = "motiv";
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println(message);
    drawClockOverlay();
    display.display();
    lastMessageTime = millis();
    isShowingMessage = true;
  }
}

// ------------------------------------------------------------------
// Function to draw the clock overlay at the bottom right corner
void drawClockOverlay() {
  if(timeStr != "") {
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    int textWidth = timeStr.length() * 6;
    int x = SCREEN_WIDTH - textWidth;
    int y = SCREEN_HEIGHT - 8;
    display.setCursor(x, y);
    display.print(timeStr);
  }
}

// ------------------------------------------------------------------
// Function to reconnect to MQTT and subscribe to required topics
void reconnect() {
  while (!client.connected()) {
    Serial.println("Reconnecting to MQTT...");
    if (client.connect("ESP32Client")) {
      Serial.println("Connected to MQTT.");
      client.subscribe("spotify/nowplaying");
      client.subscribe("time/timeserver");
      client.subscribe("motiv");
    } else {
      Serial.print("Failed, state code=");
      Serial.println(client.state());
      delay(5000);
    }
  }
}

// ------------------------------------------------------------------
void setup() {
  pinMode(BUTTON_PIN, INPUT);
  pinMode(9, OUTPUT);
  digitalWrite(9, LOW);
  
  Serial.begin(115200);
  Serial.println("Starting...");

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {  
    Serial.println("Error initializing SSD1306 display");
    while (true);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  display.println("Connecting...");
  display.display();
  delay(1000);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("WiFi: Connected");
  
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  reconnect();
  
  randomSeed(millis());
  currentFace = random(1, 5);
  lastFaceChangeTime = millis();
}

// ------------------------------------------------------------------
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}

// If u read this, have a nice day!
