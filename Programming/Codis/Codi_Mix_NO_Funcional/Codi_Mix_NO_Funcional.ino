#define X_STEP_PIN 2
#define X_DIR_PIN 3
#define X_ENABLE_PIN 4 // Define el pin de habilitación para el motor en el eje X
#define Y_STEP_PIN 5
#define Y_DIR_PIN 6
#define Y_ENABLE_PIN 7 // Define el pin de habilitación para el motor en el eje Y

const float PULSES_PER_SECOND = 1000.0;
const float PULSE_PERIOD_MICROSECONDS = 1000000.0 / PULSES_PER_SECOND;

volatile long totalStepsX = 0; // Variable para llevar un seguimiento del número total de pasos en el eje X
volatile long totalStepsY = 0; // Variable para llevar un seguimiento del número total de pasos en el eje Y

//int bot = 8; // Pin para el botón
//int botestatAnterior = LOW;

int A325_1 = A0; 
int A325_2 = A1;
int A325_3 = A2;
int A325_4 = A3;

void setup() {
  pinMode(X_STEP_PIN, OUTPUT);
  pinMode(X_DIR_PIN, OUTPUT);
  pinMode(X_ENABLE_PIN, OUTPUT); // Configura el pin de habilitación como salida
  pinMode(Y_STEP_PIN, OUTPUT);
  pinMode(Y_DIR_PIN, OUTPUT);
  pinMode(Y_ENABLE_PIN, OUTPUT); // Configura el pin de habilitación como salida
  
  //pinMode(bot, INPUT);
  
  // Inicialmente, deshabilita los motores
  digitalWrite(X_ENABLE_PIN, HIGH); // Deshabilita el motor en el eje X
  digitalWrite(Y_ENABLE_PIN, HIGH); // Deshabilita el motor en el eje Y
  
  // Inicializa el puerto serie para la comunicación
  Serial.begin(9600);
}

void moveMotorX() {
  digitalWrite(X_STEP_PIN, HIGH);
  delayMicroseconds(50); // Ajusta según la velocidad del motor
  digitalWrite(X_STEP_PIN, LOW);
  totalStepsX++; // Incrementa el contador de pasos en el eje X
}

void moveMotorY() {
  digitalWrite(Y_STEP_PIN, HIGH);
  delayMicroseconds(50); // Ajusta según la velocidad del motor
  digitalWrite(Y_STEP_PIN, LOW);
  totalStepsY++; // Incrementa el contador de pasos en el eje Y
}

void enableMotorX() {
  digitalWrite(X_ENABLE_PIN, LOW); // Habilita el motor en el eje X (activo en bajo)
}

void disableMotorX() {
  digitalWrite(X_ENABLE_PIN, HIGH); // Deshabilita el motor en el eje X (activo en alto)
}

void enableMotorY() {
  digitalWrite(Y_ENABLE_PIN, LOW); // Habilita el motor en el eje Y (activo en bajo)
}

void disableMotorY() {
  digitalWrite(Y_ENABLE_PIN, HIGH); // Deshabilita el motor en el eje Y (activo en alto)
}

ISR(TIMER1_COMPA_vect) {
  //moveMotorX();
}

//ISR(TIMER2_COMPA_vect) {moveMotorY();}

void loop() {
  // Lectura de los valores de los sensores
  float val1 = analogRead(A325_1); 
  float val2 = analogRead(A325_2);
  float val3 = analogRead(A325_3);
  float val4 = analogRead(A325_4);

  // Movimiento de los motores en función de los valores de los sensores
  if (val1 > 600) {
    enableMotorX();
    moveMotorX();
    disableMotorX();
  }
  
  if (val2 > 800) {
    enableMotorY();
    moveMotorY();
    disableMotorY();
  }

  // Lectura del botón y movimiento continuo de los motores
  //int botestat = digitalRead(bot);
  //if (botestat == HIGH && botestatAnterior == LOW) {
    enableMotorX();
    for (int i = 0; i < 200; i++) { // Movemos el motor 200 pasos (ajustar según tus necesidades)
      moveMotorX();
    }
    disableMotorX();
  
    enableMotorY();
    for (int i = 0; i < 200; i++) { // Movemos el motor 200 pasos (ajustar según tus necesidades)
      moveMotorY();
    }
    disableMotorY();
  //}
  
  //botestatAnterior = botestat;
}
