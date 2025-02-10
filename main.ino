// DeskPal
// I was bored and i did this, that's all. I think i could have used another way to get the time more precisely, but im good with what i have right now
// Made by https://github.com/brnomt
// Use this however you want to, i don't care a lot really

#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
//#include <HTTPClient.h>
#include <time.h>

// OLED screen configuration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Network and MQTT configuration
const char* ssid = "<YOUR_SSID_GOES_HERE>";          // Your SSID
const char* password = "<YOUR_PASSWORD_GOES_HERE>";    // Your password
const char* mqtt_server = "<YOUR_MQTT_SERVER_IP_GOES_HERE>"; // MQTT broker IP

WiFiClient espClient;
PubSubClient client(espClient);

// URLs for GET requests
const char* URL_PHRASE = "<YOUR_URL_GOES_HERE>";   // Motivational phrase URL
const char* URL_SPOTIFY = "<YOUR_URL_GOES_HERE>";  // Spotify trigger URL

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
const unsigned long clickThreshold = 300;    // ms to differentiate single vs double tap

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
// Functions to trigger requests based on button press type
void sendSingleClickRequest() {
  Serial.println("Sending GET request for motivational phrase...");
  // Censored host IP
  sendHttpTrigger("<YOUR_MQTT_SERVER_IP_GOES_HERE>", 1881, "/motiv");
}

void sendDoubleClickRequest() {
  Serial.println("Sending GET request for Spotify Now Playing...");
  // Censored host IP
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
// Function to draw the clock overlay in the bottom right corner (size 1)
void drawClockOverlay() {
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  int x, y = SCREEN_HEIGHT - 8;
  
  if (WiFi.status() != WL_CONNECTED) {
    // Connecting annimation
    int dotCount = (millis() / 500) % 4; // 0 - 3 dots
    String conectando = "Conectando";
    for (int i = 0; i < dotCount; i++) {
      conectando += ".";
    }
    int textWidth = conectando.length() * 6;
    x = SCREEN_WIDTH - textWidth;
    display.setCursor(x, y);
    display.print(conectando);
  } else if (timeStr != "") {
    // Show clock if hour
    int textWidth = timeStr.length() * 6;
    x = SCREEN_WIDTH - textWidth;
    display.setCursor(x, y);
    display.print(timeStr);
  }
}


// ------------------------------------------------------------------
// Function to draw the face with animation
void drawFace(int select, int offset = 0) {
  display.clearDisplay();
  
  display.setTextColor(SSD1306_WHITE);
  
  if(select == 4) {
    // Cara de sueño especial
    display.setTextSize(3);
    int cursorY = 10 - offset;
    display.setCursor(0, cursorY);
    display.print("(-_-)");
    display.setTextSize(1);
    display.setCursor(0, cursorY + 32);
    display.print("Zz...");
  } else {
    display.setTextSize(3);
    int cursorY = 20 - offset;
    display.setCursor(15, cursorY);
    switch(select) {
      case 1:
        display.println("(^_^)");
        break;
      case 2:
        display.println("(^-^)");
        break;
      case 3:
        display.println("(*-*)");   // Cara asombrada
        break;
      case 5:
        display.println("(>_<)");   // Cara enojada
        break;
      case 6:
        display.println("(o_O) ?");   // Cara confundida
        break;
      case 7:
        display.println("(7_7)");   // Cara de escepticismo
        break;
      case 8:
        display.println("(T_T)");   // Cara llorando
        break;
      case 9:
        display.println("(._.)");   // Cara random2
        break;
      case 10:
        display.println("(^3^)");   // Cara dando un beso
        break;
      case 11:
        display.println("(>u<)");   // Cara muy feliz, creo, no se
        break;
      case 12:
        display.println("(;-;)");   // Cara tampoco sé
        break;
      case 13:
        display.println("(.-.)");   // Cara Random
        break;
      default:
        display.println("?_?");
        break;
    }
  }
  
  drawClockOverlay();
  display.display();
}

// ------------------------------------------------------------------
// Función para animar la cara (animación de offset vertical)
void moveFace() {
  if (isShowingMessage) return;
  if (millis() - lastMoveTime >= 1000) {
    lastMoveTime = millis();
    static int faceAnimationStep = 0;
    const int offsets[] = {0, 2, 5, 10, 5, 2};
    offsetY = offsets[faceAnimationStep];
    faceAnimationStep = (faceAnimationStep + 1) % 6;
    drawFace(currentFace, offsetY);
  }
}

// ------------------------------------------------------------------
// Function to reconnect to MQTT and subscribe to required topics

unsigned long lastReconnectAttempt = 0;
const unsigned long reconnectInterval = 5000; // 5 segundos, por ejemplo

void tryReconnect() {
  if (millis() - lastReconnectAttempt >= reconnectInterval) {
    Serial.println("Intentando reconexión a MQTT...");
    if (client.connect("ESP32Client")) {
      Serial.println("Reconectado al MQTT.");
      client.subscribe("mqtt/1");
      client.subscribe("mqtt/2");
      client.subscribe("mqtt/3");
    }
    lastReconnectAttempt = millis();
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
    display.clearDisplay();
    display.setCursor(0, 10);
    display.println("Connecting.");
    display.display();
    delay(1000);
    display.clearDisplay();
    display.setCursor(0, 10);
    display.println("Connecting..");
    display.display();
    delay(1000);
    display.clearDisplay();
    display.setCursor(0, 10);
    display.println("Connecting...");
    display.display();
  }
  Serial.println("WiFi: Connected");
  display.clearDisplay();
  display.println("WiFi: Connected");
  display.display();

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  tryReconnect();
  
  randomSeed(millis());
  currentFace = random(1, 13);
  drawFace(currentFace);
  lastFaceChangeTime = millis();
}

// ------------------------------------------------------------------
void loop() {
  if (!client.connected()) {
    tryReconnect();
  }
  client.loop();
  
  // Manage display time based on message type:
  if (isShowingMessage) {
    if (currentMessageType == "spotify" && (millis() - lastMessageTime >= 2000)) {
      isShowingMessage = false;
      drawFace(currentFace, offsetY);
    }
    else if (currentMessageType == "motiv" && (millis() - lastMessageTime >= 30000)) {
      isShowingMessage = false;
      drawFace(currentFace, offsetY);
    }
  }
  
  moveFace();

  if (!isShowingMessage && (millis() - lastFaceChangeTime >= 60000)) {
    currentFace = random(1, 13);
    drawFace(currentFace, offsetY);
    lastFaceChangeTime = millis();
  }
  
  // Read the button state
  int buttonState = digitalRead(BUTTON_PIN);
  
  // Detect rising edge
  if (buttonState == HIGH && lastButtonState == LOW) {
    unsigned long now = millis();
    
    // If a motivational message is showing, cancel immediately
    if (isShowingMessage && currentMessageType == "motiv") {
      Serial.println("Cancelling motivational phrase");
      isShowingMessage = false;
      drawFace(currentFace, offsetY);
      lastPressTime = now;
      singleClickPending = false;
    }
    else {
      // Detect double tap
      if (now - lastPressTime < clickThreshold) {
        Serial.println("Double tap detected!");
        singleClickPending = false;
        sendDoubleClickRequest();
      } else {
        singleClickPending = true;
      }
      lastPressTime = now;
    }
  }
  
  if (singleClickPending && (millis() - lastPressTime >= clickThreshold)) {
    Serial.println("Single tap detected!");
    sendSingleClickRequest();
    singleClickPending = false;
  }
  
  lastButtonState = buttonState;
}
