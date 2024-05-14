// Per connectar a l'aruidno COM al pin digital, NO a terra i NC a 5V

#define PIN_FIN_CARRERA 10 // Pin digital al que se conecta el final de carrera


void setup() {
  Serial.begin(9600); // Iniciar comunicación serial
  pinMode(PIN_FIN_CARRERA, INPUT_PULLUP); // Configurar el pin como entrada
}

void loop() {
  int estado = digitalRead(PIN_FIN_CARRERA); // Leer el estado del final de carrera
  
  Serial.print("Estado del final de carrera: ");
  if (estado == HIGH) {
    Serial.println("Abierto");
  } else {
    Serial.println("Cerrado");
  }
  
  delay(10); // Esperar 500 milisegundos antes de la siguiente lectura
}


