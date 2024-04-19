#include <Stepper.h>

// Define el número de pasos por revolución del motor
const int stepsPerRevolution = 6000; // Puedes ajustar este valor según tu motor

// Define los pines de control del primer motor (eje X)
#define X_DIR_PIN 3   // Pin para controlar la dirección del motor X
#define X_STEP_PIN 2  // Pin para enviar pulsos al motor X

// Define los pines de control del segundo motor (eje Y)
#define Y_DIR_PIN 5   // Pin para controlar la dirección del motor Y
#define Y_STEP_PIN 4  // Pin para enviar pulsos al motor Y

// Crea objetos Stepper para los motores X e Y
Stepper stepperX(stepsPerRevolution, X_DIR_PIN, X_STEP_PIN);
Stepper stepperY(stepsPerRevolution, Y_DIR_PIN, Y_STEP_PIN);

void setup() {
  // Establece la velocidad de ambos motores en pasos por minuto
  stepperX.setSpeed(500); // 500 pasos por minuto
  stepperY.setSpeed(500); // 500 pasos por minuto
}

void loop() {
  // Gira el motor X en un sentido
  stepperX.step(stepsPerRevolution);
  // Espera un momento antes de invertir la dirección

  // Invierte la dirección y gira el motor X en el sentido contrario
  stepperX.step(-stepsPerRevolution);

  // Gira el motor Y en un sentido
  stepperY.step(stepsPerRevolution);
  // Espera un momento antes de invertir la dirección

  // Invierte la dirección y gira el motor Y en el sentido contrario
  stepperY.step(-stepsPerRevolution);
}
