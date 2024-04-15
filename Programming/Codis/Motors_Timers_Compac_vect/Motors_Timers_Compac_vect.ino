// Definimos los pines a utilizar para cada motor
#define X_STEP_PIN 2
#define X_DIR_PIN 3
#define X_ENABLE_PIN 4 // Define el pin de habilitación para el motor en el eje X
#define Y_STEP_PIN 5
#define Y_DIR_PIN 6
#define Y_ENABLE_PIN 7 // Define el pin de habilitación para el motor en el eje Y

volatile long totalStepsX = 0; // Variable para llevar un seguimiento del número total de pasos en el eje X
volatile long totalStepsY = 0; // Variable para llevar un seguimiento del número total de pasos en el eje Y

// Definimos las variables para la frecuencia y el periodo del pulso
const float PULSES_PER_SECOND = 10000.0; // Frecuencia en pulsos por segundo
const float PULSE_PERIOD_MICROSECONDS = 2000000.0 / PULSES_PER_SECOND; // Periodo del pulso en microsegundos

// Inicialización de los motores
void setup() {
  pinMode(X_STEP_PIN, OUTPUT);
  pinMode(X_DIR_PIN, OUTPUT);
  pinMode(X_ENABLE_PIN, OUTPUT); // Configura el pin de habilitación como salida
  pinMode(Y_STEP_PIN, OUTPUT);
  pinMode(Y_DIR_PIN, OUTPUT);
  pinMode(Y_ENABLE_PIN, OUTPUT); // Configura el pin de habilitación como salida
  
  // Inicialmente, deshabilita los motores
  digitalWrite(X_ENABLE_PIN, LOW); // Deshabilita el motor en el eje X
  digitalWrite(Y_ENABLE_PIN, LOW); // Deshabilita el motor en el eje Y

  // Inicializamos las direcciones de los motores (por ejemplo, hacia adelante)
  digitalWrite(X_DIR_PIN, LOW);
  digitalWrite(Y_DIR_PIN, HIGH);
  
  // Configuración de los timers para generar la señal de paso para cada motor
  // Timer 1 (para el motor en el eje X)
  TCCR1A = 0; // Configuración inicial de los registros
  TCCR1B = 0;
  TCNT1  = 0; // Inicializamos el contador del timer 1
  OCR1A = round(PULSE_PERIOD_MICROSECONDS); // Configuramos el valor de comparación para la frecuencia deseada
  TCCR1A |= (1 << COM1A0); // Configuramos el Timer 1 para el modo CTC (Clear Timer on Compare Match) y activamos la salida OC1A
  TCCR1B |= (1 << WGM12); // Configuramos el Timer 1 para el modo CTC
  TCCR1B |= (1 << CS10); // Configuramos el prescaler para que el timer cuente a la frecuencia del reloj
  TIMSK1 |= (1 << OCIE1A); // Habilitamos la interrupción de comparación del Timer 1
  
  // Timer 2 (para el motor en el eje Y)
  TCCR2A = 0; // Configuración inicial de los registros
  TCCR2B = 0;
  TCNT2  = 0; // Inicializamos el contador del timer 2
  OCR2A = round(PULSE_PERIOD_MICROSECONDS); // Configuramos el valor de comparación para la frecuencia deseada
  TCCR2A |= (1 << COM2A0); // Configuramos el Timer 2 para el modo CTC (Clear Timer on Compare Match) y activamos la salida OC2A
  TCCR2A |= (1 << WGM21); // Configuramos el Timer 2 para el modo CTC
  TCCR2B |= (1 << CS20); // Configuramos el prescaler para que el timer cuente a la frecuencia del reloj
  TIMSK2 |= (1 << OCIE2A); // Habilitamos la interrupción de comparación del Timer 2
}

// Función para mover el motor en el eje X
void moveMotorX() {
  digitalWrite(X_STEP_PIN, HIGH); // Generamos un pulso
  delayMicroseconds(100); // Esperamos un corto tiempo (ajustar según la velocidad del motor)
  digitalWrite(X_STEP_PIN, LOW); // Apagamos el pulso
  totalStepsX++; // Incrementa el contador de pasos en el eje X
}

// Función para mover el motor en el eje Y
void moveMotorY() {
  digitalWrite(Y_STEP_PIN, HIGH); // Generamos un pulso
  delayMicroseconds(100); // Esperamos un corto tiempo (ajustar según la velocidad del motor)
  digitalWrite(Y_STEP_PIN, LOW); // Apagamos el pulso
  totalStepsY++; // Incrementa el contador de pasos en el eje Y
}

void enableMotorX() {digitalWrite(X_ENABLE_PIN, LOW);} // Habilita el motor en el eje X (activo en bajo)

void disableMotorX() {digitalWrite(X_ENABLE_PIN, HIGH);} // Deshabilita el motor en el eje X (activo en alto)

void enableMotorY() {digitalWrite(Y_ENABLE_PIN, LOW);} // Habilita el motor en el eje Y (activo en bajo)

void disableMotorY() {digitalWrite(Y_ENABLE_PIN, HIGH);} // Deshabilita el motor en el eje Y (activo en alto)

// Función para manejar la interrupción del Timer 1
ISR(TIMER1_COMPA_vect) {
  moveMotorX(); // Llamamos a la función para mover el motor en el eje X
  if ((totalStepsX > 25000) || (totalStepsX > 50000)){
    disableMotorX();
    enableMotorY();
  }
  if (totalStepsY > 50000){
    enableMotorX();
    disableMotorY();
    }
  if (totalStepsX >= 80000){
    enableMotorY();
  }
  moveMotorY();
}

// Función para manejar la interrupción del Timer 2
ISR(TIMER2_COMPA_vect) {

}

// Función principal
void loop() {
}
