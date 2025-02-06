#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <time.h>

// Configuración de la pantalla OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Configuración de red y MQTT
const char* ssid = "SSID";          // Tu SSID de red
const char* password = "PASSWORD";      // Tu contraseña WiFi
const char* mqtt_server = "IP_ADD"; // IP del broker MQTT

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastFaceChangeTime = 0;
unsigned long lastMessageTime = 0;
int currentFace = 0;
bool isShowingMessage = false;

unsigned long lastMoveTime = 0;
int offsetY = 0;  // Offset para mover la cara verticalmente

// Función para dibujar la cara, con parámetro de offset (por defecto 0)
void drawFace(int select, int offset = 0) {
  display.clearDisplay();
  display.setTextSize(3);                // Se usa tamaño entero (3)
  display.setTextColor(SSD1306_WHITE);
  int cursorY = 20 - offset;
  display.setCursor(20, cursorY);

  // Selección de la cara a dibujar
  switch(select) {
    case 1:
      display.println("(^_^)");
      break;
    case 2:
      display.println("(^-^)");
      break;
    case 3:
      display.println("(@_@)");
      break;
    case 4:
      display.println("(-_-) Zz...");
      break;
    default:
      display.println("?_?");
      break;
  }
  display.display();
}

void callback(char* topic, byte* payload, unsigned int length) {
  // Usar un buffer estático para evitar la asignación dinámica de memoria
  const size_t bufferSize = 128;
  char message[bufferSize];
  
  // Asegurarse de no exceder el tamaño del buffer
  size_t copyLength = (length < bufferSize - 1) ? length : bufferSize - 1;
  memcpy(message, payload, copyLength);
  message[copyLength] = '\0';

  Serial.print("Mensaje recibido: ");
  Serial.println(message);
  
  // Mostrar mensaje en la pantalla
  if (copyLength > 0) {
    display.clearDisplay();
    display.setTextSize(1.5);  // Se usa un tamaño entero
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println(message);
    display.display();
    lastMessageTime = millis();
    isShowingMessage = true;
  }
}

void moveFace() {
  // Cambiar el offset cada 2 segundos
  if (millis() - lastMoveTime >= 1000) {
    lastMoveTime = millis();
    offsetY = (offsetY == 0) ? 5 : 0;  // Alterna entre 0 y 10 píxeles
    drawFace(currentFace, offsetY);
  }
}

void reconnect() {
  // Reconecta a MQTT
  while (!client.connected()) {
    Serial.println("Reconectando a MQTT...");
    if (client.connect("ESP32Client")) {
      Serial.println("Conectado de nuevo.");
      client.subscribe("spotify/nowplaying");
    } else {
      Serial.print("Fallo, código de estado=");
      Serial.println(client.state());
      delay(5000);
    }
  }
}

void setup() {
  pinMode(9, OUTPUT);  // Configura GPIO9 como salida
  digitalWrite(9, LOW); // Estado deseado
  Serial.begin(115200);
  Serial.println("Iniciando...");

  // Inicializar la pantalla OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {  
    Serial.println("Error al iniciar la pantalla SSD1306");
    while (true); // Bloquea si hay error
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  display.println("Conectando...");
  display.display();
  delay(1000);

  // Conexión a WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando a WiFi...");
  }
  Serial.println("Conectado a WiFi.");
  display.clearDisplay();
  display.println("WiFi: Conectado");
  display.display();

  // Configuración de MQTT
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  while (!client.connected()) {
    Serial.println("Conectando a MQTT...");
    if (client.connect("ESP32Client")) {
      Serial.println("Conectado a MQTT.");
      if (client.subscribe("mqtt/sub")) {
        Serial.println("Suscripción exitosa.");
      } else {
        Serial.println("Error al suscribirse.");
      }
    } else {
      Serial.println("Fallo de conexión al servidor MQTT. Intentando de nuevo en 5 segundos...");
      delay(5000);
    }
  }

  // Mostrar una cara aleatoria al inicio
  srand(time(NULL));
  currentFace = random(1, 5); // Valores de 1 a 4
  drawFace(currentFace);
  lastFaceChangeTime = millis();
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  // Si se muestra un mensaje y han pasado 2 segundos, vuelve a mostrar la cara
  if (isShowingMessage && (millis() - lastMessageTime >= 2500)) {
    isShowingMessage = false;
    drawFace(currentFace);
  }

  moveFace();

  // Cambia la cara cada minuto si no se está mostrando un mensaje
  if (!isShowingMessage && (millis() - lastFaceChangeTime >= 60000)) {
    currentFace = random(1, 5);
    drawFace(currentFace);
    lastFaceChangeTime = millis();
  }
}
