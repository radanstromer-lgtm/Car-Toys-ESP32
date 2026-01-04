#include "BluetoothSerial.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled!
#endif

BluetoothSerial SerialBT;

// --- PIN PWM BTS7960 ---
// Motor Penggerak (Modul #1)
#define PIN_DRIVE_FWD 26 
#define PIN_DRIVE_REV 27

// Motor Setir (Modul #2)
#define PIN_STEER_RIGHT 12
#define PIN_STEER_LEFT  13

// --- PIN ENABLE BTS7960 ---
// Motor Penggerak
#define PIN_DRIVE_REN 32
#define PIN_DRIVE_LEN 33

// Motor Setir
#define PIN_STEER_REN 14
#define PIN_STEER_LEN 15

// PWM CONFIG
const int pwmFreq = 5000;
const int pwmRes = 8;

// Variabel motor
int maxDriveSpeed = 200;
int targetDriveSpeed = 0;
int currentDriveSpeed = 0;
int driveDirection = 0;

int maxSteerSpeed = 255;
int targetSteerSpeed = 0;
int currentSteerSpeed = 0;
int steerDirection = 0;

const int ACCEL_STEP = 10;
// const int ACCEL_INTERVAL = 20;
const int ACCEL_INTERVAL = 500;
unsigned long lastAccelTime = 0;

void setup() {
  Serial.begin(115200);
  SerialBT.begin("Mobil_ESP32_BTS7960");
  Serial.println("Driver BTS7960 Siap. Kirim WSAD.");

  // --- Set pin EN sebagai OUTPUT ---
  pinMode(PIN_DRIVE_REN, OUTPUT);
  pinMode(PIN_DRIVE_LEN, OUTPUT);
  pinMode(PIN_STEER_REN, OUTPUT);
  pinMode(PIN_STEER_LEN, OUTPUT);

  // Matikan semua EN saat boot (AMAN)w
  digitalWrite(PIN_DRIVE_REN, LOW);
  digitalWrite(PIN_DRIVE_LEN, LOW);
  digitalWrite(PIN_STEER_REN, LOW);
  digitalWrite(PIN_STEER_LEN, LOW);

  // --- Set PWM ---
  ledcAttach(PIN_DRIVE_FWD,  pwmFreq, pwmRes);
  ledcAttach(PIN_DRIVE_REV,  pwmFreq, pwmRes);
  ledcAttach(PIN_STEER_RIGHT, pwmFreq, pwmRes);
  ledcAttach(PIN_STEER_LEFT,  pwmFreq, pwmRes);

  // Matikan PWM
  ledcWrite(PIN_DRIVE_FWD, 0);
  ledcWrite(PIN_DRIVE_REV, 0);
  ledcWrite(PIN_STEER_RIGHT, 0);
  ledcWrite(PIN_STEER_LEFT, 0);
}

// =========================================================
// MOTOR PENGGERAK
// =========================================================
void handleDriveAcceleration() {
  if (currentDriveSpeed < targetDriveSpeed) {
    currentDriveSpeed += ACCEL_STEP;
    if (currentDriveSpeed > targetDriveSpeed) currentDriveSpeed = targetDriveSpeed;
  } else if (currentDriveSpeed > targetDriveSpeed) {
    currentDriveSpeed -= ACCEL_STEP;
    if (currentDriveSpeed < targetDriveSpeed) currentDriveSpeed = targetDriveSpeed;
  }

  if (currentDriveSpeed == 0) {
    // Stop
    ledcWrite(PIN_DRIVE_FWD, 0);
    ledcWrite(PIN_DRIVE_REV, 0);
    digitalWrite(PIN_DRIVE_REN, LOW);
    digitalWrite(PIN_DRIVE_LEN, LOW);
    driveDirection = 0;
    return;
  }

  // Enable dulu
  digitalWrite(PIN_DRIVE_REN, HIGH);
  digitalWrite(PIN_DRIVE_LEN, HIGH);

  if (driveDirection == 1)  {  // MAJU
    ledcWrite(PIN_DRIVE_FWD, currentDriveSpeed);
    ledcWrite(PIN_DRIVE_REV, 0);
  } else if (driveDirection == -1) {  // MUNDUR
    ledcWrite(PIN_DRIVE_FWD, 0);
    ledcWrite(PIN_DRIVE_REV, currentDriveSpeed);
  }
}

// =========================================================
// MOTOR SETIR
// =========================================================
void handleSteerAcceleration() {
  if (currentSteerSpeed < targetSteerSpeed) {
    currentSteerSpeed += ACCEL_STEP;
    if (currentSteerSpeed > targetSteerSpeed) currentSteerSpeed = targetSteerSpeed;
  } else if (currentSteerSpeed > targetSteerSpeed) {
    currentSteerSpeed -= ACCEL_STEP;
    if (currentSteerSpeed < targetSteerSpeed) currentSteerSpeed = targetSteerSpeed;
  }

  if (currentSteerSpeed == 0) {
    ledcWrite(PIN_STEER_RIGHT, 0);
    ledcWrite(PIN_STEER_LEFT, 0);
    digitalWrite(PIN_STEER_REN, LOW);
    digitalWrite(PIN_STEER_LEN, LOW);
    steerDirection = 0;
    return;
  }

  // Aktifkan EN
  digitalWrite(PIN_STEER_REN, HIGH);
  digitalWrite(PIN_STEER_LEN, HIGH);

  if (steerDirection == 1) { // KANAN
    ledcWrite(PIN_STEER_RIGHT, currentSteerSpeed);
    ledcWrite(PIN_STEER_LEFT, 0);
  } else if (steerDirection == -1) { // KIRI
    ledcWrite(PIN_STEER_RIGHT, 0);
    ledcWrite(PIN_STEER_LEFT, currentSteerSpeed);
  }
}

// =========================================================
// LOOP
// =========================================================
void loop() {
  if (SerialBT.available()) {
    char cmd = SerialBT.read();
    Serial.println(cmd);

    switch (cmd) {
      case 'W':
        driveDirection = 1;
        targetDriveSpeed = maxDriveSpeed;
        break;
      case 'S':
        driveDirection = -1;
        targetDriveSpeed = maxDriveSpeed;
        break;
      case 'w':
      case 's':
        targetDriveSpeed = 0;
        break;

      case 'D':
        steerDirection = 1;
        targetSteerSpeed = maxSteerSpeed;
        break;
      case 'A':
        steerDirection = -1;
        targetSteerSpeed = maxSteerSpeed;
        break;
      case 'd':
      case 'a':
        targetSteerSpeed = 0;
        break;
    }
  }

  if (millis() - lastAccelTime > ACCEL_INTERVAL) {
    lastAccelTime = millis();
    handleDriveAcceleration();
    handleSteerAcceleration();
  }
}
