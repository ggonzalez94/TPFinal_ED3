#include <SoftwareSerial.h>

SoftwareSerial mySerial(10,11);
char letra[3];
int i;

void setup() {
        Serial.begin(9600);    //9600 baudios
        while(!Serial)
        {;}
        
        mySerial.begin(9600);
        letra[0] = 'r';
        letra[1] = 'g';
        letra[2] = 'b';
        i=0;
}

void loop() {
  if(mySerial.available()) {
           int dato = mySerial.read();
           dato = dato;
           Serial.print(letra[i]);
           Serial.print("=");
           Serial.print(dato);
           Serial.print(" ");
           if(i==2)
              Serial.println("");
           i = (i+1)%3; 
  }
}
