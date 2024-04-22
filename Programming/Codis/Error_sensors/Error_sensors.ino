const int hall1Pin = A0; // Sensor Hall 1 (arriba izquierda)
const int hall2Pin = A1; // Sensor Hall 2 (arriba derecha)
const int hall3Pin = A2; // Sensor Hall 3 (abajo izquierda)
const int hall4Pin = A3; // Sensor Hall 4 (abajo derecha)

void setup() {
  Serial.begin(9600);

  pinMode(hall1Pin, INPUT);
  pinMode(hall2Pin, INPUT);
  pinMode(hall3Pin, INPUT);
  pinMode(hall4Pin, INPUT);
}

void loop() {
  int hall1 = analogRead(hall1Pin);
  int hall2 = analogRead(hall2Pin);
  int hall3 = analogRead(hall3Pin);
  int hall4 = analogRead(hall4Pin);

  Serial.print(hall1);
  Serial.print(",");
  Serial.print(hall2);
  Serial.print(",");
  Serial.print(hall3);
  Serial.print(",");
  Serial.println(hall4);

  int errX = (hall1 + hall2) - (hall3 + hall4); // error del eje x
  Serial.print(errX);
  Serial.print(",");
  int errY = (hall1 + hall4) - (hall2 + hall3); // error del eje x
  Serial.println(errY);

  delay(500);
}