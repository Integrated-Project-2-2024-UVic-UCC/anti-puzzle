#include <util/atomic.h>
#include <EEPROM.h>

// Pins motor X
#define X_STEP_PIN 9
#define X_DIR_PIN 3
#define X_ENABLE_PIN 4

// Pins motor Y
#define Y_STEP_PIN 11
#define Y_DIR_PIN 6
#define Y_ENABLE_PIN 7

#define DX_epin 10 // Nuevo detector de final de carrera en el eje X
#define DY_spin 8  // Nuevo detector de final de carrera en el eje Y
#define DX_dpin A4 // Nuevo detector de final de carrera en el eje X
#define DY_ipin A5 // Nuevo detector de final de carrera en el eje Y

// Direcciones en la memoria EEPROM donde se guardarán las coordenadas
#define EEPROM_X_ADDRESS 0
#define EEPROM_Y_ADDRESS 2

const int hall1Pin = A0; // Sensor Hall 1 (arriba izquierda)
const int hall2Pin = A1; // Sensor Hall 2 (arriba derecha)
const int hall3Pin = A2; // Sensor Hall 3 (abajo izquierda)
const int hall4Pin = A3; // Sensor Hall 4 (abajo derecha)

volatile int totalStepsX; 
volatile int totalStepsY; 

bool dirX;
bool dirY;

// *********************************
// Incedeix a memoria
// *********************************

