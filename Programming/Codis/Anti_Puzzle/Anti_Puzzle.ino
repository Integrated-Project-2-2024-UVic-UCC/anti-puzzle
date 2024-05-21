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
#define DX_dpin 5 // Nuevo detector de final de carrera en el eje X
#define DY_ipin 2 // Nuevo detector de final de carrera en el eje Y

int A325_1 = A0; 
int A325_2 = A2;
int A325_3 = A3;
int A325_4 = A4;

volatile long totalStepsX; 
volatile long totalStepsY; 

float offset1, offset2, offset3, offset4;

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
  while (getDX_e()) {} 
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

bool Print(float V1, float V2, float V3, float V4){
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

void calibrarSensores() {
  float val1 = analogRead(A325_1);
  float val2 = analogRead(A325_2);
  float val3 = analogRead(A325_3);
  float val4 = analogRead(A325_4);
  
  // Calcula los offsets para cada sensor
  offset1 = 500 - val1;
  offset2 = 500 - val2;
  offset3 = 500 - val3;
  offset4 = 500 - val4;
}

float leerSensorCalibrado(int pin, float offset) {
  return analogRead(pin) + offset;
}

int PosicioIman(){
  // ------------------------
  int mitadX = 15300 / 2; // Mitad de la coordenada Y que son 15300 micropasos
  int mitadY = 8200 / 2;  // Mitad de la coordenada X que son 8200 micropasos --> 3200
  long errX;
  int moveY;
  int sum = 0;

  while (sum < 2300) {
    float val1 = leerSensorCalibrado(A325_1, offset1);
    float val2 = leerSensorCalibrado(A325_2, offset2);
    float val3 = leerSensorCalibrado(A325_3, offset3);
    float val4 = leerSensorCalibrado(A325_4, offset4);

    Print(val1, val2, val3, val4);

  //   int EX = CalculerrorX(val1, val2, val3, val4);
  //   int EY = CalculerrorY(val1, val2, val3, val4);
  //   Serial.print(EX);
  //   Serial.println();

  //   sum = val1 + val2 + val3 + val4; 

  //   if (EX>0){
  //     Serial.print("Con if");
  //     Serial.println();
  //     errX = EX;
  //   }else if(EX == 0){
  //     Serial.print("else if");
  //     Serial.println();
  //     errX = 0;
  //   }else{
  //     Serial.print("else");
  //     Serial.println();
  //     errX = map(abs(EX), -520, 1024, 0, mitadX); 
  //   }

  //   // Calcular el error en pasos
  //   // long errX = map(abs(EX), -520, 1024, 0, mitadX); 
  //   long errY = map(abs(EY), -520, 1024, 0, mitadY); 
  
  //   MoveToPosition(errX, errY);
  }
}

  // ------------------------


//   int mitadX = (15300)/2; // Mitad de la coordenada Y que son 15300 micropasos
//   int mitadY = (8200)/2; // Mitad de la coordenada X que son 8200 micropasos --> 3200
//   int sum = 0;

//   while (sum<2300){
//     // 200 pasos per 16micropasoso = 3200micropasos 1 volta
//     float val1 = analogRead(A325_1); 
//     float val2 = analogRead(A325_2);
//     float val3 = analogRead(A325_3);
//     float val4 = analogRead(A325_4);

//     Print(val1,val2,val3,val4);

//     int EX = CalculerrorX(val1,val2,val3,val4);
//     int EY = CalculerrorY(val1,val2,val3,val4);

//     float sum = val1 + val2 + val3 + val4;
//     // Serial.print(-500);
//     // Serial.print(" ");
//     // Serial.print(EX);
//     // Serial.print(" ");
//     // Serial.print(EY+500);
//     // Serial.print(" ");
//     // Serial.print(sum);
//     // Serial.println(" ");
//     // if (sum<2600){}
//     int moveX = map(abs(EX), 0, 516, 0, 15300/2);
//     // Serial.print("moveX: ");
//     // Serial.println(moveX);
    
//     int moveY = map(EY, 0, 516, 0, 8200/2);
//     MoveToPosition(moveX,moveY);

//     // Serial.print("moveY: ");
//     // Serial.println(moveY);

//     // bool dirX = EX < 0; // Si errX es positivo, el imán se mueve en la dirección positiva de X
//     // bool dirY = EY < 0; // Si errY es positivo, el imán se mueve en la dirección positiva de Y
//     // Serial.println(dirX);
//     // Serial.println(dirY);

//     // Es posible que els valors dels pasos s'haigui de cambiar perque no faigui un salt tant gran 
//     // float moveY = map(EY, 532, 998, 0, 8200); // Es posible que els valors dels pasos s'haigui de cambiar perque no faigui un salt tant gran 

//     // Serial.println("Moure motors");

//     // Serial.print("moveX: ");
//     // Serial.println(moveX);
//     // Serial.print("moveY: ");
//     // Serial.println(moveY);
//     }
// }

int CalculerrorX(int hall1,int hall2,int hall3,int hall4){
  int errX = (hall1 + hall2) - (hall3 + hall4);
  return errX;
}

int CalculerrorY(int hall1,int hall2,int hall3,int hall4){
  int errY = (hall1 + hall4) - (hall2 + hall3);
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

  pinMode(DX_epin,INPUT_PULLUP);
  pinMode(DY_spin,INPUT_PULLUP);
  pinMode(DX_dpin,INPUT_PULLUP);
  pinMode(DY_ipin,INPUT_PULLUP);

  pinMode(A325_1, INPUT);
  pinMode(A325_2, INPUT);
  pinMode(A325_3, INPUT);
  pinMode(A325_4, INPUT);
  calibrarSensores();
  
  configurarTimer1(255); 
  configurarTimer2(1);
  disableMotorX();
  disableMotorY();

  totalStepsX = 0;
  totalStepsY = 0;
  // CI();
  // Home();
}

void loop() {
  PosicioIman();
}