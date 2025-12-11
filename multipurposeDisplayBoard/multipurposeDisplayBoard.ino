#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <WiFiManager.h>
#include <ESPmDNS.h>
#include <WiFiMulti.h>
#include <Arduino.h>
#include <ESP32-HUB75-VirtualMatrixPanel_T.hpp>
#include <Fonts/FreeSans24pt7b.h>
#define PANEL_RES_X     80
#define PANEL_RES_Y     40
#define VDISP_NUM_ROWS      2 // Number of rows of individual LED panels, change to 1 for single panel
#define VDISP_NUM_COLS      1
#define PANEL_CHAIN_LEN     (VDISP_NUM_ROWS*VDISP_NUM_COLS)
#define PANEL_CHAIN_TYPE CHAIN_BOTTOM_RIGHT_UP // change to CHAIN_NONE for single panel
#define PANEL_SCAN_TYPE  FOUR_SCAN_40PX_HIGH
MatrixPanel_I2S_DMA *dma_display = nullptr;
using MyScanTypeMapping = ScanTypeMapping<PANEL_SCAN_TYPE>;
VirtualMatrixPanel_T<PANEL_CHAIN_TYPE, MyScanTypeMapping>* virtualDisp = nullptr;

// ---------------- PIN MAP ----------------

String newHostname = "matrixmaestro-esp32";
const uint32_t connectTimeoutMs = 5000;
WiFiMulti wifiMulti;
HUB75_I2S_CFG mxconfig(PANEL_RES_X*2,PANEL_RES_Y/2,PANEL_CHAIN_LEN);
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

// ---------------- WiFi ----------------
const char* ssid     = "ShauryasNet-kitchen-2.4GHz";
const char* password = "ucD8KFvW  ";

WebServer server(80);

// Text to display
String currentText = "Initializing....";

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
  drawTextOnPanel();
}

void drawTextOnPanel() {
  //virtualDisp->setFont(&FreeSans24pt7b);
  virtualDisp->fillScreen(0);
  virtualDisp->setCursor(0, 30);
  virtualDisp->setTextSize(1);
  virtualDisp->setTextColor(dma_display->color565(0, 255, 255));
  virtualDisp->clearScreen();
  virtualDisp->print(currentText);
}

void setup() {
  Serial.begin(115200);
  
  setupPins();  
  mxconfig.i2sspeed = HUB75_I2S_CFG::HZ_10M;
  mxconfig.clkphase = false;
  dma_display = new MatrixPanel_I2S_DMA(mxconfig);
  dma_display->begin();
  dma_display->setBrightness8(50); //0-255
  dma_display->clearScreen();
  virtualDisp = new VirtualMatrixPanel_T<PANEL_CHAIN_TYPE, MyScanTypeMapping>(VDISP_NUM_ROWS, VDISP_NUM_COLS, PANEL_RES_X, PANEL_RES_Y);
  virtualDisp->setDisplay(*dma_display);
  virtualDisp->print(currentText);

   //delay(3000);
   //virtualDisp->clearScreen();
   //virtualDisp->drawDisplayTest();
  // Setup WiFi
  Serial.println(F("Setting AP (Access Point)..."));
  WiFiManager wm;

  // OPTIONAL: reset saved WiFi creds for testing
  // wm.resetSettings();

  Serial.println("Connecting to saved WiFi...");

  bool res = wm.autoConnect("ESP-Wait10Sec-192.168.4.1");
  delay(1000);

  if (!res) {
    Serial.println(F("Failed to connect or setup timeout. Restarting..."));
    currentText = "starting in AP mode";
    drawTextOnPanel();
    ESP.restart();
  } else {
    Serial.println(F("Connected to WiFi!"));
    Serial.println(WiFi.SSID());
  }

  Serial.println("WiFi connected!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  currentText=WiFi.localIP().toString();
  drawTextOnPanel();
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
  // OPTIONS preflight handler
    server.on("/display", HTTP_OPTIONS, []() {
        server.sendHeader("Access-Control-Allow-Origin", "*");
        server.sendHeader("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
        server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
        server.send(204);
    });

    // POST handler
    server.on("/display", HTTP_POST, []() {
        server.sendHeader("Access-Control-Allow-Origin", "*");
        server.sendHeader("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
        server.sendHeader("Access-Control-Allow-Headers", "Content-Type");

        handleDisplayPost();
    });
  server.begin();
  Serial.println(F("Webserver started..."));
}

void loop() {
  server.handleClient();
  delay(50);
}