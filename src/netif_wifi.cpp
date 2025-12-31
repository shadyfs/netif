// src/netif_wifi.cpp
#include "netif.h"

// WiFi module (ATWINC1500)
#include <SPI.h>
#include <WiFi101.h>
#include <SD.h>
#define WIFI_MISO              12
#define WIFI_MOSI              11 
#define WIFI_CS                10
#define WIFI_SCK               13  
#define WIFI_IRQ               33
#define WIFI_RST               34
#define WIFI_EN                35
char ssid[] = "CanLogger4";
char pass[] = "CanLogger4";
int status = WL_IDLE_STATUS;
WiFiServer server(80);

void netif_wifi_init()
{
  SPI.setMISO(WIFI_MISO);
  SPI.setMOSI(WIFI_MOSI);
  SPI.setCS(WIFI_CS);
  SPI.setSCK(WIFI_SCK);
  SPI.begin();
  WiFi.setPins(WIFI_CS, WIFI_IRQ, WIFI_RST, WIFI_EN);
  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    #ifdef DEBUG 
      Serial.println("WiFi shield not present");
    #endif
    // don't continue:
    while (true);
  }
  // attempt to create access point
  status = WiFi.beginAP(ssid, pass);
  if (status != WL_AP_LISTENING) {
    #ifdef DEBUG
      Serial.println("Creating access point failed");
    #endif
    // don't continue
    while (true);
  }
  delay(10000);
  server.begin();
  #ifdef DEBUG
    Serial.println("Wifi Setup Successful");
  #endif
}

void netif_wifi_poll() {
  WiFiClient client = server.available();
  if (!client) return;

  Serial.println("Client connected");

  while (client.connected() && !client.available()) {
    delay(1);
  }

  String request = client.readStringUntil('\r');
  Serial.print("Request: ");
  Serial.println(request);
  client.read(); // consume '\n'

  String filename = "/";
  int idx = request.indexOf(' ');
  if (idx != -1) {
    int idx2 = request.indexOf(' ', idx + 1);
    if (idx2 != -1) {
      filename = request.substring(idx + 1, idx2);
    }
  }

  // Serve file list if requesting root
  if (filename == "/") {
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connection: close");
    client.println();
    client.println("<html><head><title>CAN Logger Files</title></head><body>");
    client.println("<h1>Logged Files:</h1><ul>");

    File root = SD.open("/");
    if (root && root.isDirectory()) {
      File entry = root.openNextFile();
      while (entry) {
        if (!entry.isDirectory()) {
          String name = entry.name();
          client.print("<li><a href=\"");
          client.print(name);
          client.print("\">");
          client.print(name);
          client.println("</a></li>");
        }
        entry.close();
        entry = root.openNextFile();
      }
    } else {
      client.println("<li>No files found.</li>");
    }

    client.println("</ul></body></html>");
    client.stop();
    return;
  }

  // Serve file download
  if (SD.exists(filename.c_str())) {
    File downloadFile = SD.open(filename.c_str());
    if (downloadFile) {
      client.println("HTTP/1.1 200 OK");
      client.println("Content-Type: application/octet-stream");
      client.println("Content-Disposition: attachment; filename=\"" + filename + "\"");
      client.println("Connection: close");
      client.println();

      uint8_t buf[512];
      while (downloadFile.available()) {
        size_t bytesRead = downloadFile.read(buf, sizeof(buf));
        client.write(buf, bytesRead);
      }

      downloadFile.close();
      Serial.println("File sent successfully.");
    } else {
      client.println("HTTP/1.1 500 Internal Server Error");
      client.println();
    }
  } else {
    client.println("HTTP/1.1 404 Not Found");
    client.println("Content-Type: text/plain");
    client.println("Connection: close");
    client.println();
    client.println("404 File Not Found");
  }

  delay(10);
  client.stop();
  #ifdef DEBUG
    Serial.println("Client disconnected");
  #endif
}

bool netif_wifi_is_up() {
    return WiFi.status() == WL_AP_LISTENING;
}