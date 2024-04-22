#include <util/atomic.h>

#define X_STEP_PIN 9
#define X_DIR_PIN 3
#define X_ENABLE_PIN 4 // Define el pin de habilitación para el motor en el eje X
#define Y_STEP_PIN 11
#define Y_DIR_PIN 6
#define Y_ENABLE_PIN 7 // Define el pin de habilitación para el motor en el eje Y
#define pot1 A4
#define pot2 A5

volatile int totalStepsX = 0; 
volatile int totalStepsY = 0; 

const float distancia_por_paso = (3.1416 * 2.54) * 1.8 / 360; // cm/paso (2.54 cm por pulgada)

bool dirX;
bool dirY;

void setDirectionMotorX(bool dir){
  dirX = dir;
  digitalWrite(X_DIR_PIN,dirX);
}

void setDirectionMotorY(bool dir){
  dirY = dir;
  digitalWrite(Y_DIR_PIN,dirX);
}

unsigned int Preescaler(unsigned int frecuenciaDeseada){
  unsigned long frecuenciaReloj = 16000000UL; // Frecuencia del reloj del ATmega328P en Hz
  unsigned long contadorMaximo =  65535UL; // Contador máximo para los timers de 16 bits
  
  unsigned int preescaler = frecuenciaReloj / ((frecuenciaDeseada) * contadorMaximo * 2) - 1;
  unsigned long frecuenciaTimer1 = frecuenciaReloj / preescaler;

  return frecuenciaTimer1;
}

void configurarTimer1(unsigned int frecuenciaDeseada) {

  TCCR1A = 0; 
  TCCR1B = 0;
  TCNT1  = 0; 

  OCR1A = Preescaler(frecuenciaDeseada);
  // Utilitzem un preescaler de 8 bits perque ens permet mes control en freq altes i per tant ens permet mes velocitat 
  TCCR1A |= (1 << COM1A0); 
  TCCR1B |= (1 << WGM12); 
  TCCR1B |= (0 << CS12); 
  TCCR1B |= (1 << CS11); 
  TCCR1B |= (1 << CS10); 
  TIMSK1 |= (1 << OCIE1A); 
}

void configurarTimer2(unsigned int frecuenciaDeseada) {

  TCCR2A = 0; // Configuración inicial de los registros
  TCCR2B = 0;
  TCNT2  = 0; // Inicializamos el contador del timer 2

  OCR2A = Preescaler(frecuenciaDeseada); // Configuramos el preescaler

  TCCR2A |= (1 << COM2A0); // Configuramos el Timer 2 para el modo CTC (Clear Timer on Compare Match) y activamos la salida OC2A
  TCCR2A |= (1 << WGM21); // Configuramos el Timer 2 para el modo CTC
  TCCR2B |= (0 << CS20);
  TCCR2B |= (0 << CS21); // Configuramos el prescaler para que el timer cuente a la frecuencia del reloj
  TCCR2B |= (1 << CS22);
  TIMSK2 |= (1 << OCIE2A); // Habilitamos la interrupción de comparación del Timer 2
}

void setSpeedMotorX(float speed){
  OCR1A = speed;
  TCNT1 = 0;
}

void setSpeedMotorY(float speed){
  OCR2A = speed;
  TCNT2 = 0;
}

void enableMotorX() {digitalWrite(X_ENABLE_PIN, LOW);} 

void disableMotorX() {digitalWrite(X_ENABLE_PIN, HIGH);} 

void enableMotorY() {digitalWrite(Y_ENABLE_PIN, LOW);} 

void disableMotorY() {digitalWrite(Y_ENABLE_PIN, HIGH);} 

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

// Función para manejar la interrupción del Timer 1
ISR(TIMER1_COMPA_vect) {
  if(dirX) {totalStepsX++;} else {totalStepsX--;}
}

// Función para manejar la interrupción del Timer 2
ISR(TIMER2_COMPA_vect) {}

// =========================================================
// =========================================================

void moveToHomePosition() {
  int stepsX = (25.0 / distancia_por_paso) / 2; 
  
  int stepsY = (17.0 / distancia_por_paso) / 2; 
  
  setDirectionMotorX(true);
  setDirectionMotorY(true);
  
  setSpeedMotorX(200);
  setSpeedMotorY(255);
  
  enableMotorX();
  enableMotorY();
  
  // Espera a que ambos motor arribin a la pos 
  while (getStepsX() < stepsX || getStepsY() < stepsY) {
  }

  disableMotorX();
  disableMotorY();
}

// =========================================================
// =========================================================

void setup() {
  Serial.begin(9600);
  pinMode(pot1, INPUT);
  pinMode(pot2, INPUT);
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

// =========================================================
// =========================================================

void loop() {
  int lect1 = 100; //map(analogRead(pot1),1,1024, 16, 256);
  int lect2 = 16; //map(analogRead(pot2),1,1024, 16, 256);
  
  setSpeedMotorY(lect2);

  if(getStepsX()>6000){
    setSpeedMotorX(16);
  } else if (getStepsX()>4000) {
     setSpeedMotorX(255);
     Serial.println(getStepsX());
  } else {
    setSpeedMotorX(100);
    Serial.println(getStepsX());
  }
}

