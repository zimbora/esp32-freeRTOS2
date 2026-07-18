
#include "wifiAP.h"

/*
  Creates a WiFi access point and provides a web server on it.

  Steps:
  1. Connect to the access point "yourAp"
  2. Point your web browser to http://192.168.4.1/
  3. Set the WiFi credentials and submit the form
*/


WiFiServer server(80);

static String urlDecode(String input) {
  input.replace("+", " ");
  String out = "";
  for (uint16_t i = 0; i < input.length(); i++) {
    if (input[i] == '%' && i + 2 < input.length()) {
      String hex = input.substring(i + 1, i + 3);
      char decoded = (char) strtol(hex.c_str(), nullptr, 16);
      out += decoded;
      i += 2;
    } else {
      out += input[i];
    }
  }
  return out;
}

static String getFormValue(const String& payload, const String& key) {
  String prefix = key + "=";
  int16_t start = payload.indexOf(prefix);
  if (start < 0) return "";
  start += prefix.length();
  int16_t end = payload.indexOf("&", start);
  if (end < 0) end = payload.length();
  return urlDecode(payload.substring(start, end));
}

void WIFIAP::setup(String mac) {

  Serial.println();
  Serial.println("Configuring access point...");

  String apSsid = String(ssid);
  mac.replace(":", "");
  if (mac.length() >= 6) {
    String suffix = mac.substring(mac.length() - 6);
    suffix.toUpperCase();
    apSsid += "-" + suffix;
  }

  // You can remove the password parameter if you want the AP to be open.
  //WiFi.softAP(apSsid.c_str(), password);
  WiFi.softAP(apSsid.c_str());
  Serial.println("AP SSID: " + apSsid);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  server.begin();
  Serial.println("Server started");
}

void WIFIAP::stop() {
  server.stop();
  WiFi.softAPdisconnect(true);
  Serial.println("SoftAP stopped");
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
            const char* body = "<!DOCTYPE html><html lang=\"en\">\
<head>\
  <meta charset=\"utf-8\">\
  <meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">\
  <title>ESP32 Setup Portal</title>\
  <style>\
    :root {\
      --bg-1: #f7f4eb;\
      --bg-2: #e8efe9;\
      --card: #fffdf8;\
      --ink: #16211a;\
      --muted: #5f6f62;\
      --accent: #136f63;\
      --accent-2: #d17b0f;\
      --border: #d9e3da;\
      --shadow: rgba(22, 33, 26, 0.14);\
    }\
    * { box-sizing: border-box; }\
    body {\
      margin: 0;\
      min-height: 100vh;\
      font-family: 'Trebuchet MS', 'Segoe UI', sans-serif;\
      color: var(--ink);\
      background: radial-gradient(circle at 8% 10%, rgba(209,123,15,0.22), transparent 35%),\
                  radial-gradient(circle at 92% 88%, rgba(19,111,99,0.20), transparent 38%),\
                  linear-gradient(135deg, var(--bg-1), var(--bg-2));\
      display: flex;\
      align-items: flex-start;\
      justify-content: center;\
      padding: 20px;\
    }\
    .card {\
      width: min(760px, 100%);\
      background: var(--card);\
      border: 1px solid var(--border);\
      border-radius: 18px;\
      box-shadow: 0 18px 45px -22px var(--shadow);\
      overflow: hidden;\
    }\
    .hero {\
      padding: 22px 24px 14px;\
      border-bottom: 1px dashed #c9d7cb;\
    }\
    h1 {\
      margin: 0;\
      font-size: clamp(1.3rem, 2.7vw, 2rem);\
      letter-spacing: 0.4px;\
    }\
    .sub {\
      margin: 8px 0 0;\
      color: var(--muted);\
      font-size: .96rem;\
    }\
    form {\
      display: grid;\
      grid-template-columns: repeat(2, minmax(0, 1fr));\
      gap: 12px;\
      padding: 18px 24px 24px;\
    }\
    .row {\
      display: flex;\
      flex-direction: column;\
      gap: 6px;\
    }\
    .full { grid-column: 1 / -1; }\
    label {\
      font-size: .84rem;\
      color: var(--muted);\
      text-transform: uppercase;\
      letter-spacing: .07em;\
      font-weight: 700;\
    }\
    input, textarea {\
      width: 100%;\
      border: 1px solid var(--border);\
      border-radius: 10px;\
      padding: 10px 12px;\
      background: #fffeff;\
      color: var(--ink);\
      font: inherit;\
      outline: none;\
      transition: border-color .2s, box-shadow .2s, transform .15s;\
    }\
    input:focus, textarea:focus {\
      border-color: var(--accent);\
      box-shadow: 0 0 0 3px rgba(19,111,99,.16);\
      transform: translateY(-1px);\
    }\
    .hint {\
      grid-column: 1 / -1;\
      color: var(--muted);\
      font-size: .86rem;\
      margin-top: 2px;\
    }\
    button {\
      grid-column: 1 / -1;\
      margin-top: 2px;\
      border: 0;\
      border-radius: 12px;\
      padding: 12px 16px;\
      color: #fff;\
      font-weight: 700;\
      letter-spacing: .04em;\
      background: linear-gradient(135deg, var(--accent), #0d5b51);\
      cursor: pointer;\
      transition: transform .15s ease, filter .2s ease;\
    }\
    button:hover { filter: brightness(1.05); }\
    button:active { transform: translateY(1px); }\
    .footer {\
      display: flex;\
      justify-content: space-between;\
      gap: 10px;\
      padding: 0 24px 22px;\
      color: #6d7e70;\
      font-size: .8rem;\
    }\
    .badge {\
      background: #f4efe2;\
      border: 1px solid #eadcc1;\
      color: var(--accent-2);\
      border-radius: 999px;\
      padding: 4px 10px;\
      font-weight: 700;\
    }\
    @media (max-width: 640px) {\
      form { grid-template-columns: 1fr; }\
    }\
  </style>\
</head>\
<body>\
  <section class=\"card\">\
    <header class=\"hero\">\
      <h1>Device Setup Portal</h1>\
      <p class=\"sub\">Configure Wi-Fi and optional cloud metadata for this ESP32 unit.</p>\
    </header>\
    <form action=\"/wifi\" method=\"post\">\
      <div class=\"row\">\
        <label for=\"ssid\">Wi-Fi SSID</label>\
        <input type=\"text\" id=\"ssid\" name=\"ssid\" placeholder=\"Your network name\" required>\
      </div>\
      <div class=\"row\">\
        <label for=\"pass\">Wi-Fi Password</label>\
        <input type=\"password\" id=\"pass\" name=\"pass\" placeholder=\"At least 8 characters\" required>\
      </div>\
      <p class=\"hint\">Only Wi-Fi SSID and password are applied immediately by this portal.</p>\
      <button type=\"submit\">Save & Reboot</button>\
    </form>\
    <div class=\"footer\">\
      <span>ESP32 SoftAP Config</span><span class=\"badge\">Local Mode</span>\
    </div>\
  </section>\
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

          String ssid = getFormValue(request, "ssid");
          String pass = getFormValue(request, "pass");

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

          String ssid = getFormValue(body, "ssid");
          String pass = getFormValue(body, "pass");
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
