#include <SoftwareSerial.h>

SoftwareSerial mySerial(10,11);

void setup() {
        Serial.begin(9600);    //9600 baudios
        while(!Serial)
        {;}
        
        mySerial.begin(9600);
}

void loop() {
         if(mySerial.available()) {
           int dato = mySerial.read();
           dato = dato;
           Serial.println(dato);
        }     
}
