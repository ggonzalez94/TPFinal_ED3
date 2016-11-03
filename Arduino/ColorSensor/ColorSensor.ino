#include <math.h>
 
#define S0 4
#define S1 5
#define S2 6
#define S3 7
#define sensorOut 8
int freqRojo = 0, freqVerde = 0, freqAzul = 0;
int rojo = 11;
int verde = 10;
int azul = 9;

int j;
float scaledResult; 

void setup() {
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(sensorOut, INPUT);

  pinMode(rojo,OUTPUT);
  pinMode(verde,OUTPUT);
  pinMode(azul,OUTPUT);
  
  // Setting frequency-scaling to 20%
  digitalWrite(S0,HIGH);
  digitalWrite(S1,LOW);
  
  Serial.begin(9600);
}
void loop() {
  // Setting red filtered photodiodes to be read
  digitalWrite(S2,LOW);
  digitalWrite(S3,LOW);
  // Reading the output frequency
  freqRojo = pulseIn(sensorOut, LOW);
//  freqRojo = map(freqRojo, 28,89,255,0);
//  freqRojo = constrain(freqRojo,0,255);

  freqRojo = map(freqRojo, 28,70,255,0);
  freqRojo = constrain(freqRojo,0,255);
  
  // Printing the value on the serial monitor
  Serial.print("R= ");//printing name
  Serial.print(freqRojo);//printing RED color frequency
  Serial.print("  ");
  delay(100);
  // Setting Green filtered photodiodes to be read
  digitalWrite(S2,HIGH);
  digitalWrite(S3,HIGH);
  // Reading the output frequency
  freqVerde = pulseIn(sensorOut, LOW);
//  freqVerde = map(freqVerde, 30,70,75,0);
//  freqVerde = constrain(freqVerde,0,75);

//  freqVerde = map(freqVerde, 28,70,75,0);
  freqVerde = fscale( 32, 89, 75, 0, freqVerde, 10);
//  freqVerde = constrain(freqVerde,0,75);
  
  // Printing the value on the serial monitor
  Serial.print("G= ");//printing name
  Serial.print(freqVerde);//printing RED color frequency
  Serial.print("  ");
  delay(100);
  // Setting Blue filtered photodiodes to be read
  digitalWrite(S2,LOW);
  digitalWrite(S3,HIGH);
  // Reading the output frequency
  freqAzul = pulseIn(sensorOut, LOW);  
//  freqAzul = map(freqAzul, 30,50,60,0);
//  freqAzul = constrain(freqAzul,0,50);

 // freqAzul = map(freqAzul, 30,70,15,0);
  freqAzul = fscale( 24, 68, 50, 0, freqAzul, 10);
  freqAzul = constrain(freqAzul,0,50);
  
  // Printing the value on the serial monitor
  Serial.print("B= ");//printing name
  Serial.print(freqAzul);//printing RED color frequency
  
  decidir(freqRojo, freqVerde, freqAzul);
  Serial.println("  ");

  analogWrite(rojo,freqRojo);
  analogWrite(azul, freqAzul);
  analogWrite(verde, freqVerde);

  delay(100);
}

void decidir(int rojo, int verde, int azul){

    if(rojo>180){
        if(verde>24 && azul > 20)
            Serial.print(" BLANCO");
    
        if(verde>24 && azul < 10)
            Serial.print(" AMARILLO");

        if(verde>4 && verde<24 && azul<10)
            Serial.print(" NARANJA");

        if(verde<3 && azul<2)
            Serial.print(" ROJO");

        if(verde<4 && azul>=2)
            Serial.print(" ROSA");
    }
    
    else if(rojo<=180){
        if(verde<4 && azul<2)
            Serial.print(" NEGRO");

        if(verde<10 && azul>9)
            Serial.print(" AZUL");

        if(verde>=10 && azul>9)
            Serial.print(" CELESTE");

        if(verde>5 && azul<9)
            Serial.print(" VERDE");
    }
}


float fscale( float originalMin, float originalMax, float newBegin, float
newEnd, float inputValue, float curve){

  float OriginalRange = 0;
  float NewRange = 0;
  float zeroRefCurVal = 0;
  float normalizedCurVal = 0;
  float rangedValue = 0;
  boolean invFlag = 0;


  // condition curve parameter
  // limit range

  if (curve > 10) curve = 10;
  if (curve < -10) curve = -10;

  curve = (curve * -.1) ; // - invert and scale - this seems more intuitive - postive numbers give more weight to high end on output 
  curve = pow(10, curve); // convert linear scale into lograthimic exponent for other pow function

  // Check for out of range inputValues
  if (inputValue < originalMin) {
    inputValue = originalMin;
  }
  if (inputValue > originalMax) {
    inputValue = originalMax;
  }

  // Zero Refference the values
  OriginalRange = originalMax - originalMin;

  if (newEnd > newBegin){ 
    NewRange = newEnd - newBegin;
  }
  else
  {
    NewRange = newBegin - newEnd; 
    invFlag = 1;
  }

  zeroRefCurVal = inputValue - originalMin;
  normalizedCurVal  =  zeroRefCurVal / OriginalRange;   // normalize to 0 - 1 float

  // Check for originalMin > originalMax  - the math for all other cases i.e. negative numbers seems to work out fine 
  if (originalMin > originalMax ) {
    return 0;
  }

  if (invFlag == 0){
    rangedValue =  (pow(normalizedCurVal, curve) * NewRange) + newBegin;

  }
  else     // invert the ranges
  {   
    rangedValue =  newBegin - (pow(normalizedCurVal, curve) * NewRange); 
  }

  return rangedValue;
}
