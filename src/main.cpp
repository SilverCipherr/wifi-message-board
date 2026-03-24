// ═══════════════════════════════════════════════════════════
//  ESP32 WiFi OLED Terminal (MQTT Edition)
//  Features: Public Internet Access via MQTT Broker
// ═══════════════════════════════════════════════════════════

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// ── CONFIG ── change these ───────────────────────────────────
const char* SSID       = "Your_WiFI_SSID";
const char* PASSWORD   = "Your_WiFi_Password";

// The APP_TOKEN serves as the secure MQTT topic path.
// Changing this ensures no one else controls your ESP32.
const char* APP_TOKEN  = "Your_App_Token"; 

const char* MQTT_BROKER = "broker.emqx.io";
const int   MQTT_PORT   = 1883;

// ── OLED ─────────────────────────────────────────────────────
#define SCREEN_WIDTH   128
#define SCREEN_HEIGHT   64
#define OLED_RESET      -1
#define MAX_LINES        4
#define CHARS_PER_LINE  21

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

String oledLines[MAX_LINES];
int    lineCount = 0;

WiFiClient espClient;
PubSubClient mqtt(espClient);
String subscribeTopic;

// ─────────────────────────────────────────────────────────────
//  OLED HELPERS
// ─────────────────────────────────────────────────────────────
void addLine(String text) {
  while (text.length() > 0) {
    String chunk;
    if (text.length() <= CHARS_PER_LINE) {
      chunk = text;
      text  = "";
    } else {
      int breakAt = CHARS_PER_LINE;
      for (int i = CHARS_PER_LINE; i > 0; i--) {
        if (text[i] == ' ') { breakAt = i; break; }
      }
      chunk = text.substring(0, breakAt);
      text  = text.substring(breakAt + (text[breakAt] == ' ' ? 1 : 0));
    }
    if (lineCount >= MAX_LINES) {
      for (int i = 0; i < MAX_LINES - 1; i++) oledLines[i] = oledLines[i + 1];
      oledLines[MAX_LINES - 1] = chunk;
    } else {
      oledLines[lineCount++] = chunk;
    }
  }
}

void renderDisplay() {
  display.clearDisplay();
  display.fillRect(0, 0, 128, 10, SSD1306_WHITE);
  display.setTextColor(SSD1306_BLACK);
  display.setTextSize(1);
  display.setCursor(2, 1);
  display.print("WiFi Message");
  display.setTextColor(SSD1306_WHITE);
  int startLine = max(0, lineCount - MAX_LINES);
  for (int i = startLine; i < lineCount; i++) {
    display.setCursor(0, 12 + (i - startLine) * 13);
    display.print(oledLines[i]);
  }
  display.display();
}

void showStatus(String l1, String l2 = "") {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 10); display.print(l1);
  display.setCursor(0, 26); display.print(l2);
  display.display();
}

void clearOLED() {
  lineCount = 0;
  for (int i = 0; i < MAX_LINES; i++) oledLines[i] = "";
}

// ─────────────────────────────────────────────────────────────
//  MQTT CALLBACK
// ─────────────────────────────────────────────────────────────
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.println(message);

  StaticJsonDocument<512> doc;
  DeserializationError error = deserializeJson(doc, message);
  
  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return;
  }

  const char* action = doc["action"];
  
  if (action && strcmp(action, "MSG") == 0) {
    const char* textBlock = doc["text"];
    if (textBlock && strlen(textBlock) > 0) {
      addLine(String(textBlock));
      renderDisplay();
    }
  } 
  else if (action && strcmp(action, "CLEAR") == 0) {
    clearOLED();
    addLine("Screen cleared");
    renderDisplay();
  }
  else if (action && strcmp(action, "PING") == 0) {
    String txTopic = String("wifimsg/") + APP_TOKEN + "/tx";
    mqtt.publish(txTopic.c_str(), "{\"status\":\"PONG\"}");
  }
}

// ─────────────────────────────────────────────────────────────
//  MQTT RECONNECT
// ─────────────────────────────────────────────────────────────
void reconnect() {
  while (!mqtt.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    
    if (mqtt.connect(clientId.c_str())) {
      Serial.println("connected");
      // Subscribe back to the unique topic
      mqtt.subscribe(subscribeTopic.c_str());
      
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqtt.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

// ─────────────────────────────────────────────────────────────
//  SETUP
// ─────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);

  // Initialize OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED init failed!"); while (true);
  }
  showStatus("Connecting WiFi...", SSID);

  // Connect Wi-Fi
  WiFi.begin(SSID, PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
  }
  Serial.println("\nConnected! IP: " + WiFi.localIP().toString());

  // Show status
  showStatus("Connecting to", "MQTT Broker...");
  
  // Set MQTT Topic dynamically using APP_TOKEN
  subscribeTopic = String("wifimsg/") + APP_TOKEN + "/rx";

  // Connect to MQTT Broker
  mqtt.setServer(MQTT_BROKER, MQTT_PORT);
  mqtt.setCallback(mqttCallback);

  renderDisplay(); // Switch from status screen to main WiFi Message screen
}

// ─────────────────────────────────────────────────────────────
//  LOOP
// ─────────────────────────────────────────────────────────────
void loop() {
  if (!mqtt.connected()) {
    reconnect();
  }
  mqtt.loop();
}