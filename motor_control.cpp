#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>
#include <AccelStepper.h>

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

WebServer server(80);

Servo servo1;
Servo servo2;
AccelStepper stepper(AccelStepper::DRIVER, 27, 26);

const int servo1Pin = 18;
const int servo2Pin = 19;
const int stepPin = 27;
const int dirPin = 26;
const int ledPin = 2;

int servo1Pos = 90;
int servo2Pos = 90;
long stepperPos = 0;
long stepperTarget = 0;

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);

  servo1.attach(servo1Pin);
  servo2.attach(servo2Pin);
  servo1.write(servo1Pos);
  servo2.write(servo2Pos);

  stepper.setMaxSpeed(2000);
  stepper.setAcceleration(1000);
  stepper.setCurrentPosition(0);

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
  server.on("/servo1", handleServo1);
  server.on("/servo2", handleServo2);
  server.on("/stepper", handleStepper);
  server.on("/status", handleStatus);
  server.begin();
}

void loop() {
  server.handleClient();
  stepper.run();

  if (stepper.distanceToGo() == 0 && stepperTarget != stepperPos) {
    stepperPos = stepperTarget;
    Serial.printf("Stepper reached position: %ld\n", stepperPos);
  }

  delay(1);
}

void handleRoot() {
  String html = "<!DOCTYPE html><html><head><title>Motor Control</title>";
  html += "<style>body{font-family:Arial;margin:40px;background:#f0f0f0;}";
  html += ".control{background:white;padding:20px;margin:10px;border-radius:8px;box-shadow:0 2px 4px rgba(0,0,0,0.1);}";
  html += ".slider{width:100%;height:25px;border-radius:5px;background:#ddd;outline:none;}";
  html += ".button{background:#2196F3;color:white;padding:10px 20px;border:none;border-radius:5px;cursor:pointer;margin:5px;}";
  html += ".status{font-size:1.2em;color:#333;margin:10px 0;}</style>";
  html += "<script>function updateServo1(val){fetch('/servo1?pos='+val);document.getElementById('s1val').innerHTML=val;}";
  html += "function updateServo2(val){fetch('/servo2?pos='+val);document.getElementById('s2val').innerHTML=val;}";
  html += "function moveStepper(steps){fetch('/stepper?steps='+steps);}</script></head><body>";
  html += "<h1>ESP32 Motor Control</h1>";
  html += "<div class='control'><h3>Servo 1</h3>";
  html += "<div class='status'>Position: <span id='s1val'>" + String(servo1Pos) + "</span>°</div>";
  html += "<input type='range' min='0' max='180' value='" + String(servo1Pos) + "' class='slider' onchange='updateServo1(this.value)'></div>";
  html += "<div class='control'><h3>Servo 2</h3>";
  html += "<div class='status'>Position: <span id='s2val'>" + String(servo2Pos) + "</span>°</div>";
  html += "<input type='range' min='0' max='180' value='" + String(servo2Pos) + "' class='slider' onchange='updateServo2(this.value)'></div>";
  html += "<div class='control'><h3>Stepper Motor</h3>";
  html += "<div class='status'>Position: " + String(stepperPos) + " steps</div>";
  html += "<button class='button' onclick='moveStepper(200)'>+200 Steps</button>";
  html += "<button class='button' onclick='moveStepper(100)'>+100 Steps</button>";
  html += "<button class='button' onclick='moveStepper(-100)'>-100 Steps</button>";
  html += "<button class='button' onclick='moveStepper(-200)'>-200 Steps</button>";
  html += "<button class='button' onclick='moveStepper(-" + String(stepperPos) + ")'>Home</button></div>";
  html += "</body></html>";
  
  server.send(200, "text/html", html);
}

void handleServo1() {
  if (server.hasArg("pos")) {
    int pos = server.arg("pos").toInt();
    pos = constrain(pos, 0, 180);
    servo1Pos = pos;
    servo1.write(pos);
    Serial.printf("Servo 1 moved to: %d°\n", pos);
  }
  server.send(200, "text/plain", "OK");
}

void handleServo2() {
  if (server.hasArg("pos")) {
    int pos = server.arg("pos").toInt();
    pos = constrain(pos, 0, 180);
    servo2Pos = pos;
    servo2.write(pos);
    Serial.printf("Servo 2 moved to: %d°\n", pos);
  }
  server.send(200, "text/plain", "OK");
}

void handleStepper() {
  if (server.hasArg("steps")) {
    long steps = server.arg("steps").toInt();
    stepperTarget += steps;
    stepper.moveTo(stepperTarget);
    Serial.printf("Stepper moving %ld steps to position: %ld\n", steps, stepperTarget);
  }
  server.send(200, "text/plain", "OK");
}

void handleStatus() {
  String json = "{";
  json += "\"servo1\":" + String(servo1Pos) + ",";
  json += "\"servo2\":" + String(servo2Pos) + ",";
  json += "\"stepperPos\":" + String(stepperPos) + ",";
  json += "\"stepperTarget\":" + String(stepperTarget) + ",";
  json += "\"stepperRunning\":" + String(stepper.isRunning() ? "true" : "false");
  json += "}";
  
  server.send(200, "application/json", json);
}