#define X_STEP_PIN 9
#define X_DIR_PIN 3
#define X_ENABLE_PIN 4 // Define el pin de habilitación para el motor en el eje X
#define Y_STEP_PIN 11
#define Y_DIR_PIN 6
#define Y_ENABLE_PIN 7 // Define el pin de habilitación para el motor en el eje Y
#define pot1 A4
#define pot2 A5

volatile long totalStepsX = 0; 
volatile long totalStepsY = 0; 

// Inicialización de los motores
void setup() {
  Serial.begin(9600);
  Serial.print(totalStepsX);
  Serial.println( "1! " );

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

  // Inicializamos las direcciones de los motores (por ejemplo, hacia adelante)
  digitalWrite(X_DIR_PIN, LOW);
  digitalWrite(Y_DIR_PIN, HIGH);

  configurarTimer1(10000.0); 
  configurarTimer2(10000.0);
  totalStepsX = 0;
  totalStepsY = 0;
}

unsigned int Preescaler(unsigned int frecuenciaDeseada){
  unsigned long frecuenciaReloj = 16000000UL; // Frecuencia del reloj del ATmega328P en Hz
  unsigned long contadorMaximo =  65535UL; // Contador máximo para los timers de 16 bits
  
  unsigned int preescaler = frecuenciaReloj / ((frecuenciaDeseada) * contadorMaximo * 2) - 1;
  return preescaler;
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
// Función para configurar el Timer 2 con el preescaler proporcionado
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


// Función para manejar la interrupción del Timer 1
ISR(TIMER1_COMPA_vect) {
// 1 pas son 1.8º per tant 1 volta son 200 pasos.
// l'eix del motor es de 5mm 
// la distancia que recorre en una volta son 5*pi o 15.7079...
// per tant si la distancia que ha de recorre son 280 x 280mm 
// per recorre aquesta distancia el motor ha de fer 17,8253... voltes que son 3565.070725 pasos.
  if (digitalRead(X_ENABLE_PIN) == LOW){totalStepsX++;}
  if (Y_ENABLE_PIN){totalStepsY++;}

  Serial.println(totalStepsX);

  // if (totalStepsX == 3565){
  //   disableMotorX();
  //   Serial.print(totalStepsX);
  //   Serial.println();
  // }
  // Serial.print(lect1);
  // Serial.println();
  // Serial.print(lect2);
  // Serial.println();
}

// Función para manejar la interrupción del Timer 2
ISR(TIMER2_COMPA_vect) {
}

void loop() {
  int lect1 = map(analogRead(pot1),1,1024, 16, 256);
  int lect2 = map(analogRead(pot2),1,1024, 16, 256);
  
  // Serial.print(lect1);
  // Serial.println();
  // Serial.print(lect2);
  // Serial.println();

  setSpeedMotorX(lect1);
  setSpeedMotorY(lect2);
}