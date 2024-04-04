// Definición de los pines del driver TCM2209
#define DIR_PIN 3 // Pin para controlar la dirección del motor
#define STEP_PIN 2 // Pin para enviar pulsos al motor
#define ENABLE_PIN 4 // Pin para habilitar o deshabilitar el driver (puede ser útil para el ahorro de energía)

void setup() {
  // Configuración de los pines como salida
  pinMode(DIR_PIN, OUTPUT);
  pinMode(STEP_PIN, OUTPUT);
  pinMode(ENABLE_PIN, OUTPUT);

  // Inicialización del driver
  digitalWrite(ENABLE_PIN, LOW); // Habilitar el driver (LOW para habilitar, HIGH para deshabilitar)

  // Dirección del motor (cambia a HIGH o LOW según la dirección deseada)
  digitalWrite(DIR_PIN, LOW); // Dirección en sentido horario (CW)
}

void loop() {
  // Realizar un paso del motor
  digitalWrite(STEP_PIN, HIGH);
  delayMicroseconds(100); // Ajusta este valor según la velocidad deseada del motor
  digitalWrite(STEP_PIN, LOW);
  delayMicroseconds(100); // Tiempo entre pulsos (ajusta según la velocidad deseada y el motor)

  // Agregar cualquier otra lógica o control aquí según sea necesario
}
