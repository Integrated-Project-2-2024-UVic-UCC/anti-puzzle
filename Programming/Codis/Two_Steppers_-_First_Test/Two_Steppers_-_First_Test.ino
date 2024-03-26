#define PIN_GRN_LED 10
#define PIN_RED_LED 11
#define PIN_BTN_1   14 // A0
#define PIN_BTN_2   15 // A1
#define PIN_BTN_3   16 // A2
#define PIN_POT     17 // A3

int ledStatus = 0;
const char* btnStatus[] = {"pressed ", "released"};

// Define the stepper patterns.
const int stepPatternB[8] = {B00000000, B00000000, B00000000, B00000001, B00000001, B00000011, B00000010, B00000010};
const int stepPatternD[8] = {B01000100, B11001100, B10001000, B10011000, B00010000, B00110000, B00100000, B01100100};

void setup() {
  for(int i = 2; i <= 9; i++){
    pinMode(i, OUTPUT);
  }
  pinMode(PIN_GRN_LED, OUTPUT);
  pinMode(PIN_RED_LED, OUTPUT);
  pinMode(PIN_BTN_1, INPUT_PULLUP);
  pinMode(PIN_BTN_2, INPUT_PULLUP);
  pinMode(PIN_BTN_3, INPUT_PULLUP);
  Serial.begin(9600);
}

void loop(){
  int k;
  char uiStatus[96];
  double potPercent; 

  // Read the status of the input elements.
  potPercent = (double)analogRead(PIN_POT) * 100 / 1023;
  sprintf(uiStatus, "Pot: % 3d%% --- Btn 1: %s --- Btn 2: %s --- Btn 3: %s", (int)potPercent, btnStatus[digitalRead(PIN_BTN_1)], btnStatus[digitalRead(PIN_BTN_2)], btnStatus[digitalRead(PIN_BTN_3)]);
  Serial.println(uiStatus);

  // Switch the LEDs.
  ledStatus = 1-ledStatus;
  digitalWrite(PIN_GRN_LED, ledStatus);
  digitalWrite(PIN_RED_LED, 1-ledStatus);

  // Now, move the motors.
  for(int j = 0; j < 128; j++){
    for(int i = 0; i < 8; i++){
      if(!ledStatus){
        k = i;    // Go half a step forward.
      }
      else{
        k = 7-i;  // Go half a step backward.
      }
      PORTB = (PORTB & B11111100) | stepPatternB[k];
      PORTD = (PORTD & B00000011) | stepPatternD[k];
      delay(1);
    }
  }
  // Add a small delay to make the motors settle.
  delay(100);
}
