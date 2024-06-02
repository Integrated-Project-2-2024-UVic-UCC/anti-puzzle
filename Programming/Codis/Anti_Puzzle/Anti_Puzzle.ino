#include <util/atomic.h>
#include <EEPROM.h>

 /*
 __________________
/                  \  A0 = sensor inferior dret
|  s4          s2  |  A4 = senso inferior esquerre
|                  |  A3 = sensor superior dret
|                  |  A2 = sensor superior esquerre
|  s3          s1  |
\__________________/
*/

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
#define DX_dpin 5 // Nuevo detector de final de carrera en el eje X
#define DY_ipin 2 // Nuevo detector de final de carrera en el eje Y

const int pinLED_Rojo = 8;    // Pin para el LED rojo
const int pinLED_Verde = 9;    // Pin para el LED verde
const int pinInterruptor1 = 2; // Pin para el primer contacto del interruptor (posición 1)
const int pinInterruptor2 = 3; // Pin para el segundo contacto del interruptor (posición 3)

float A1324_1 = A0; 
float A1324_2 = A1;
float A1324_3 = A2;
float A1324_4 = A3;

volatile long totalStepsX, totalStepsY; 

bool dirX;
bool dirY;

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

bool enableMotorX() {
  digitalWrite(X_ENABLE_PIN, LOW);
} 

bool disableMotorX() {
  digitalWrite(X_ENABLE_PIN, HIGH);
} 

bool enableMotorY() {
  digitalWrite(Y_ENABLE_PIN, LOW);
} 

bool disableMotorY() {
  digitalWrite(Y_ENABLE_PIN, HIGH);
} 

// ################################################
// Geters
// ################################################

bool getDX_d(){
  bool DX_d = digitalRead(DX_dpin); // Cambio el pin al detector nuevo
  return DX_d;
}

bool getDX_e(){
  bool DX_e = digitalRead(DX_epin); // Cambio el pin al detector nuevo
  return DX_e;
}

bool getDY_i(){
  bool DY_i = digitalRead(DY_ipin); // Cambio el pin al detector nuevo
  return DY_i;
}

bool getDY_s(){
  bool DY_s = digitalRead(DY_spin); // Cambio el pin al detector nuevo
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

// =================================================
// Colors LED
// =================================================

void encenderVerde() {
  digitalWrite(pinLED_Rojo, HIGH);   // Apagar LED rojo
  digitalWrite(pinLED_Verde, LOW); // Encender LED verde
}

void encenderNaranja() {
  digitalWrite(pinLED_Rojo, LOW);  // Encender LED rojo
  digitalWrite(pinLED_Verde, LOW); // Encender LED verde 
}

void encenderRojo() {
  digitalWrite(pinLED_Rojo, LOW);  // Encender LED rojo
  digitalWrite(pinLED_Verde, HIGH);  // Apagar LED verde
}

// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
// Programa  ---  CI. ---  HOME. ----  Nivells.  
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
}

void Home() {
  // Paso 1: Mover los motores a la mitad de las coordenadas
  int mitadX = (15300)/2; // Mitad de la coordenada Y que son 15300 micropasos
  int mitadY = (8200)/2; // Mitad de la coordenada X que son 8200 micropasos --> 3200

  int Vx = mitadY / 75;
  int Vy = mitadX / 75;

  // Paso 1: Mover el motor X a la posición de inicio en X
  setDirectionMotorX(false); 
  setDirectionMotorY(false); 

  setSpeedMotorX(150);
  setSpeedMotorY(150);

  enableMotorX();
  
  // Mover motor X
  while (getDX_e()) {
    Serial.println("move");
  } 
  // Serial.println(getDX_d());
  disableMotorX();

  enableMotorY();

  // Mover motor Y
  while (getDY_s()) {}
  // Serial.println(getDY_s());
  disableMotorY();

  totalStepsX = 0;
  totalStepsY = 0;

  MoveToPosition(mitadX,mitadY);
}

