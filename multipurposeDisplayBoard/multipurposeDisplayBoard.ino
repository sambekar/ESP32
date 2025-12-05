#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>

#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>

#define PANEL_WIDTH   80
#define PANEL_HEIGHT  40
#define PANEL_CHAIN   2   // set correct number of chained panels

// ---------------- PIN MAP ----------------
HUB75_I2S_CFG mxconfig(PANEL_WIDTH, PANEL_HEIGHT, PANEL_CHAIN);

void setupPins() {
  mxconfig.gpio.r1 = 25;
  mxconfig.gpio.g1 = 26;
  mxconfig.gpio.b1 = 27;

  mxconfig.gpio.r2 = 14;
  mxconfig.gpio.g2 = 12;
  mxconfig.gpio.b2 = 13;

  mxconfig.gpio.a  = 23;
  mxconfig.gpio.b  = 19;
  mxconfig.gpio.c  = 5;
  mxconfig.gpio.d  = 17;
  mxconfig.gpio.e  = 18;

  mxconfig.gpio.lat = 4;
  mxconfig.gpio.oe  = 15;
  mxconfig.gpio.clk = 16;
}

MatrixPanel_I2S_DMA *dma_display = nullptr;

// ---------------- WiFi ----------------
const char* ssid     = "ShauryasNet-kitchen-2.4GHz";
const char* password = "ucD8KFvW  ";

WebServer server(80);

// Text to display
String currentText = "READY";

void handleDisplayPost() {
  if (server.method() != HTTP_POST) {
    server.send(405, "text/plain", "Method Not Allowed");
    return;
  }

  String body = server.arg("plain");

  // Parse JSON
  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, body);

  if (error) {
    server.send(400, "text/plain", "Invalid JSON");
    return;
  }

  if (!doc.containsKey("text")) {
    server.send(400, "text/plain", "JSON must contain 'text'");
    return;
  }

  currentText = doc["text"].as<String>();

  server.send(200, "application/json", "{\"status\":\"OK\"}");

  Serial.print("Updated display text: ");
  Serial.println(currentText);
}

void drawTextOnPanel() {
  dma_display->fillScreen(0);
  dma_display->setCursor(5, 20);
  dma_display->setTextSize(1);
  dma_display->setTextColor(dma_display->color565(255, 255, 0));
  dma_display->print(currentText);
}

void setup() {
  Serial.begin(115200);
  setupPins();

  dma_display = new MatrixPanel_I2S_DMA(mxconfig);
  dma_display->begin();
  dma_display->setBrightness8(160);

  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected! IP:");
  Serial.println(WiFi.localIP());

  server.on("/display", handleDisplayPost);
  server.begin();
}

void loop() {
  server.handleClient();
  drawTextOnPanel();
  delay(50);
}
