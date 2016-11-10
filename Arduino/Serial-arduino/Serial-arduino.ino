#include <HashMap.h>

//Tamaño maximo del hashmap
const byte HASH_SIZE = 9;
//Tipo de hashmap: clave: char, elemento: cadena de chars
HashType<char,char*> hashRawArray[HASH_SIZE];
HashMap<char,char*> hashMap = HashMap<char,char*>( hashRawArray , HASH_SIZE );
//Libreria para utilizar dos puertos seria simultaneamente
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
        
        //Inicializacion del HashMap
        hashMap[0]('r',"Rojo");
        hashMap[1]('g',"Verde");
        hashMap[2]('b',"Azul");
        hashMap[3]('y',"Amarillo");
        hashMap[4]('p',"Rosado");
        hashMap[5]('c',"Celeste");
        hashMap[6]('w',"Blanco");
        hashMap[7]('k',"Negro");
        hashMap[8]('o',"Naranja");
        i=0;

        //Impresion del mensaje de bienvenida
        Serial.println("Bienvenido al identificador de color!");
        Serial.println("Para iniciar la lectura, ingrese 'i'");
        Serial.println("Para finalizar la lectura, ingrese 'f'");
        Serial.println("Se mostraran las componentes R, G y B, y el nombre del color medido");
}

void loop() {
  //Si recibi algo por el puerto serie de la PC
  if(Serial.available()){
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

  //Si recibi algo por el puerto serie de la placa LPC
  if(mySerial.available()){
           int dato = mySerial.read();
           dato = dato;

           //Imprimo los datos en el orden que los recibo
           //Valor Rojo, Verde, Azul, Caracter de color detectado
           //Y mapeo el caracter al nombre del color
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
                    Serial.print(" ");
                    break;
                case 3:
                    Serial.print("Color: ");
                    char *color = hashMap.getValueOf((char) dato);
                    Serial.print(color); 
                    Serial.println(" ");
           }
           i = (i+1)%4;
    }
}

