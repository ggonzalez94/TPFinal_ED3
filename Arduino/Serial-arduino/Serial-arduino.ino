#include <SoftwareSerial.h>

SoftwareSerial mySerial(10,11);
char letra[3];
int i;
bool recibiendo = false;

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
  if(mySerial.available()) {
           int dato = mySerial.read();
           dato = dato;
           
           if(dato == 255){
                recibiendo = true;
           }

           if(recibiendo){
                switch(i){
                      case 0:
                          Serial.print("R=");
                          Serial.print(dato);
                          Serial.print(" ");
                          i++;
                          break;
                      case 1:
                          Serial.print("G=");
                          Serial.print(dato);
                          Serial.print(" ");
                          i++;
                          break;
                      case 2:
                          Serial.print("B=");
                          Serial.print(dato);
                          Serial.print(" ");
                          i=0;
                          break;
                }
           }

           if(dato == 0){
            recibiendo = false;
           }
             
//           if(dato<3){
//            Serial.print(letra[dato]);
//            Serial.print("=");
//            i++;
//           }
//
//           else if(dato>=3){
//            Serial.print(dato);
//            Serial.print(" ");
//            i++;
//           }
//
//           if(i == 6){
//            i=0;
//            Serial.println("");
//           }
  }
}
