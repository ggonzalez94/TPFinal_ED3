#include <SoftwareSerial.h>

SoftwareSerial mySerial(10,11);
char letra[3];
int i;

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

           if(dato<3){
            Serial.print(letra[dato]);
            Serial.print("=");
            i++;
           }

           else if(dato>=3){
            Serial.print(dato);
            Serial.print(" ");
            i++;
           }

           if(i == 6){
            i=0;
            Serial.println("");
           }
  }
}
