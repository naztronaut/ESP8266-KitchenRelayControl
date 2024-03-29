/*
    This sketch demonstrates how to set up a simple HTTP-like server.
    The server will set a GPIO pin depending on the request
      http://server_ip/gpio/0 will set the GPIO2 low,
      http://server_ip/gpio/1 will set the GPIO2 high
      http://server_ip/gpio/status will get the status
    server_ip is the IP address of the ESP8266 module, will be
    printed to Serial when the module is connected.
*/

#include <ESP8266WiFi.h>

#ifndef STASSID
#define STASSID "ASHLAN"
#define STAPSK  "****"
#endif

const char* ssid = STASSID;
const char* password = STAPSK;

// Create an instance of the server
// specify the port to listen on as an argument
WiFiServer server(80);
int relay_pin = 5;
void setup() {
  Serial.begin(115200);
  
  // prepare LED
  pinMode(relay_pin, OUTPUT);
  digitalWrite(relay_pin, 0);

  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print(F("Connecting to "));
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(F("."));
  }
  Serial.println();
  Serial.println(F("WiFi connected"));

  // Start the server
  server.begin();
  Serial.println(F("Server started"));

  // Print the IP address
  Serial.println(WiFi.localIP());
}

void loop() {
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
  Serial.println(F("new client"));

  client.setTimeout(5000); // default is 1000

  // Read the first line of the request
  String req = client.readStringUntil('\r');
  Serial.println(F("request: "));
  Serial.println(req);

  // Match the request
  int val;
  if (req.indexOf(F("/gpio/0")) != -1) {
    val = 0;
  } else if (req.indexOf(F("/gpio/1")) != -1) {
    val = 1;
  } else {
    Serial.println(F("invalid request"));
    val = digitalRead(relay_pin);
  }

  // Set LED according to the request
  digitalWrite(relay_pin, val);

  // read/ignore the rest of the request
  // do not client.flush(): it is for output only, see below
  while (client.available()) {
    // byte by byte is not very efficient
    client.read();
  }

  // Send the response to the client
  // it is OK for multiple small client.print/write,
  // because nagle algorithm will group them into one single packet

  // Set headers and send 2 new lines for ajax to work properly
  client.print(F("HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin:*"));
  client.print("\r\n\r\n");
  if(req.indexOf(F("/gpio/status")) != -1) {
    client.print(digitalRead(relay_pin));
  } else {
    client.print(digitalRead(relay_pin));
  }
//    client.print(F("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\n"));
//    client.print(F("GPIO is now "));
//    client.print((val) ? F("high") : F("low"));
//    client.print(F("<br><br>Click <a href='http://"));
//    client.print(WiFi.localIP());
//    client.print(F("/gpio/1'>here</a> to switch LED GPIO on, or <a href='http://"));
//    client.print(WiFi.localIP());
//    client.print(F("/gpio/0'>here</a> to switch LED GPIO off."));
//    client.print(F("Relay pin is "));
//    client.print(digitalRead(relay_pin));
//    client.print(F("</html>"));
//  }
  

  // The client will actually be *flushed* then disconnected
  // when the function returns and 'client' object is destroyed (out-of-scope)
  // flush = ensure written data are received by the other side
  Serial.println(F("Disconnecting from client"));
}
