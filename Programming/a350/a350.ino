int Led = 13 ;// Declaration of the LED output pin
int A325 = A0; // Declaration of the sensor input pin int val; // Temporary Variable
void setup () {
  Serial.begin(9600);
  pinMode (A325, INPUT) ; // Sensor pin initialization digitalWrite(Sensor, HIGH); // Activation of internal pull-up resistor
}
void loop () {
  delay(100);
  float val = analogRead(A325) ; // The current signal at the sensor is read out
  Serial.print("valor del sensor: ");
  Serial.println(val);}