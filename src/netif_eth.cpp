// src/netif_eth.cpp
#include "netif.h"

#include <SPI.h>
#include <NativeEthernet.h>
#include <SD.h>

byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
IPAddress ip(192, 168, 1, 2);
IPAddress myDns(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
static EthernetServer server(80);

void netif_eth_init() {
    Ethernet.begin(mac, ip, myDns, gateway, subnet);
    server.begin();
}

void netif_eth_poll() {
  EthernetClient client = server.available();
  if (!client) return;
  #ifdef DEBUG
    Serial.println("Client connected");
  #endif 
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

bool netif_eth_is_up() {
    return Ethernet.linkStatus() == LinkON;
}
