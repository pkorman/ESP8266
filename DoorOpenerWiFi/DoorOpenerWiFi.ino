//ESP-01 relay modul, board: Generic ESP8266 Module
// Load Wi-Fi library
#include <ESP8266WiFi.h>

// Your network credentials in credentials.h
#include "credentials.h"
const char* ssid     = WIFI_SSID;
const char* password = WIFI_PASSWD;

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Auxiliar variables to store the current output state
String outputReleState = "off";

// Assign output variables to GPIO pins
const int HEARTBEAT_PIN = 1;
const int outputRele = 2;

long measure_time;
long open_time;
const long max_open_time = 12000;

void setup() {
  // Initialize the output variables as outputs
  pinMode(HEARTBEAT_PIN, OUTPUT);
  pinMode(outputRele, OUTPUT);
  // Set outputs to LOW
  digitalWrite(HEARTBEAT_PIN, HIGH);
  digitalWrite(outputRele, LOW);

  // Connect to Wi-Fi network with SSID and password
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(5000);

  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    digitalWrite(HEARTBEAT_PIN, LOW);
    delay(100);
    digitalWrite(HEARTBEAT_PIN, HIGH);
  }

  server.begin();
}

void loop(){
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // turns the GPIOs on and off
            if (header.indexOf("GET /2/on") >= 0) {
              outputReleState = "on";
              digitalWrite(outputRele, HIGH);
              open_time = millis();
            } else if (header.indexOf("GET /2/off") >= 0) {
              outputReleState = "off";
              digitalWrite(outputRele, LOW);
            }
            
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { background-color: #616161; font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: green; border: none; color: lightgray; padding: 40px 80px;");
            client.println("text-decoration: none; font-size: 60px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: red;}</style></head>");
            
            // Web Page Heading
            client.println("<body><h1>Door Opener</h1>");
            
            // Display current state, and ON/OFF buttons for GPIO 2  
            client.println("<p>GPIO 2 - State " + outputReleState + "</p>");
            // If the outputReleState is off, it displays the ON button       
            if (outputReleState=="off") {
              client.println("<p><a href=\"/2/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/2/off\"><button class=\"button button2\">OFF</button></a></p>");
            } 
            
            client.println("</body></html>");
            
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
      if (millis() > measure_time + 125) {
        measure_time = millis();
        digitalWrite(HEARTBEAT_PIN, !digitalRead(HEARTBEAT_PIN));
      }
      if (millis() > open_time + max_open_time) {
        outputReleState = "off";
        digitalWrite(outputRele, LOW);
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
  } else {
      if (millis() > measure_time + 2000) {
        measure_time = millis();
        digitalWrite(HEARTBEAT_PIN, !digitalRead(HEARTBEAT_PIN));
      }
      if (millis() > open_time + max_open_time) {
        outputReleState = "off";
        digitalWrite(outputRele, LOW);
      }
  }
}

