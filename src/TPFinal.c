/*
===============================================================================
 Name        : TPFinal.c
 Author      : Gustavo Gonzalez, Facundo Maero
 Version     : 1.0
===============================================================================
*/

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#include "math.h"
#endif

#include <cr_section_macros.h>

//Defino constantes a utilizar en el programa
#define S0 4
#define S1 5
#define S2 10
#define S3 11
#define INPUT 0
#define PUL_EXT 1
#define TIME 0.3
#define CANTIDAD_COLORES 9

//Constantes de calibracion
//Valores maximos y minimos de cada sensor usados para calibrar la salida
#define R_MAX 71
#define R_MIN 169
#define G_MAX 60
#define G_MIN 177
#define B_MAX 40
#define B_MIN 137

//Pin Registers
unsigned int volatile *const fio0dir = (unsigned int *) 0x2009C000;
unsigned int volatile *const fio0pin = (unsigned int *) 0x2009C014;
unsigned int volatile *const fio0set = (unsigned int *) 0x2009C018;
unsigned int volatile *const fio0clr = (unsigned int *) 0x2009C01C;

//Timer Registers
unsigned int volatile *const t0ir = (unsigned int *) 0x40004000;
unsigned int volatile *const t0tcr = (unsigned int *) 0x40004004; //Habilitar Timer
unsigned int volatile *const t0pr = (unsigned int *) 0x4000400C;
unsigned int volatile *const t0mcr = (unsigned int *) 0x40004014;
unsigned int volatile *const t0mr0 = (unsigned int *) 0x40004018;
unsigned int volatile *const t0ctcr = (unsigned int *) 0x40004070; //Seleccionar modo(TMR o Counter)
unsigned int volatile *const t0ccr = (unsigned int *) 0x40004028;
unsigned int volatile *const t0cr0 = (unsigned int *) 0x4000402C;
unsigned int volatile *const pinsel3 = (unsigned int *) 0x4002C00C;

//Interrupt Registers
unsigned int volatile *const iser0 = (unsigned int *) 0xE000E100;
unsigned int volatile *const icer0 = (unsigned int *) 0xE000E180;
unsigned int volatile *const iabr0 = (unsigned int *) 0xE000E300;
unsigned int volatile *const io0intenr = (unsigned int *)0x40028090;
unsigned int volatile *const io0intclr = (unsigned int *)0x4002808C;

//Puerto Serie Registers
unsigned int volatile *const pconp = (unsigned int *) 0x400FC0C4;
unsigned int volatile *const u0dll = (unsigned int *) 0x4000C000;
unsigned int volatile *const u0dlm = (unsigned int *) 0x4000C004;
unsigned int volatile *const u0lcr = (unsigned int *) 0x4000C00C;
unsigned int volatile *const u0lsr = (unsigned int *) 0x4000C014;
unsigned int volatile *const u0thr = (unsigned int *) 0x4000C000;
unsigned int volatile *const pinsel0 = (unsigned int *) 0x4002C000;
unsigned int volatile *const u0rbr = (unsigned int *) 0x4000C000;
unsigned int volatile *const u0ier = (unsigned int *) 0x4000C004;

//PWM Registers
unsigned int volatile *const pwm1tcr = (unsigned int *) 0x40018004;
unsigned int volatile *const pwm1mcr = (unsigned int *) 0x40018014;
unsigned int volatile *const pwm1mr0 = (unsigned int *) 0x40018018;
unsigned int volatile *const pwm1mr1 = (unsigned int *) 0x4001801C;
unsigned int volatile *const pwm1mr2 = (unsigned int *) 0x40018020;
unsigned int volatile *const pwm1mr3 = (unsigned int *) 0x40018024;
unsigned int volatile *const pwm1pcr = (unsigned int *) 0x4001804C;
unsigned int volatile *const pwm1ler = (unsigned int *) 0x40018050;

//otros punteros y variables globales

int volatile transmitir = 0;
int volatile color_leyendo = 0; //Para saber que color debo leer
								//0=rojo;1=verde;2=azul

//Arreglo con los valores RGB para cada color(primera fila componente R,segunda G,tercera B)
//Estos valores surgen de la calibracion previa del sensor, con los valores tipicos de cada color medido
int color_values[3][CANTIDAD_COLORES] = {
										 {241,	200,	40,		255,	255,	195,	255,	0,	255},
										 {47,	200,	120,	241,	120,	250,	255,	0,	130},
										 {52,	150,	100,	168,	160,	239,	249,	0,	76}
										};
//Valores a utilizar en los registros MATCH del modulo PWM para lograr la reproduccion de los colores en el LED RGB
int led_values[3][CANTIDAD_COLORES] = {
										{50000,	0,		0,		40000,	50000,	0,		50000,	0,	50000},
										{0,		50000,	0,		5000,	0,		10000,	10000,	0,	1000},
										{0,		0,		50000,	0,		8000,	20000,	10000,	0,	0}
									  };
int rgb_values[3] = {1,1,1}; // Valores en escala RGB