void Print(float V1, float V2, float V3, float V4){
  Serial.print(0);
  Serial.print(" ");
  Serial.print(V1);
  Serial.print(" ");
  Serial.print(V2+1000);
  Serial.print(" ");
  Serial.print(V3+2000);
  Serial.print(" ");
  Serial.print(V4+3000);
  Serial.print(" ");
  Serial.print(4000);
  Serial.println(" ");
}

void Nivell_1(){
  int mitadX = 15300 / 2; // Mitad de la coordenada Y que son 15300 micropasos
  int mitadY = 8200 / 2;  // Mitad de la coordenada X que son 8200 micropasos --> 3200
  int stepX = 0;
  int stepY = 0; 

  int val1 = analogRead(A1324_1);
  int val2 = analogRead(A1324_2);
  int val3 = analogRead(A1324_3);
  int val4 = analogRead(A1324_4);

  int eX = CalculerrorX(val1,val2,val3,val4);
  int eY = CalculerrorY(val1,val2,val3,val4);

  // Print(val1, val2, val3, val4);

  int win = val1 + val2 + val3 + val4;

  int sX = 0;
  int sY = 0;
  if (win != 3900) {
    if (eX > 10) {
      sX = -map(abs(eX), 10, 2000, 0, mitadX);
    } else if (eX < -10) {
      sX = -map(abs(eX), -2000, -10, -mitadX, 0);
    }
    if (eY > 10) {
      sY = -map(abs(eY), 10, 2000, 0, mitadY);
    } else if (eY < -10) {
      sY = -map(abs(eY), -2000, -10, -mitadY, 0);
    }
  }

  MoveToPosition(sX, sY);
}

void Nivell_2() {
  int mitadX = 15300 / 2; // Mitad de la coordenada Y que son 15300 micropasos
  int mitadY = 8200 / 2;  // Mitad de la coordenada X que son 8200 micropasos --> 3200
  int stepX = 0;
  int stepY = 0; 

  int val1 = analogRead(A1324_1);
  int val2 = analogRead(A1324_2);
  int val3 = analogRead(A1324_3);
  int val4 = analogRead(A1324_4);

  int eX = CalculerrorX(val1,val2,val3,val4);
  int eY = CalculerrorY(val1,val2,val3,val4);

  int win = val1 + val2 + val3 + val4;

  int sX = 0;
  int sY = 0;

  if (win != 3900) {
    if (eX > 10) {
      sX = map(abs(eX), 10, 2000, 0, mitadX);
    } else if (eX < -10) {
      sX = map(abs(eX), -2000, -10, -mitadX, 0);
    }
    if (eY > 10) {
      sY = map(abs(eY), 10, 2000, 0, mitadY);
    } else if (eY < -10) {
      sY = map(abs(eY), -2000, -10, -mitadY, 0);
    }
  }

  MoveToPosition(sX, sY);
}

void Nivell_3() {
  int mitadX = 15300 / 2; // Mitad de la coordenada Y que son 15300 micropasos
  int mitadY = 8200 / 2;  // Mitad de la coordenada X que son 8200 micropasos --> 3200
  int sX = 0;
  int sY = 0; 

  int val1 = analogRead(A1324_1);
  int val2 = analogRead(A1324_2);
  int val3 = analogRead(A1324_3);
  int val4 = analogRead(A1324_4);

  int eX = CalculerrorX(val1,val2,val3,val4);
  int eY = CalculerrorY(val1,val2,val3,val4);

  int win = val1 + val2 + val3 + val4;

  if (win != 3900) {
    if (abs(eX) > 10 || abs(eY) > 10) {
      sX = random(-mitadX, mitadX);
      sY = random(-mitadY, mitadY);
    }
  }

  MoveToPosition(sX, sY);
}


int CalculerrorX(int hall1,int hall2,int hall3,int hall4){
  int errX = (hall3 + hall4) - (hall2 + hall1);
  return errX;
}

