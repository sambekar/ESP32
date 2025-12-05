#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <WiFiManager.h>
#include <ESPmDNS.h>
#include <WiFiMulti.h>


#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>

#define PANEL_WIDTH   80
#define PANEL_HEIGHT  20
#define PANEL_CHAIN   2   // set correct number of chained panels

// ---------------- PIN MAP ----------------
HUB75_I2S_CFG mxconfig(PANEL_WIDTH, PANEL_HEIGHT, PANEL_CHAIN);
String newHostname = "matrixmaestro-esp32";
const uint32_t connectTimeoutMs = 5000;
WiFiMulti wifiMulti;

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
  dma_display->setCursor(0, 0);
  dma_display->setTextSize(2);
  dma_display->setTextColor(dma_display->color565(255, 255, 0));
  dma_display->print(currentText);
}

void setup() {
  Serial.begin(115200);
  setupPins();
  dma_display = new MatrixPanel_I2S_DMA(mxconfig);
  dma_display->begin();
  dma_display->setBrightness8(160);
  // Setup WiFi
  Serial.println(F("Setting AP (Access Point)..."));
  WiFiManager wm;

  // OPTIONAL: reset saved WiFi creds for testing
  // wm.resetSettings();

  Serial.println("Connecting to saved WiFi...");

  bool res = wm.autoConnect("ShauryasNet-jiofi", "ucD8KFvW");
  // AP name: MatrixMaestro-Setup
  // password: 12345678 (you can remove the password if you want open AP)

  if (!res) {
    Serial.println("Failed to connect. Rebooting...");
    delay(2000);
    ESP.restart();
  }

  Serial.println("WiFi connected!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  // Start MDNS
  if (MDNS.begin("matrixmaestro-esp32")) {
    Serial.println("mDNS responder started: http://matrixmaestro-esp32.local");
  } else {
    Serial.println("Error setting up MDNS responder!");
  }
  server.onNotFound([]() {
    if (server.method() == HTTP_OPTIONS) {
      server.sendHeader("Access-Control-Allow-Origin", "*");
      server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
      server.sendHeader("Access-Control-Allow-Headers", "*");
      server.send(204);  // No Content for preflight requests
    } else {
      server.send(404, "text/plain", "Not found");
    }
  });
  server.on("/display", handleDisplayPost);
  server.begin();
  Serial.println(F("Webserver started..."));
}

void loop() {
  server.handleClient();
  drawTextOnPanel();
  delay(50);
}