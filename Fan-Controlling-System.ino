#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Ultrasonic
#define TRIG 6
#define ECHO 5

// Motor Driver
#define IN1 8
#define IN2 9
#define ENA 10

long duration;
int distance;
int speedValue;

void setup() {
  Serial.begin(9600);

  lcd.init();
  lcd.backlight();

  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENA, OUTPUT);

  lcd.setCursor(0, 0);
  lcd.print("Fuzzy Fan System ");
  delay(2000);
  lcd.clear();-
}

void loop() {

  // -------- ULTRASONIC --------
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);

  digitalWrite(TRIG, LOW);

  duration = pulseIn(ECHO, HIGH);
  distance = duration * 0.034 / 2;

  // -------- FUZZY MEMBERSHIP --------
  float veryNear = max(0.0, (20.0 - distance) / 20.0);   // stronger
  float nearVal = max(0.0, (30.0 - distance) / 30.0);    // weaker
  float mediumVal = max(0.0, 1.0 - abs(distance - 30.0) / 15.0);
  float farVal = max(0.0, (distance - 35.0) / 35.0);
  float veryFar = max(0.0, (distance - 60.0) / 40.0);

  veryNear = constrain(veryNear, 0, 1);
  nearVal = constrain(nearVal, 0, 1);
  mediumVal = constrain(mediumVal, 0, 1);
  farVal = constrain(farVal, 0, 1);
  veryFar = constrain(veryFar, 0, 1);

  // -------- FUZZY RULES --------
  float numerator =
    (veryNear * 50) +     // VERY LOW
    (nearVal * 80) +      // LOW
    (mediumVal * 150) +   // MEDIUM
    (farVal * 220) +      // HIGH
    (veryFar * 255) +     // VERY HIGH
    ((nearVal * mediumVal) * 130) +   // smooth transition
    ((mediumVal * farVal) * 200);     // smooth transition

  float denominator =
    (veryNear + nearVal + mediumVal + farVal + veryFar + 
    (nearVal * mediumVal) +
    (mediumVal * farVal));

  if (denominator == 0) {
    speedValue = 0;
  } else {
    speedValue = (int)(numerator / denominator);
  }

  // -------- MOTOR CONTROL WITH THRESHOLD --------
if (distance <= 10) {
  // FORCE OFF
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, 0);
  speedValue = 0;   // important for display
}
else {
  // NORMAL FUZZY CONTROL
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, speedValue);
}

  // -------- LCD DISPLAY --------
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Dist:");
  lcd.print(distance);
  lcd.print("cm");

  lcd.setCursor(0, 1);
  lcd.print("Speed:");

  int confidence = 0;

  if (distance <= 10) {
  lcd.print("OFF ");
  confidence = 100;
}
  else if (veryNear > nearVal && veryNear > mediumVal) {
    lcd.print("VLOW ");
    confidence = (int)(veryNear * 100);
  }
  else if (nearVal > mediumVal) {
    lcd.print("LOW ");
    confidence = (int)(nearVal * 100);
  }
  else if (mediumVal > farVal) {
    lcd.print("MED ");
    confidence = (int)(mediumVal * 100);
  }
  else if (farVal > veryFar) {
    lcd.print("HIGH ");
    confidence = (int)(farVal * 100);
  }
  else {
    lcd.print("VHIGH ");
    confidence = (int)(veryFar * 100);
  }

  lcd.print(confidence);
  lcd.print("%");

  // -------- SERIAL DEBUG --------
  Serial.print("Distance: ");
  Serial.print(distance);

  Serial.print(" | VNear: ");
  Serial.print(veryNear);

  Serial.print(" | Near: ");
  Serial.print(nearVal);

  Serial.print(" | Med: ");
  Serial.print(mediumVal);

  Serial.print(" | Far: ");
  Serial.print(farVal);

  Serial.print(" | VFar: ");
  Serial.print(veryFar);

  Serial.print(" | Speed: ");
  Serial.println(speedValue);

  delay(400);
}