#include <LiquidCrystal_I2C.h>
#include <Wire.h>

// Pin definitions
const int TRIG_PIN = 11;
const int ECHO_PIN = 10;
const int ENA = 6;     
const int IN1 = 8;     
const int IN2 = 9;     
const int POT_PIN = A0;  // Potentiometer untuk Set Point

// Constants
const float SOUND_SPEED = 0.034;
const float TOLERANCE = 0;      
const int MIN_PUMP_SPEED = 255;   
const int MAX_PUMP_SPEED = 255;   
const int SENSOR_HEIGHT = 13.3;    // Jarak sensor ke dasar (cm)
const int MIN_DISTANCE = 0;      // Jarak minimum yang bisa dibaca sensor (cm)
const int MAX_WATER_LEVEL = 14;  // Ketinggian air maksimum (cm)

// Variables
long duration;
float currentLevel;     // Process Value (PV) dari dasar wadah (cm)
float setPoint;         // Set point dalam bentuk ketinggian dari dasar (cm)
int pumpSpeed;         
bool isFillingMode;    

// Initialize LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  Serial.begin(9600);
  
  lcd.init();
  lcd.backlight();
  
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  
  // Pastikan pompa mati saat startup
  stopPump();
}

void runPump(bool filling, int speed) {
  pumpSpeed = speed;
  isFillingMode = filling;
  
  if (speed == 0) {
    // Matikan pompa
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    analogWrite(ENA, 0);
    return;
  }
  
  if (filling) {
    // Mode mengisi
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
  } else {
    // Mode menguras
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
  }
  
  // Terapkan kecepatan
  analogWrite(ENA, speed);
}

void stopPump() {
  runPump(true, 0);  // Parameter filling tidak berpengaruh saat speed = 0
}

float readWaterLevel() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  duration = pulseIn(ECHO_PIN, HIGH);
  
  // Hitung jarak dalam cm dari sensor ke permukaan air
  float distance = duration * SOUND_SPEED / 2;
  
  // Batasi pembacaan jarak antara MIN_DISTANCE dan SENSOR_HEIGHT
  distance = constrain(distance, MIN_DISTANCE, SENSOR_HEIGHT);
  
  // Konversi jarak ke ketinggian air dari dasar
  // Saat jarak = SENSOR_HEIGHT, berarti ketinggian air = 0 cm
  // Saat jarak = MIN_DISTANCE, berarti ketinggian air = 35 cm
  float waterLevel = SENSOR_HEIGHT - distance;
  
  // Batasi ketinggian air antara 0 dan MAX_WATER_LEVEL
  waterLevel = constrain(waterLevel, 0, MAX_WATER_LEVEL);
  
  return waterLevel;
}

void updateLCD() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SP:");
  lcd.print(setPoint, 1);
  lcd.print("cm ");
  
  if (pumpSpeed > 0) {
    lcd.print(isFillingMode ? "FILL" : "DRAIN");
  } else {
    lcd.print("STOP");
  }
  
  lcd.setCursor(0, 1);
  lcd.print("PV:");
  lcd.print(currentLevel, 1);
  lcd.print("cm P:");
  lcd.print(pumpSpeed);
}

void controlPump() {
  float error = setPoint - currentLevel;
  
  // Hentikan pompa jika dalam toleransi
  if (abs(error) <= TOLERANCE) {
    stopPump();
    return;
  }
  
  // Hitung kecepatan berdasarkan error
  int speed = map(abs(error) * 10, 0, 50, MIN_PUMP_SPEED, MAX_PUMP_SPEED);
  speed = constrain(speed, MIN_PUMP_SPEED, MAX_PUMP_SPEED);
  
  if (error > 0) {
    // Air terlalu rendah - perlu mengisi
    runPump(true, speed);
  } else {
    // Air terlalu tinggi - perlu menguras
    runPump(false, speed);
  }
}

void loop() {
  // Baca Set Point dari potentiometer (range 0-MAX_WATER_LEVEL cm)
  setPoint = map(analogRead(POT_PIN), 0, 1023, 0, MAX_WATER_LEVEL);
  
  // Baca ketinggian air aktual
  currentLevel = readWaterLevel();
  
  // Kontrol pompa
  controlPump();
  
  // Update LCD
  updateLCD();
  
  // Debug info
  Serial.print("SP:");
  Serial.print(setPoint);
  Serial.print(" PV:");
  Serial.print(currentLevel);
  Serial.print(" Mode:");
  Serial.println(isFillingMode ? "FILL" : "DRAIN");
  
  delay(100);
}