//Los colores a detectar son: rojo, verde, azul, amarillo, rosa, celeste, blanco, negro, naranja
char color_names[CANTIDAD_COLORES] = {'r','g','b','y','p','c','w','k','o'}; //Arreglo con el nombre de cada color

int volatile vuelta_captura = 0; //para saber cuantas veces hice captura
//variables para guardar valores del captura
int volatile tiempo1;
int volatile tiempo2;
int volatile suma_captura = 0;
int primeraVez = 1;
int charRecibido;

//Prototipos de funciones
void config_pines();
void config_timer0();
void config_puerto_serie();
void config_PWM();
void leer_rojo();
void leer_verde();
void leer_azul();
void actualizar_PWM(int colorDetectado);
void enviar(int colorDetectado);
int classify();
int map(int x,int in_min,int in_max, int out_min, int out_max);
void comenzarLectura();
void terminarLectura();

int main(void) {

	config_pines();
	*fio0clr |= (1<<S1);
	*fio0set |= (1<<S0);
	config_timer0();
	config_puerto_serie();
	config_PWM();
//	comenzarLectura();
	terminarLectura();

    while(1) {}
    return 0 ;
}

void config_timer0(){
	*t0ctcr = 0; //Timer 0 como timer y no contador(ya viene asi por defecto)

	//Match 0
	*t0mr0 = 25000000 *TIME; //Interrumpir cada TIME segundo
	*t0pr = 0;  //Preescaler register en 0
	*t0mcr |= (1<<1);  //Configuramos para que resetee el TC

	//Captura
	*t0ccr |= (1<<0); //Captura en rising edge del pin 1.26
	*pinsel3 |= (1<<20) | (1<<21);

	//Interrupciones
	*iser0 |= (1<<1); //Habilito interrupciones por TMR0
	*t0tcr |= (1<<0); //Empiezo a contar

	return;
}

void config_puerto_serie(){
	*pconp |= (1<<3); //Prendo UART
	*u0lcr |= (1<<0) | (1<<1); //8 bits

	//Dejo pclock para el UART en 25MHZ
	*u0lcr |= (1<<7); //Habilito acceso a los bits del divisor
	*u0dll = 163; //Baud_rate = 9585
	*u0dlm = 0;
	*u0lcr &= ~(1<<7); //Deshabilito acceso a bits divisor
	*pinsel0 |= (1<<4); //Confiuro pin 0.2
	*u0lcr &= ~(1<<2 | 1<<3); //1 bit de stop y sin paridad

	*pinsel0 |= (1<<6);	//Configuro recepcion por UART0 Rx
	*u0ier |= (1<<1);	//habilito interrupciones por UART0
	*iser0 |= (1<<5);	//habilito interrupciones en nvic

	return;
}


void config_pines(){
	*fio0dir |= (1<<S0) | (1<<S1) | (1<<S2) | (1<<S3); //Pines de configuracion del sensor como salida
	*fio0dir &= ~(1<<PUL_EXT); //Pin 0.PUL_EXT como entrada para el pulsador
	*io0intenr |= (1<<PUL_EXT); //Habilito interrupciones por flanco de subida
	*iser0 |= (1<<21); //Habilito interrupciones externas

	return;
}

void config_PWM(){
	*pconp |= (1<<6); //Habilito el modulo PWM

	//Pines 1.18,20,21
	*pinsel3 |= (1<<5);
	*pinsel3 |= (1<<9);
	*pinsel3 |= (1<<11);
	*pwm1mr0 |= 25000000/500; //Periodo del pulso (igual al de Arduino)
	*pwm1mcr |= (1<<1); //Reseteo en Match0
	*pwm1ler |= (1<<0); //Hago efectivo el valor del Match
	*pwm1tcr |= (1<<0) | (1<<3); //Configuro el modulo como PWM

	//Habilito las salidas 1,2 y 3
	*pwm1pcr |= (1<<9) | (1<<10) | (1<<11);

	return;
}

//Setea los pines de configuracion del sensor para leer la componente Rojo
void leer_rojo(){
	*fio0clr |= (1<<S2) | (1<<S3);
	return;
}

//Setea los pines de configuracion del sensor para leer la componente Verde
void leer_verde(){
	*fio0set |= (1<<S2) | (1<<S3);
	return;
}

//Setea los pines de configuracion del sensor para leer la componente Azul
void leer_azul(){
	*fio0clr |= (1<<S2);
	*fio0set |= (1<<S3);
	return;
}

//Actualiza el valor de los registros Match del modulo PWM en funcion del color detectado
//Enciende en el LED el color detectado
void actualizar_PWM(int colorDetectado){

	//Cambio el duty cicle actualizando los match
	*pwm1mr1 = led_values[0][colorDetectado];
	*pwm1mr2 = led_values[1][colorDetectado];
	*pwm1mr3 = led_values[2][colorDetectado];

	//Hago que el cambio sea efectivo
	*pwm1ler |= (1<<1) | (1<<2) | (1<<3);
	return;
}