int CalculerrorY(int hall1,int hall2,int hall3,int hall4){
  int errY = (hall2 + hall4) - (hall1 + hall3);
  return errY;
}

void MoveToPosition(long stepX, long stepY) {
  // Serial.print(stepX);
  // Serial.print(" : ");
  // Serial.print(stepX);
  // Serial.println("");

  long errX=10000;
  long errY=10000;
  long periode;

  while(errX>100 || errY>100) {
    delay(100);
    // control eix X
    errX = stepX - totalStepsX;
    setDirectionMotorX(errX>0);

    if (abs(errX)<=100) {
      periode = 255;
    } else if (abs(errX)>3200) {
      periode = 16;
    } else {
      periode = map(abs(errX),100, 3200, 255, 50);
    }
    setSpeedMotorX(periode);

    if (errX>100 && getDX_d() || errX<-100 && getDX_e()) {
      enableMotorX();
    } else {
      disableMotorX();
    }

    // control eix Y
    errY = stepY - totalStepsY;
    setDirectionMotorY(errY>0);

    if (abs(errY)<=100) {
      periode = 255;
    } else if (abs(errY)>3200) {
      periode = 50;
    } else {
      periode = map(abs(errY),100, 3200, 255, 50);
    }
    setSpeedMotorY(periode);

    if (errY>100 && getDY_i() || errY<-100 && getDY_s()) {
      enableMotorY();
    } else {
      disableMotorY();
    }
  }
  // Serial.println("at position!!!");
}

// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
// Interrupcions
// &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&

// Función para manejar la interrupción del Timer 1
ISR(TIMER1_COMPA_vect) {
  if (digitalRead(X_ENABLE_PIN) == LOW){
    if(dirX) {totalStepsX++;} else {totalStepsX--;}
  }
}

// Función para manejar la interrupción del Timer 2
ISR(TIMER2_COMPA_vect) {
  if (digitalRead(Y_ENABLE_PIN) == LOW){
    if(dirY) {totalStepsY++;} else {totalStepsY--;}
  }
}

// 
// Codi Principal
// 

void setup() {
  Serial.begin(9600);

  pinMode(X_STEP_PIN, OUTPUT);
  pinMode(X_DIR_PIN, OUTPUT);
  pinMode(X_ENABLE_PIN, OUTPUT);
  pinMode(Y_STEP_PIN, OUTPUT);
  pinMode(Y_DIR_PIN, OUTPUT);
  pinMode(Y_ENABLE_PIN, OUTPUT);

  pinMode(pinLED_Rojo, OUTPUT);
  pinMode(pinLED_Verde, OUTPUT);

  pinMode(pinInterruptor1, INPUT_PULLUP);
  pinMode(pinInterruptor2, INPUT_PULLUP);

  pinMode(DX_epin,INPUT_PULLUP);
  pinMode(DY_spin,INPUT_PULLUP);
  pinMode(DX_dpin,INPUT_PULLUP);
  pinMode(DY_ipin,INPUT_PULLUP);

  pinMode(A1324_1, INPUT);
  pinMode(A1324_2, INPUT);
  pinMode(A1324_3, INPUT);
  pinMode(A1324_4, INPUT);
  
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
  int estadoInterruptor1 = digitalRead(pinInterruptor1);
  int estadoInterruptor2 = digitalRead(pinInterruptor2);

  if (estadoInterruptor1 == LOW && estadoInterruptor2 == HIGH) {
    encenderRojo();
    Nivell_3();
    Home();

  } else if (estadoInterruptor1 == HIGH && estadoInterruptor2 == HIGH) {
    encenderNaranja();
    Nivell_2(); 
    Home();

  } else if (estadoInterruptor1 == HIGH && estadoInterruptor2 == LOW) {
    encenderVerde();
    Nivell_1(); 
    Home();
  }
}