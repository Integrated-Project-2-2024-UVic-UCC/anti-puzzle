float M1[30];
float M2[30];
float M3[30];
float M4[30];

int bot = 2;
int botestatAnterior = LOW; 

int suma1 = 0;
int suma2 = 0;
int suma3 = 0;
int suma4 = 0;

int A325_1 = A0; 
int A325_2 = A1;
int A325_3 = A2;
int A325_4 = A3;

int val1 = 0;
int val2 = 0;
int val3 = 0;
int val4 = 0;

float val_in1;
float val_in2;
float val_in3;
float val_in4;

float T = 2;

void setup () {
  Serial.begin(9600);
  pinMode (bot, INPUT);
  pinMode (A325_1, INPUT) ;
  pinMode (A325_2, INPUT) ;
  pinMode (A325_3, INPUT) ;
  pinMode (A325_4, INPUT) ;

  Serial.println("Configurant sensors. ");
  for(int i = 0; i<=29; i++){
    M1[i] = analogRead(A325_1);
    suma1 += M1[i];
    M2[i] = analogRead(A325_2);
    suma2 += M2[i];
    M3[i] = analogRead(A325_3);
    suma3 += M3[i];
    M4[i] = analogRead(A325_4);
    suma4 += M4[i];
  }

  val_in1 = suma1/30;
  val_in2 = suma2/30;
  val_in3 = suma3/30;
  val_in4 = suma4/30;

  Serial.println("Configuració dels sensors finalitzada. ");
  delay(2000);
}

void loop () {
  int botestat = digitalRead(bot); 

  if (botestat == HIGH) {
    float val1 = analogRead(A325_1); 
    float val2 = analogRead(A325_2);
    float val3 = analogRead(A325_3);
    float val4 = analogRead(A325_4);

    // Cálculo de las diferencias entre los sensores opuestos
    float diff_vert1 = val1 - val2;
    float diff_vert2 = val3 - val4;

    float diff_vert = diff_vert1 - diff_vert2;

    float diff_horiz1 = val2 - val3;
    float diff_horiz2 = val1 - val4;

    float diff_horiz = diff_horiz1 - diff_horiz2;
    
    if ((val1 > 511 && val2 > 511) || (val3 > 511 && val4> 511)){
      if (diff_vert > diff_vert1){
        Serial.println("abajo");
      } else  if (diff_vert > diff_vert2){
        Serial.println("arriba");
      }
    }
  }
  //   if (diff_horiz < diff_horiz1){
  //     Serial.println("izquierda");
  //   } else if (diff_horiz > diff_horiz2){
  //     Serial.println("derecha");
  //   }
  // }
  



    // // Determinar la dirección de movimiento basándose en las diferencias
    // if (abs(Aprox(diff_vert1,diff_vert2,T)) > abs(diff_vert)) {
    //   // Mover en dirección vertical
    //   if (diff_vert > 0) {
    //     // Mover hacia arriba
    //     Serial.println("arriba");
    //   } else {
    //     // Mover hacia abajo
    //     Serial.println("abajo");
    //   }
    //   if (abs(Aprox(diff_horiz1,diff_horiz2,T)) > abs(diff_horiz)){
    //     // Mover en dirección horizontal
    //     if (diff_horiz > 0) {
    //       // Mover hacia la derecha
    //       Serial.println("derecha");
    //     } else{
    //       // Mover hacia la izquierda
    //       Serial.println("izquierda");
    //     }
    //   }
  botestatAnterior = botestat;
  // delay(1000); // Espera un segundo antes de realizar la próxima lectura
}
bool Print(float V1, float V2, float V3, float V4){
  Serial.print(V1);
  Serial.print(" ");
  Serial.print(V2+1000);
  Serial.print(" ");
  Serial.print(V3+2000);
  Serial.print(" ");
  Serial.print(V4+3000);
  Serial.print(" ");
  Serial.print(4000);
  Serial.println(" ");
}

bool Aprox(float a, float b, float tolerancia) {
  float diferenciaAbsoluta = abs(a - b);
  return diferenciaAbsoluta <= tolerancia;
}