
#include "wifiAP.h"

extern CALLS call;
extern SYSFILE sysfile;
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
//WebServer server(80);
/*
void handleRoot() {

  char temp[400];
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;

  snprintf(temp, 400,

           "<html>\
  <head>\
    <meta http-equiv='refresh' content='5'/>\
    <title>ESP32 Demo</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
  <body>\
    <h1>Hello from ESP32!</h1>\
    <p>Uptime: %02d:%02d:%02d</p>\
    <img src=\"/test.svg\" />\
  </body>\
</html>",

           hr, min % 60, sec % 60
          );
  server.send(200, "text/html", temp);

}

void handleNotFound() {

  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }

  server.send(404, "text/plain", message);

}

void drawGraph() {
  String out = "";
  char temp[100];
  out += "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" width=\"400\" height=\"150\">\n";
  out += "<rect width=\"400\" height=\"150\" fill=\"rgb(250, 230, 210)\" stroke-width=\"1\" stroke=\"rgb(0, 0, 0)\" />\n";
  out += "<g stroke=\"black\">\n";
  int y = rand() % 130;
  for (int x = 10; x < 390; x += 10) {
    int y2 = rand() % 130;
    sprintf(temp, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" stroke-width=\"1\" />\n", x, 140 - y, x + 10, 140 - y2);
    out += temp;
    y = y2;
  }
  out += "</g>\n</svg>\n";

  server.send(200, "image/svg+xml", out);
}
*/
void WIFIAP::setup() {

  Serial.println();
  Serial.println("Configuring access point...");

  // You can remove the password parameter if you want the AP to be open.
  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  /*
  server.on("/",handleRoot);
  server.on("/test.svg", drawGraph);
  server.onNotFound(handleNotFound);
  */
  server.begin();
  Serial.println("Server started");
}

void WIFIAP::loop() {

  //server.handleClient();

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
                  <form action=\"/wifi\">\
                    <label for=\"ssid\">SSID:</label><br>\
                    <input type=\"text\" id=\"ssid\" name=\"ssid\"><br>\
                    <label for=\"pass\">Password:</label><br>\
                    <input type=\"text\" id=\"pass\" name=\"pass\">\
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
          settings_set_param("wifi_ssid",ssid);
          settings_set_param("wifi_pwd",pass);

          settings_log();
          if(sysfile.write_file(FW_SETTINGS_FILENAME,settings.fw.version,sizeof(settings))){
            client.stop();
            Serial.println("Client Disconnected.");
            call.fw_reboot();
          }else{
            Serial.println("failing writing file: "+String(FW_SETTINGS_FILENAME));
          }


        }
        if (currentLine.endsWith("GET /L")) {
          //digitalWrite(LED_BUILTIN, LOW);                // GET /L turns the LED off
        }
      }
    }
    // close the connection:
    client.stop();
    Serial.println("Client Disconnected.");
  }

}
