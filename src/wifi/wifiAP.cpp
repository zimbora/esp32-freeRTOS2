
#include "wifiAP.h"

/*
  WiFiAccessPoint.ino creates a WiFi access point and provides a web server on it.

  Steps:
  1. Connect to the access point "yourAp"
  2. Point your web browser to http://192.168.4.1/H to turn the LED on or http://192.168.4.1/L to turn it off
     OR
     Run raw TCP "GET /H" and "GET /L" on PuTTY terminal with 192.168.4.1 as IP address and 80 as port

  Created for arduino-esp32 on 04 July, 2018
  by Elochukwu Ifediora (fedy0)
*/


WiFiServer server(80);

void WIFIAP::setup() {

  Serial.println();
  Serial.println("Configuring access point...");

  // You can remove the password parameter if you want the AP to be open.
  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  server.begin();
  Serial.println("Server started");
}

void WIFIAP::loop() {

  WiFiClient client = server.available();   // listen for incoming clients

  if (client) {                             // if you get a client,
    Serial.println("New Client.");           // print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        if (c == '\n') {                    // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

            // body
            const char* body = "<!DOCTYPE html><html>\
                <head>\
                  <meta content='5'/>\
                  <title>ESP32 Demo</title>\
                  <style>\
                    body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
                  </style>\
                </head>\
                <body>\
                  <h1>Hello from ESP32!</h1>\
                  <form action=\"/wifi\" method=\"post\">\
                    <label for=\"ssid\">SSID:</label><br>\
                    <input type=\"text\" id=\"ssid\" name=\"ssid\"><br>\
                    <label for=\"pass\">Password:</label><br>\
                    <input type=\"text\" id=\"pass\" name=\"pass\"><br><br>\
                    <input type=\"submit\" value=\"Submit\">\
                  </form>\
                </body>\
              </html>";

            client.print(body);

            // the content of the HTTP response follows the header:
            //client.print("Click <a href=\"/H\">here</a> to turn ON the LED.<br>");
            //client.print("Click <a href=\"/L\">here</a> to turn OFF the LED.<br>");

            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
            break;
          } else {    // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }

        // Check to see if the client request was "GET /H" or "GET /L":
        if (currentLine.endsWith("GET /wifi")) {
          //digitalWrite(LED_BUILTIN, HIGH);               // GET /H turns the LED on
          String request = "";
          while(client.available()){
            char r = client.read();
            if(r == ' ')
              break;
            else request += r;
          }
          Serial.println();
          Serial.println("request: "+request);

          String ssid = "";
          String pass = "";
          uint8_t i = 0;
          int16_t index_i = 0;
          int16_t index_f = 0;
          while(index_f != -1){

            index_i = request.indexOf("=");
            index_f = request.indexOf("&");
            String param = "";
            if(index_f == -1)
              param = request.substring(index_i+1);
            else
              param = request.substring(index_i+1,index_f);
            if(i == 0)
              ssid = param;
            else if(i==1)
              pass = param;
            i++;
            request = request.substring(index_f+1);
          }

          Serial.println("ssid: "+ssid);
          Serial.println("pass: "+pass);
          if(ssid != "" && pass != ""){
            client.stop();
            pWiFiCallbacks->onWiFiSet(ssid,pass);
          }

        }
        else if (currentLine.endsWith("POST /wifi")) {
          //digitalWrite(LED_BUILTIN, HIGH);               // GET /H turns the LED on
          String body = "";
          while(client.available() && body == ""){
            char r = client.read();

            if(r == '\n' && currentLine == "\r"){
              while(client.available()){
                body += (char)client.read();
                if(body.endsWith("\r\n\r\n"))
                  break;
              }
            }else if(r == '\n'){
              Serial.println(currentLine);
              currentLine = "";
            }else currentLine += r;
          }
          Serial.println();
          Serial.println("body: "+body);

          String ssid = "";
          String pass = "";
          uint8_t i = 0;
          int16_t index_i = 0;
          int16_t index_f = 0;
          while(index_f != -1){

            index_i = body.indexOf("=");
            index_f = body.indexOf("&");
            String param = "";
            if(index_f == -1)
              param = body.substring(index_i+1);
            else
              param = body.substring(index_i+1,index_f);
            if(i == 0)
              ssid = param;
            else if(i==1)
              pass = param;
            i++;
            body = body.substring(index_f+1);
          }
          Serial.println("ssid: "+ssid);
          Serial.println("pass: "+pass);
          if(ssid != "" && pass != ""){
            client.stop();
            pWiFiCallbacks->onWiFiSet(ssid,pass);
          }
        }

      }
    }
    // close the connection:
    client.stop();
    Serial.println("Client Disconnected.");
  }

}
