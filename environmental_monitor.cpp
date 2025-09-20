#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_BME280.h>
#include <Adafruit_Sensor.h>

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

Adafruit_BME280 bme;
WebServer server(80);

const int ledPin = 2;
const int mq135Pin = 34;

float temperature = 0;
float humidity = 0;
float pressure = 0;
int airQuality = 0;

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  Wire.begin();

  if (!bme.begin(0x76)) {
    Serial.println("Could not find BME280 sensor!");
    while (1);
  }

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
    digitalWrite(ledPin, !digitalRead(ledPin));
  }

  digitalWrite(ledPin, HIGH);
  Serial.println("");
  Serial.print("Connected to WiFi. IP: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.begin();
}

void loop() {
  server.handleClient();
  
  temperature = bme.readTemperature();
  humidity = bme.readHumidity();
  pressure = bme.readPressure() / 100.0F;
  airQuality = analogRead(mq135Pin);

  Serial.printf("Temp: %.2f°C, Humidity: %.2f%%, Pressure: %.2f hPa, Air Quality: %d\n", 
                temperature, humidity, pressure, airQuality);

  delay(5000);
}

void handleRoot() {
  String html = "<!DOCTYPE html><html><head><title>Environmental Monitor</title>";
  html += "<meta http-equiv='refresh' content='5'>";
  html += "<style>body{font-family:Arial;margin:40px;background:#f0f0f0;}";
  html += ".sensor{background:white;padding:20px;margin:10px;border-radius:8px;box-shadow:0 2px 4px rgba(0,0,0,0.1);}";
  html += ".value{font-size:2em;color:#2196F3;}</style></head><body>";
  html += "<h1>ESP32 Environmental Monitor</h1>";
  html += "<div class='sensor'><h3>Temperature</h3><div class='value'>" + String(temperature, 1) + "°C</div></div>";
  html += "<div class='sensor'><h3>Humidity</h3><div class='value'>" + String(humidity, 1) + "%</div></div>";
  html += "<div class='sensor'><h3>Pressure</h3><div class='value'>" + String(pressure, 1) + " hPa</div></div>";
  html += "<div class='sensor'><h3>Air Quality</h3><div class='value'>" + String(airQuality) + "</div></div>";
  html += "</body></html>";
  
  server.send(200, "text/html", html);
}

void handleData() {
  String json = "{";
  json += "\"temperature\":" + String(temperature, 2) + ",";
  json += "\"humidity\":" + String(humidity, 2) + ",";
  json += "\"pressure\":" + String(pressure, 2) + ",";
  json += "\"airQuality\":" + String(airQuality);
  json += "}";
  
  server.send(200, "application/json", json);
}