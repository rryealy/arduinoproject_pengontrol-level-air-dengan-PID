#include <LiquidCrystal_I2C.h>
#include <Wire.h>

//Mendefinisikan PIN
const int TRIG = 10;
const int ECHO = 11;
const int ENA = 6;     
const int IN3 = 8;     
const int IN4 = 9;     

LiquidCrystal_I2C lcd(0x27, 16, 2);

//PID Constants
double Kp = 100;    // Proportional gain
double Ki = 0.01;   // Integral gain
double Kd = 10;     // Derivative gain

double setpoint = 4.00;  // Target water level in cm
double error = 0;
double lastError = 0;
double integral = 0;
double derivative = 0;
double PIDValue = 0;
int motorSpeed = 0;

unsigned long lastTime = 0;
const long interval = 100;  // Interval waktu untuk update PID (100ms)

void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);
  pinMode(ENA, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  
  // Set arah motor
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void loop() {
  unsigned long currentTime = millis();
  
  if (currentTime - lastTime >= interval) {
    // Baca jarak air
    double currentLevel = distance();

    if(currentLevel > 13.00) {
      currentLevel = 0;  // Jika lebih dari 13cm, anggap 0
    } else {
      currentLevel = 13.00 - currentLevel;  // Konversi ke level air
    }
    // Hitung PID
    calculatePID(currentLevel);
    
    // Jalankan motor
    runMotor(motorSpeed);
    
    // Update display
    updateDisplay(currentLevel);
    
    // Serial print untuk monitoring grafik
    Serial.print(currentLevel);
    Serial.print(",");
    Serial.println(motorSpeed);
    
    lastTime = currentTime;
  }
}

long distance() {
  digitalWrite(TRIG, LOW);
  delayMicroseconds(4);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);

  long t = pulseIn(ECHO, HIGH);
  long cm = t / 29 / 2;
  return cm;
}

void calculatePID(double currentLevel) {
  // Hitung error
  error =  setpoint - currentLevel  ;
  
  // Hitung integral
  integral += error;  // Convert interval to seconds
  
  // Hitung derivative
  derivative = error - lastError;
  
  // Hitung output PID
  PIDValue = (Kp * error) + (Ki * integral) + (Kd * derivative);
  
  // Batasi output motor
  motorSpeed = constrain(PIDValue, 0, 255);
  
  // Simpan error untuk iterasi berikutnya
  lastError = error;
}

void runMotor(int speed) {
  // Jalankan motor dengan PWM
  analogWrite(ENA, speed);
}

void updateDisplay(double currentLevel) {
  lcd.clear();
  
  // Baris pertama: Level air
  lcd.setCursor(0, 0);
  lcd.print("Level: ");
  lcd.print(currentLevel, 1);
  lcd.print(" cm");
  
  // Baris kedua: Kecepatan motor
  lcd.setCursor(0, 1);
  lcd.print("Speed: ");
  lcd.print(motorSpeed);
}
