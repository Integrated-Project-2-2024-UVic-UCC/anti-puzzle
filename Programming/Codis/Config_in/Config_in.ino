#include <util/atomic.h>
#include <EEPROM.h>


#define X_STEP_PIN 9
#define X_DIR_PIN 3
#define X_ENABLE_PIN 4 // Define el pin de habilitación para el motor en el eje X
#define Y_STEP_PIN 11
#define Y_DIR_PIN 6
#define Y_ENABLE_PIN 7 // Define el pin de habilitación para el motor en el eje Y

// Direcciones en la memoria EEPROM donde se guardarán las coordenadas
#define EEPROM_X_ADDRESS 0
#define EEPROM_Y_ADDRESS 2

int X = 105; // Coordenada X deseada
int Y = 85;  // Coordenada Y deseada

const int hall1Pin = A0; // Sensor Hall 1 (arriba izquierda)
const int hall2Pin = A1; // Sensor Hall 2 (arriba derecha)
const int hall3Pin = A2; // Sensor Hall 3 (abajo izquierda)
const int hall4Pin = A3; // Sensor Hall 4 (abajo derecha)

volatile int totalStepsX = 0; 
volatile int totalStepsY = 0; 

const float distancia_por_paso = (3.1416 * 2.54) * 1.8 / 360; // cm/paso (2.54 cm por pulgada)

bool dirX;
bool dirY;

void Guardar_Memoria(int x, int y) {
  int posX = EEPROM.read(EEPROM_X_ADDRESS);
  int posY = EEPROM.read(EEPROM_Y_ADDRESS);
  
  if (posX != x || posY != y) {
    EEPROM.write(EEPROM_X_ADDRESS, x);
    EEPROM.write(EEPROM_Y_ADDRESS, y);
  } 
}

// ················································
// Timers i preescales
// ················································
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
// ················································
// ················································

void enableMotorX() {digitalWrite(X_ENABLE_PIN, LOW);} 

void disableMotorX() {digitalWrite(X_ENABLE_PIN, HIGH);} 

void enableMotorY() {digitalWrite(Y_ENABLE_PIN, LOW);} 

void disableMotorY() {digitalWrite(Y_ENABLE_PIN, HIGH);} 

// Geters
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

long getSpeedX(){
  long speedX;
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    speedX = OCR1A;
  };
  return speedX;
}

long getSpeedY(){
  long speedY; 
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    speedY = OCR2A;
  };
  return speedY;
}

// Seters 
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

// void setStepsX(int steps) {
//   ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
//     totalStepsX += steps;
//   }
// }

// void setStepsY(int steps) {
//   ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
//     totalStepsY += steps;
//   }
// }

void CI() {
  // Paso 1: Mover los motores a la mitad de las coordenadas
  int mitadX = 42780 / 2; // Mitad de la coordenada X que son 42.780 micropasos
  int mitadY = 34624 / 2; // Mitad de la coordenada Y que son 34.624 micropasos

  float Vx = mitadX / 5.0; 
  float Vy = mitadY / 5.0; 

  setDirectionMotorX(true);
  setDirectionMotorY(true);
  
  setSpeedMotorX(Vx);
  setSpeedMotorY(Vy);

  enableMotorX();
  enableMotorY();

  while (!(getStepsX() >= mitadX && getStepsY() >= mitadY)) {}

  disableMotorX();
  disableMotorY();

  // Paso 2: Mover el motor X hacia la derecha para completar los 210mm
  // Calcular los pasos necesarios para recorrer los 210mm en X
  int pasosX = 42.780;
  enableMotorX();
  while (!(getStepsX() >= pasosX)){}
  disableMotorX();

  // Paso 3: Guardar las coordenadas en la memoria EEPROM
  Guardar_Memoria(pasosX, mitadY);
}


// Función para manejar la interrupción del Timer 1
ISR(TIMER1_COMPA_vect) {if(dirX) {totalStepsX++;} else {totalStepsX--;}}

// Función para manejar la interrupción del Timer 2
ISR(TIMER2_COMPA_vect) {}

void setup() {
  Serial.begin(9600);

  int posX = EEPROM.read(EEPROM_X_ADDRESS);
  int posY = EEPROM.read(EEPROM_Y_ADDRESS);
  
  pinMode(X_STEP_PIN, OUTPUT);
  pinMode(X_DIR_PIN, OUTPUT);
  pinMode(X_ENABLE_PIN, OUTPUT);
  pinMode(Y_STEP_PIN, OUTPUT);
  pinMode(Y_DIR_PIN, OUTPUT);
  pinMode(Y_ENABLE_PIN, OUTPUT); 

  pinMode(hall1Pin, INPUT);
  pinMode(hall2Pin, INPUT);
  pinMode(hall3Pin, INPUT);
  pinMode(hall4Pin, INPUT);
  
  setDirectionMotorX(true);
  setDirectionMotorY(true);
  configurarTimer1(255); 
  configurarTimer2(255);

  totalStepsX = 0;
  totalStepsY = 0;
}
void loop() {
  CI();
}