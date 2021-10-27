#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <Wire.h>
#include <VL53L0X.h>

const char* ssid = "***";
const char* password = "******";
const char* ap_ssid = "deliveryBot01";
const char* ap_password = "00593287";

WiFiServer server(80);
String header;
String valueString1 = String(5);
String valueString2 = String(5);

int16_t lspeed = 0;
int16_t rspeed = 0;
int16_t lnspeed = 0;
int16_t rnspeed = 0;

float bat_volt = 0;

int16_t odist = 8190;

#define PWMB D0
#define BIN2 D3
#define BIN1 D4
#define STBY D5
#define AIN1 D6
#define AIN2 D7
#define PWMA D8

#define V_SDA D2
#define V_SCL D1

VL53L0X sensor;

void standby() {
  digitalWrite(STBY, LOW);
}

void setspeed(uint8_t pwm, uint8_t in1, uint8_t in2, int16_t speed) {
  digitalWrite(pwm, HIGH);
  if (speed > 0) {
    digitalWrite(STBY, HIGH);
    analogWrite(in1, speed);
    digitalWrite(in2, LOW);
  } else if (speed < 0) {
    digitalWrite(STBY, HIGH);
    digitalWrite(in1, LOW);
    analogWrite(in2, -speed);
  } else {
    digitalWrite(in1, HIGH);
    digitalWrite(in2, HIGH);
  }
}

void setleftspeed(int16_t speed) {
  setspeed(PWMB, BIN1, BIN2, speed);
}

void setrightspeed(int16_t speed) {
  setspeed(PWMA, AIN1, AIN2, speed);
}

void setup() {
  Serial.begin(115200);
  Serial.println("Coffe car v0.0");
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.softAP(ap_ssid, ap_password); 
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  Wire.begin(V_SDA, V_SCL);
  sensor.setTimeout(500);
  if (!sensor.init())
  {
    Serial.println("Failed to detect and initialize sensor!");
    while (1) {}
  }
  sensor.startContinuous();

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
  
  server.begin();
  
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(PWMA, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);
  pinMode(PWMB, OUTPUT);
  pinMode(STBY, OUTPUT);
}

void loop() {
  WiFiClient client = server.available();
  if (client) {
    String currentLine = "";
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        header += c;
        if (c == '\n') {
          if (currentLine.length() == 0) {
            sendPage(client);
            
            if (header.indexOf("GET /?id=")>=0) {
              int strpos1 = header.indexOf("id=");
              int strpos2 = header.indexOf("&value=");
              int strpos3 = header.indexOf("&ok=ok");
              String idString = header.substring(strpos1+3+11, strpos2);
              String valString = header.substring(strpos2+7, strpos3);
              int id = idString.toInt();
              int val = valString.toInt();
              
              Serial.println(id); 
              Serial.println(val);
              switch (id) {
                case 1: {
                  lnspeed = val;
                  break;
                }
                case 2: {
                  rnspeed = val;
                  break;
                }
                default: {
                  break;
                }
              }
              
            }         
            client.println();
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }
    header = "";
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }

  int16_t o = sensor.readRangeContinuousMillimeters();
  odist = (o != 0) ? o : odist; 
  if (sensor.timeoutOccurred()) {
    Serial.println(" TIMEOUT");
  }

  if (odist < 50) {
    if ((lnspeed > 0) && (rnspeed > 0)) {
      lnspeed = 0;
      rnspeed = 0;
    }
  }

  if (lnspeed != lspeed) {
    lspeed = lnspeed;
    setleftspeed(lspeed);
  }

  if (rnspeed != rspeed) {
    rspeed = rnspeed;
    setrightspeed(rspeed);
  }

  bat_volt = analogRead(A0) * 3.91 / 973.0;

  ArduinoOTA.handle();
}

void sendPage(WiFiClient client) {
  client.println("<!DOCTYPE html>");
  client.println("<html>");
  client.println("<head>");
  client.println("  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
  client.println("  <link rel=\"icon\" href=\"data:,\">");
  client.println("  <style>");
  client.println("    body {");
  client.println("      text-align: center;");
  client.println("      font-family: \"Trebuchet MS\", Arial;");
  client.println("      margin-left:auto;");
  client.println("      margin-right:auto;");
  client.println("    }");
  client.println("    .slider {");
  client.println("      width: 300px;");
  client.println("    }");
  client.println("  </style>");
  client.println("  <script src=\"https://ajax.googleapis.com/ajax/libs/jquery/3.3.1/jquery.min.js\"></script>");
  client.println("</head>");
  client.println("<body>");
  client.println("  <h1>Coffe robot</h1>");
  client.print("  <h2>Battery voltage: ");
  client.print(bat_volt);
  client.print("V (");
  client.print((bat_volt - 3.0) * 100 / 1.2);
  client.print("%)</h2>");
  client.println("  <p>S1: <span id=\"servoPos1\"></span></p>");
  client.println("  <input type=\"range\" min=\"-1000\" max=\"1000\" class=\"slider\" id=\"servoSlider1\" onchange=\"servo(this.id, this.value)\" value=\"0\"/>");
  client.println("  <p>S2: <span id=\"servoPos2\"></span></p>");
  client.println("  <input type=\"range\" min=\"-1000\" max=\"1000\" class=\"slider\" id=\"servoSlider2\" onchange=\"servo(this.id, this.value)\" value=\"0\"/>");
  client.println("  <script>");
  client.println("    var slider1 = document.getElementById(\"servoSlider1\");");
  client.println("    var slider2 = document.getElementById(\"servoSlider2\");");
  client.println("    var servoP1 = document.getElementById(\"servoPos1\");");
  client.println("    var servoP2 = document.getElementById(\"servoPos2\");");
  client.println("    servoP1.innerHTML = slider1.value;");
  client.println("    servoP2.innerHTML = slider2.value;");
  client.println("    slider1.oninput = function() {");
  client.println("      slider1.value = this.value;");
  client.println("      servoP1.innerHTML = this.value;");
  client.println("    }");
  client.println("    slider2.oninput = function() {");
  client.println("      slider2.value = this.value;");
  client.println("      servoP2.innerHTML = this.value;");
  client.println("    }");
  client.println("    $.ajaxSetup({timeout:1000});");
  client.println("    function servo(eid, pos) {");
  client.println("      $.get(\"/?id=\" + eid + \"&value=\" + pos + \"&ok=ok\");");
  client.println("      {Connection: close};");
  client.println("    }");
  client.println("  </script>");
  client.println("</body>");
  client.println("</html>");
}
