#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "TMPL3sNrn-fwo"
#define BLYNK_TEMPLATE_NAME "DC motor controlling"
#define BLYNK_AUTH_TOKEN "k0IveMH93_OG1pz4xY97jW4lCFyzjzuT"

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

// WiFi credentials 
char auth[]= "k0IveMH93_OG1pz4xY97jW4lCFyzjzuT";
char ssid[] = "Infinix NOTE 30 5G";
char pass[] = "harshita05";

// Pin definitions
const int motorout1 = D3;
const int motorEnable = D5;
const int motorout2 = D4;
const int speedSensor = D0;
const int currentSensor = A0;
const int voltageDetectPin = D6;  // Digital voltage status pin

volatile unsigned long pulseCount = 0;
unsigned long prevMillis = 0;
int rpm = 0;
float current = 0;
int pwmValue = 0;

String voltageStatus = "LOW";

void IRAM_ATTR countPulse() {
  pulseCount++;
}

void setup() {
  Serial.begin(9600);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass, "blynk.cloud",80);

  pinMode(motorout1, OUTPUT);
  pinMode(motorout2, OUTPUT);
  digitalWrite(motorEnable, HIGH);

  pinMode(speedSensor, INPUT);
  attachInterrupt(digitalPinToInterrupt(speedSensor), countPulse, RISING);

  pinMode(voltageDetectPin, INPUT);  // For voltage threshold detection

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Init...");
  delay(1000);
  lcd.clear();
}

void loop() {
  Blynk.run(); 

  if (millis() - prevMillis >= 1000) {
    noInterrupts();
    int count = pulseCount;
    pulseCount = 0;
    interrupts();

    rpm = count * 60;

    int adcValue = analogRead(currentSensor);
    float voltage = adcValue * (3.3 / 1023.0);
    current = (voltage - 2.5) / 0.185;
    if (current < 0) current = 0;

    // Voltage detection logic
    int voltRead = digitalRead(voltageDetectPin);
    voltageStatus = voltRead == HIGH ? "HIGH" : "LOW";

    // LCD Display
    lcd.setCursor(0, 0);
    lcd.print("RPM:");
    lcd.print(rpm);
    lcd.print(" ");

    lcd.setCursor(0, 1);
    lcd.print("I:");
    lcd.print(current, 2);
    lcd.print(" V:");
    lcd.print(voltageStatus);

    // Blynk virtual pins
    Blynk.virtualWrite(V1, rpm);
    Blynk.virtualWrite(V2, current);
    Blynk.virtualWrite(V3, voltageStatus);

    prevMillis = millis();
  }
}

BLYNK_WRITE(V0) {
  int sliderVal = param.asInt();
  pwmValue = map(sliderVal, 0, 1500, 0, 5);
  analogWrite(motorEnable, pwmValue);
}
