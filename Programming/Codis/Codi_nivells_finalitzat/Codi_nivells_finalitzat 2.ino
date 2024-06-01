// Definición de pines
const int pinLED_Rojo = 8;    // Pin para el LED rojo
const int pinLED_Verde = 9;    // Pin para el LED verde
const int pinInterruptor1 = 2; // Pin para el primer contacto del interruptor (posición 1)
const int pinInterruptor2 = 3; // Pin para el segundo contacto del interruptor (posición 3)

void setup() {
  // Configurar pines como salida
  pinMode(pinLED_Rojo, OUTPUT);
  pinMode(pinLED_Verde, OUTPUT);

  // Configurar pines de interruptor como entrada con resistencia pull-up
  pinMode(pinInterruptor1, INPUT_PULLUP);
  pinMode(pinInterruptor2, INPUT_PULLUP);

  // Inicializar comunicación serie para depuración
  Serial.begin(9600);
}

void loop() {
  // Leer el estado de los interruptores
  int estadoInterruptor1 = digitalRead(pinInterruptor1);
  int estadoInterruptor2 = digitalRead(pinInterruptor2);

  // Determinar qué color encender basado en la posición del interruptor
  if (estadoInterruptor1 == LOW && estadoInterruptor2 == HIGH) {
    Serial.println("C1");
    encenderRojo();
  } else if (estadoInterruptor1 == HIGH && estadoInterruptor2 == HIGH) {
    Serial.println("C2");
    encenderNaranja();
  } else if (estadoInterruptor1 == HIGH && estadoInterruptor2 == LOW) {
    Serial.println("C3");
    encenderVerde();
    }

  // Espera un momento antes de la siguiente lectura
  delay(500);
}

// Funciones para encender cada color
void encenderVerde() {
  digitalWrite(pinLED_Rojo, HIGH);   // Apagar LED rojo
  digitalWrite(pinLED_Verde, LOW); // Encender LED verde
}

void encenderNaranja() {
  digitalWrite(pinLED_Rojo, LOW);  // Encender LED rojo
  digitalWrite(pinLED_Verde, LOW); // Encender LED verde (combinación puede dar naranja si es un LED RGB o combinación)
}

void encenderRojo() {
  digitalWrite(pinLED_Rojo, LOW);  // Encender LED rojo
  digitalWrite(pinLED_Verde, HIGH);  // Apagar LED verde
}


