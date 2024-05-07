#include <util/atomic.h>

#define X_STEP_PIN 9
#define X_DIR_PIN 3
#define X_ENABLE_PIN 4 // Define el pin de habilitación para el motor en el eje X
#define Y_STEP_PIN 11
#define Y_DIR_PIN 6
#define Y_ENABLE_PIN 7 // Define el pin de habilitación para el motor en el eje Y

volatile int totalStepsX = 0; 
volatile int totalStepsY = 0; 

const float distancia_por_paso = (3.1416 * 2.54) * 1.8 / 360; // cm/paso (2.54 cm por pulgada)

bool dirX;
bool dirY;

void setDirectionMotorX(bool dir){
  dirX = dir;
  digitalWrite(X_DIR_PIN, dirX);
}

void setDirectionMotorY(bool dir){
  dirY = dir;
  digitalWrite(Y_DIR_PIN, dirY);
}

void setSpeedMotorX(float speed){
  OCR1A = speed;
  TCNT1 = 0;
}

void setSpeedMotorY(float speed){
  OCR2A = speed;
  TCNT2 = 0;
}

void enableMotorX() {
  digitalWrite(X_ENABLE_PIN, LOW);
} 

void disableMotorX() {
  digitalWrite(X_ENABLE_PIN, HIGH);
} 

void enableMotorY() {
  digitalWrite(Y_ENABLE_PIN, LOW);
} 

void disableMotorY() {
  digitalWrite(Y_ENABLE_PIN, HIGH);
} 

long getStepsX(void){
  long aux;
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    aux = totalStepsX;
  };
  return aux;
}

long getStepsY(void){
  long aux;
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    aux = totalStepsY;
  };
  return aux;
}

void setStepsX(int steps) {
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    totalStepsX = steps;
  }
}

void setStepsY(int steps) {
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    totalStepsY = steps;
  }
}

void moveToHomePosition() {
  int stepsX = (25.0 / distancia_por_paso) / 2; 
  int stepsY = (17.0 / distancia_por_paso) / 2; 
  
  setDirectionMotorX(true);
  setDirectionMotorY(true);
  
  setSpeedMotorX(200);
  setSpeedMotorY(255);
  
  enableMotorX();
  enableMotorY();
  
  // Espera a que ambos motores lleguen a la posición de inicio
  while (getStepsX() < stepsX || getStepsY() < stepsY) {
  }

  disableMotorX();
  disableMotorY();
}

// =========================================================
// =========================================================

void setup() {
  Serial.begin(9600);

  pinMode(X_STEP_PIN, OUTPUT);
  pinMode(X_DIR_PIN, OUTPUT);
  pinMode(X_ENABLE_PIN, OUTPUT);
  pinMode(Y_STEP_PIN, OUTPUT);
  pinMode(Y_DIR_PIN, OUTPUT);
  pinMode(Y_ENABLE_PIN, OUTPUT); 
  
  digitalWrite(X_ENABLE_PIN, LOW); 
  digitalWrite(Y_ENABLE_PIN, LOW); 

  setDirectionMotorX(true);
  setDirectionMotorY(true);
  configurarTimer1(255); 
  configurarTimer2(255);

  totalStepsX = 0;
  totalStepsY = 0;
  
  moveToHomePosition();
}

void loop() {
  int hall1 = analogRead(A0);
  int hall2 = analogRead(A1);
  int hall3 = analogRead(A2);
  int hall4 = analogRead(A3);

  int errX = (hall1 + hall2) - (hall3 + hall4); // error del eje x
  int errY = (hall1 + hall4) - (hall2 + hall3); // error del eje y

  // Ganancias del controlador PI
  float kp = 0.148914955780646;
  float ki = 8938.3224092643;


  static float integralErrorX = 0;
  static float integralErrorY = 0;

  integralErrorX += errX;
  integralErrorY += errY;

  float outputX = kp * errX + ki * integralErrorX;
  float outputY = kp * errY + ki * integralErrorY;

  // Control de la velocidad de los motores basado en la salida del controlador PI
  setSpeedMotorX(outputX);
  setSpeedMotorY(outputY);
}