//Envia al Arduino la informacion recogida por el sensor
void enviar(int colorDetectado){
		int rojo = rgb_values[0];
		int verde = rgb_values[1];
		int azul = rgb_values[2];

		while((*u0lsr & (1<<5))==0){} //Espero a que el buffer este vacio
		*u0thr = (rojo & 0xFF);
		while((*u0lsr & (1<<5))==0){} //Espero a que el buffer este vacio
		*u0thr = (verde & 0xFF);
		while((*u0lsr & (1<<5))==0){} //Espero a que el buffer este vacio
		*u0thr = (azul & 0xFF);

//		envio la letra correspondiente al color identificado
		while((*u0lsr & (1<<5))==0){} //Espero a que el buffer este vacio
		*u0thr = ((color_names[colorDetectado]) & 0xFF);
}

//Algoritmo de distancia minima
//Compara los valores medidos de las 3 componentes con la matriz de calibracion
//Decide en funcion de la menor distancia de los 3 valores con algun color de la matriz
int classify(){
	int i_color;
	int ClosestColor = 0;
	float MaxDiff;
	float MinDiff = 1000.0;
	for (i_color = 0; i_color < CANTIDAD_COLORES; i_color ++) {
	  // Calcula la distancia
	  float ED = sqrt(pow((color_values[0][i_color] - rgb_values[0]),2.0) +
	  pow((color_values[1][i_color] - rgb_values[1]),2.0) + pow((color_values[2][i_color] - rgb_values[2]),2.0));
	  MaxDiff = ED;
	  // Encuentra la distancia menor
	  if (MaxDiff < MinDiff) {
		MinDiff = MaxDiff;
		ClosestColor = i_color;
	  }
	}
	//Devuelvo el indice (0 - 8) del color detectado
	return ClosestColor;
}

//Convierte el valor en Periodo, medido en el sensor, por los valores RGB 0 - 255
int map(int x,int in_min,int in_max, int out_min, int out_max){
	if (x<=in_max)
		return out_max;

	else if(x>=in_min)
		return out_min;

	else
		return (x-in_min)*(out_max-out_min)/(in_max - in_min) + out_min;
}

//Configura las interrupciones para comenzar a leer valores del sensor
void comenzarLectura(){
	*t0tcr = (1<<1); //Reseteo y deshabilito el timer
	*t0mcr |= (1<<0); //Enciendo interrupciones por match
	*t0ccr |= (1<<2); //Enciendo interrupciones por captura
	*t0tcr = (1<<0); //Habilito nuevamente el Timer
}

//Desactiva las interrupciones de timer y captura, y apaga el LED
void terminarLectura(){
	*t0tcr = (1<<1); //Reseteo y deshabilito el timer
	*t0mcr &= ~(1<<0); //Apago interrupciones por match
	*t0ccr &= ~(1<<2); //Apago interrupciones por captura
	actualizar_PWM(7);	//apago los leds cuando no estoy leyendo
	*t0tcr = (1<<0); //Habilito nuevamente el Timer
}

//Rutinas de interrupcion

void TIMER0_IRQHandler(){

	//Si la interrupcion fue por captura calculo la diferencia con el valor anterior y la guardo en suma_captura
	if(*t0ir & (1<<4)){
		*t0ir |= (1<<4); //Bajo la bandera
		if(vuelta_captura==0){
			tiempo1 = *t0cr0;
		}
		else{
			tiempo2 = *t0cr0;
			suma_captura = (tiempo2 - tiempo1)/100;
			tiempo1 = tiempo2;
		}
		vuelta_captura++;
		return;
	}

	//Interrupcion por Match
	//Mido primero R, luego G y por ultimo B
	//Guardo los valores en un vector de resultados
	//Al tener los 3 valores y la decision del color leido, envio los 4 datos por puerto serie al Arduino
	if (*t0ir & (1<<0)){
		*t0ir |= (1<<0); //Bajo la bandera

		switch(color_leyendo){
			case 0:
				leer_rojo();
				rgb_values[2] = map(suma_captura,B_MIN,B_MAX,0,255);
				break;
			case 1:
				leer_verde();
				rgb_values[0] = map(suma_captura,R_MIN,R_MAX,0,255);
				break;
			case 2:
				leer_azul();
				rgb_values[1] = map(suma_captura,G_MIN,G_MAX,0,255);
				break;
		}


		if (color_leyendo == 0 && !primeraVez){ //envio solo cuando tengo los 3 valores listos
			//decidir color
			int colorDetectado = classify();
			enviar(colorDetectado);
			actualizar_PWM(colorDetectado);
		}

		//Reinicio las variables
		vuelta_captura = 0;
		suma_captura = 0;
		color_leyendo = (color_leyendo+1)%3; //Reinicio la lectura
		if(primeraVez && (color_leyendo == 0)){//aca entro la primera vez que tengo los 3 colores listos para enviar
			primeraVez = 0;
		}
		return;
	}
	return;
}


//Si recibi algo por puerto serie, identifico el caracter
//Con una 'i' inicio la lectura, con una 'f' la termino.
void UART0_IRQHandler(){
	charRecibido = *u0rbr;

	if(charRecibido == 'i')		//comienzo a leer el sensor
		comenzarLectura();

	if(charRecibido == 'f')		//dejo de leer el sensor
		terminarLectura();
	return;
}
