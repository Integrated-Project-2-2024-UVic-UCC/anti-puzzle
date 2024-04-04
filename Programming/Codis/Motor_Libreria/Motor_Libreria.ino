#include <Stepper.h>

// Define el número de pasos por revolución del motor
const int stepsPerRevolution = 5000;

// Define los pines de control del motor
#define DIR_PIN 3 // Pin para controlar la dirección del motor
#define STEP_PIN 2 // Pin para enviar pulsos al motor

// Crea un objeto Stepper
Stepper myStepper(stepsPerRevolution, DIR_PIN, STEP_PIN);

void setup() {
  // Establece la velocidad del motor en pasos por minuto
  myStepper.setSpeed(500); // 500 pasos por minuto
}

void loop() {
  // Gira el motor en un sentido
  myStepper.step(stepsPerRevolution);
  // Espera un momento antes de invertir la dirección
  delay(1000);
  // Invierte la dirección y gira el motor en el sentido contrario
  myStepper.step(-stepsPerRevolution);



}
