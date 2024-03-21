// Defineix el pin del sensor
const int sensorPin = 2;

void setup() {
  // Inicialitza el pin del sensor com a entrada
  pinMode(sensorPin, INPUT);

  // Configura la resistència de pull-up (opcional)
  // digitalWrite(sensorPin, HIGH);
}

void loop() {
  // Llegeix l'estat del sensor
  int sensorState = digitalRead(sensorPin);

  // Imprimeix l'estat del sensor
  Serial.println(sensorState);
}
