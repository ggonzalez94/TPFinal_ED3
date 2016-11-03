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
           
//           if(dato == 255){
//                recibiendo = true;
//           }
//
//           if(dato == 0){
//            recibiendo = false;
//           }
             
  }
}