void Guardar_Memoria(int x, int y) {
  int posX = EEPROM.read(EEPROM_X_ADDRESS);
  int posY = EEPROM.read(EEPROM_Y_ADDRESS);
  
  if (posX != x && posY != y) {
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
// Habilita i deshabilita motors
// ················································

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

// ################################################
// Geters
// ################################################

int getDX_d(){
  int DX_d = digitalRead(DX_dpin); // Cambio el pin al detector nuevo
  return DX_d;
}

int getDX_e(){
  int DX_e = digitalRead(DX_epin); // Cambio el pin al detector nuevo
  return DX_e;
}

int getDY_i(){
  int DY_i = digitalRead(DY_ipin); // Cambio el pin al detector nuevo
  return DY_i;
}

int getDY_s(){
  int DY_s = digitalRead(DY_spin); // Cambio el pin al detector nuevo
  return DY_s;
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

// ################################################
// Seters 
// ################################################

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

// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
// Programa  ---  CI. ---  HOME. ----  PosicioIman.  
// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

void CI() {
  // Paso 1: Mover los motores a la mitad de las coordenadas
  int mitadX = (15300)/2; // Mitad de la coordenada Y que son 15300 micropasos
  int mitadY = (8200)/2; // Mitad de la coordenada X que son 8200 micropasos --> 3200

  int Vx = mitadY / 75;
  int Vy = mitadX / 75;

  setDirectionMotorX(true);
  setDirectionMotorY(true);
  
  setSpeedMotorX(Vx);
  setSpeedMotorY(Vy);

  enableMotorX();
  enableMotorY();

  while ((getStepsY() < mitadY) && (getStepsX() < mitadX)) {}

  disableMotorX();
  disableMotorY();

  // Paso 2: Mover el motor X hacia la derecha para completar los 210mm
  // Calcular los pasos necesarios para recorrer los 210mm en X
  int pasX = getStepsX() + mitadX;
  enableMotorX();
  while ((getStepsX() < pasX)){
    delay(100);
    int difdist = pasX - getStepsX();
    float velocidadX = map(difdist, 0, pasX, 225, 16);
    setSpeedMotorX(velocidadX);
  }
  disableMotorX();

  // Paso 3: Guardar las coordenadas en la memoria EEPROM
  Guardar_Memoria(getStepsX(), getStepsY());
}

void Home() {
  int homeX = (15418)/2; // Mitad de la coordenada Y que son 15300 micropasos
  int homeY = (8200)/2; // Mitad de la coordenada X que son 8200 micropasos --> 3200 4100

  int Vx = homeY / 75;
  int Vy = homeX / 75;

  // Paso 1: Mover el motor X a la posición de inicio en X
  setDirectionMotorX(false); 
  setDirectionMotorY(false); 

  setSpeedMotorX(100);
  setSpeedMotorY(100);

  enableMotorX();

  // Mover motor X
  while (!getDX_d() || getDX_e()) {} 
  disableMotorX();

  enableMotorY();

  // Mover motor Y
  while (!getDY_i() || getDY_s()) {}
  disableMotorY();

  setSpeedMotorX(Vx);
  setSpeedMotorY(Vy);

  if (getDX_d() == HIGH){
    setDirectionMotorX(false); 
  }else if (getDX_e() == HIGH){
    setDirectionMotorX(true);
  }
  
  if (getDY_i() == HIGH){
    setDirectionMotorY(false);
  }else if (getDY_s() == HIGH){
    setDirectionMotorY(true);
  }

  enableMotorX();
  enableMotorY();
  while (getStepsY() < homeX){}//&& (getStepsY() <= homeY)) {}

  disableMotorX();
  disableMotorY();
}

int PosicioIman(){
  // 200 pasos per 16micropasoso = 3200micropasos 1 volta
  int hall1 = analogRead(hall1Pin);
  int hall2 = analogRead(hall2Pin);
  int hall3 = analogRead(hall3Pin);
  int hall4 = analogRead(hall4Pin);

  int EX = CalculerrorX(hall1,hall2,hall3,hall4);
  int EY = CalculerrorY(hall1,hall2,hall3,hall4);

  bool dirX = EX > 0; // Si errX es positivo, el imán se mueve en la dirección positiva de X
  bool dirY = EY > 0; // Si errY es positivo, el imán se mueve en la dirección positiva de Y

  float moveX = map(EX, 505, 1024, 0, 8250); // Es posible que els valors dels pasos s'haigui de cambiar perque no faigui un salt tant gran 
  float moveY = map(EY, 505, 1024, 0, 4570); // Es posible que els valors dels pasos s'haigui de cambiar perque no faigui un salt tant gran 

  if (moveX == 0 && moveY == 0) {Guardar_Memoria(getStepsX(), getStepsY());} 
  
  Movemotors(moveX, moveY, dirX, dirY);
}

int CalculerrorX(int hall1,int hall2,int hall3,int hall4){
  int errX = (hall1 + hall2) - (hall3 + hall4);
  return errX;
}

int CalculerrorY(int hall1,int hall2,int hall3,int hall4){
  int errY = (hall1 + hall4) - (hall2 + hall3);
  return errY;
}

void Movemotors(float posX, float posY, bool dirX, bool dirY) {
  // Mover el motor X
  Movepos(posX, dirX, posY, dirY);

  disableMotorX();
  disableMotorY();
}

void Movepos(float stepsX, bool dirX, float stepsY, bool dirY) {
  
  int Vx = stepsY / 75;
  int Vy = stepsX / 75;

  setDirectionMotorX(dirX);
  setDirectionMotorY(dirY);
  
  enableMotorX();
  enableMotorY();

  while (getStepsX() < stepsX ){//&& getStepsY() >= stepsY)) {
    // Configurar la velocidad del motor X en función de la distancia restante
    delay(100);
    float velX = map(stepsX - getStepsX(), 0, stepsX, 225, Vx);
    float velY = map(stepsY - getStepsY(), 0, stepsY, 225, Vx);
    setSpeedMotorX(velX);
    setSpeedMotorY(velY);
  }
}

// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
// Interrupcions
// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&

// Función para manejar la interrupción del Timer 1
ISR(TIMER1_COMPA_vect) {
  if(dirX) {totalStepsX++;} else {totalStepsX--;}
}

// Función para manejar la interrupción del Timer 2
ISR(TIMER2_COMPA_vect) {
  if(dirY) {totalStepsY++;} else {totalStepsY--;}
}

// 
// Codi Principal
// 

void setup() {
  Serial.begin(9600);

  pinMode(X_STEP_PIN, OUTPUT);
  pinMode(X_DIR_PIN, OUTPUT);
  pinMode(X_ENABLE_PIN, OUTPUT);
  pinMode(Y_STEP_PIN, OUTPUT);
  pinMode(Y_DIR_PIN, OUTPUT);
  pinMode(Y_ENABLE_PIN, OUTPUT); 

  pinMode(DX_epin, OUTPUT);
  pinMode(DY_spin, OUTPUT);
  pinMode(DX_dpin, OUTPUT);
  pinMode(DY_ipin, OUTPUT);

  pinMode(hall1Pin, INPUT);
  pinMode(hall2Pin, INPUT);
  pinMode(hall3Pin, INPUT);
  pinMode(hall4Pin, INPUT);

  pinMode(DX, INPUT_PULLUP); // Configura el pin DX como entrada con resistencia de pull-up
  pinMode(DY, INPUT_PULLUP);
  
  configurarTimer1(255); 
  configurarTimer2(1);
  disableMotorX();
  disableMotorY();

  totalStepsX = 0;
  totalStepsY = 0;
  // CI();
  Home();
}

void loop() {
  // PosicioIman();
}