#include <SoftwareSerial.h>

SoftwareSerial mySerial(10,11);
char letra[3];
int i;
bool recibiendo = false;
char incomingChar;

void setup() {
        Serial.begin(9600);    //9600 baudios
        while(!Serial)
        {;}
        
        mySerial.begin(9600);
        letra[0] = 'R';
        letra[1] = 'G';
        letra[2] = 'B';
        i=0;
}

void loop() {
  if(Serial.available()){   //si mande algo por teclado
      incomingChar = Serial.read();
      if(incomingChar == 'i'){
        Serial.println("Iniciando la lectura");\
        mySerial.write('i');
      }
      else if(incomingChar == 'f'){
        Serial.println("Finalizando la lectura");
        mySerial.write('f');
      }
  }
  if(mySerial.available()){ //si la lpc mando algo
           int dato = mySerial.read();
           dato = dato;

           if(true){
                switch(i){
                      case 0:
                          Serial.print("R=");
                          Serial.print(dato);
                          Serial.print(" ");
                          break;
                      case 1:
                          Serial.print("G=");
                          Serial.print(dato);
                          Serial.print(" ");
                          break;
                      case 2:
                          Serial.print("B=");
                          Serial.print(dato);
                          Serial.println(" ");
                          break;

                }
                i = (i+1)%3;
                
           }
  }
}
