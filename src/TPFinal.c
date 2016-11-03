/*
===============================================================================
 Name        : TPFinal.c
 Author      : $(author)
 Version     :
 Copyright   : $(copyright)
 Description : main definition
===============================================================================
*/

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#include "math.h"
#endif

#include <cr_section_macros.h>


// TODO: insert other definitions and declarations here
#define S0 4
#define S1 5
#define S2 10
#define S3 11
#define INPUT 0
#define PUL_EXT 1
#define TIME 1
#define CANTIDAD_COLORES 8

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
unsigned int volatile *const t0ctcr = (unsigned int *) 0x40004000; //Seleccionar modo(TMR o Counter)
unsigned int volatile *const t0ccr = (unsigned int *) 0x40004028;
unsigned int volatile *const cr0 = (unsigned int *) 0x4000402C;
//Timer1
unsigned int volatile *const t1ir = (unsigned int *) 0x40008000;
unsigned int volatile *const t1tcr = (unsigned int *) 0x40008004; //Habilitar Timer
unsigned int volatile *const t1pr = (unsigned int *) 0x4000800C;
unsigned int volatile *const t1mcr = (unsigned int *) 0x40008014;
unsigned int volatile *const t1mr0 = (unsigned int *) 0x40008018;
unsigned int volatile *const t1ctcr = (unsigned int *) 0x40008070; //Seleccionar modo(TMR o Counter)
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

int rgb_values[3] = {0,0,0}; // Valores en escala RGB
int led_values[3] = {0,0,0}; //Valores para pasarle a los Match del PWM
int color_values[3][CANTIDAD_COLORES]; //Arreglo con los valores RGB para cada color
char color_names[CANTIDAD_COLORES]; //Arreglo con el nombre de cada color
int volatile vuelta_captura = 0; //para saber cuantas veces hice captura
int volatile tiempo1;
int volatile tiempo2; //variables para guardar valores del captura
int volatile suma_captura = 0;
int volatile frecuencia_promedio = 0;
int flag = 255; //flag de inicio  de transmision
int flagFin = 0;
int primeraVez = 1;


void config_pines();
void config_timer0();
void config_puerto_serie();
void config_PWM();
void leer_rojo();
void leer_verde();
void leer_azul();
void rgb_to_led(int rojo, int verde, int azul);
void actualizar_PWM();
void enviar(char colorDetectado);
char classify();

int main(void) {

	config_pines();
	*fio0clr |= (1<<S1);
	*fio0set |= (1<<S0);
	config_puerto_serie();
	config_timer0();
	//config_PWM();

    while(1) {

    }
    return 0 ;
}

void config_timer0(){
	*t0ctcr = 0; //Timer 0 como timer y no contador(ya viene asi por defecto)

	//Match
	*t0mr0 = 25000000 *TIME; //Interrumpir cada TIME segundo
	*t0pr = 0;  //Preescaler register en 0
	*t0mcr |= (1<<1);  //Configuramos para que resetee el TC

	//Captura
	*t0ccr |= (1<<0); //Captura en rising edge del pin 1.26
	*t0ccr &= ~(1<<2); //desactivo las interrupciones de captura
	*t0mcr &= ~(1<<0); //desactiva interrupciones por Match
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

	return;
}


void config_pines(){
	*fio0dir |= (1<<S0) | (1<<S1) | (1<<S2) | (1<<S3); //Pines de config del sensor como salida
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
	*pwm1mr0 |= 25000000/500; //Periodo del pulso(igual supuestamente al de Arduino https://learn.adafruit.com/adafruit-arduino-lesson-3-rgb-leds/theory-pwm)
	*pwm1mcr |= (1<<1); //Reseteo en Match0
	*pwm1ler |= (1<<0); //Hago efectivo el valor del Match
	*pwm1tcr |= (1<<3); //Configuro el modulo como PWM

	//Habilito las salidas 1,2 y 3
	*pwm1pcr |= (1<<9) | (1<<10) | (1<<11);

	return;
}

void leer_rojo(){
	*fio0clr |= (1<<S2) | (1<<S3);
	return;
}

void leer_verde(){
	*fio0set |= (1<<S2) | (1<<S3);
	return;
}

void leer_azul(){
	*fio0clr |= (1<<S2);
	*fio0set |= (1<<S3);
	return;
}

void actualizar_PWM(){
	rgb_to_led(rgb_values[0],rgb_values[1],rgb_values[2]); //Actualizo los valores del array led_values

	//Cambio el duty cicle actualizando los match
	*pwm1mr1 = led_values[0];
	*pwm1mr2 = led_values[1];
	*pwm1mr3 = led_values[2];

	//Hago que el cambio sea efectivo
	*pwm1ler |= (1<<1) | (1<<2) | (1<<3);
	return;
}

void rgb_to_led(int rojo, int verde, int azul){
	int rojo_led = 0;
	int verde_led = 0;
	int azul_led = 0;
	//Interpolacion
	///Falta hacer

	led_values[0] = rojo_led;
	led_values[1] = verde_led;
	led_values[2] = azul_led;
	return;
}

void enviar(char colorDetectado){

		while((*u0lsr & (1<<5))==0){} //Espero a que el buffer este vacio
		*u0thr = (flag);

		while((*u0lsr & (1<<5))==0){} //Espero a que el buffer este vacio
		*u0thr = (rgb_values[0] && 0xFF);
		while((*u0lsr & (1<<5))==0){} //Espero a que el buffer este vacio
		*u0thr = (rgb_values[1] && 0xFF);
		while((*u0lsr & (1<<5))==0){} //Espero a que el buffer este vacio
		*u0thr = (rgb_values[2] && 0xFF);
//		envio la letra correspondiente al color identificado
		while((*u0lsr & (1<<5))==0){} //Espero a que el buffer este vacio
		*u0thr = (colorDetectado && 0xFF);

		while((*u0lsr & (1<<5))==0){} //Espero a que el buffer este vacio
		*u0thr = (flagFin);
		//actualizar_PWM();
}

char classify(){
	int i_color;
	int ClosestColor = 0;
	float MaxDiff;
	float MinDiff = 1000.0;
	for (i_color = 0; i_color < 6; i_color ++) {
	  // compute Euclidean distances
	  float ED = sqrt(pow((color_values[0][i_color] - rgb_values[0]),2.0) +
	  pow((color_values[1][i_color] - rgb_values[1]),2.0) + pow((color_values[2][i_color] - rgb_values[2]),2.0));
	  MaxDiff = ED;
	  // find minimum distance
	  if (MaxDiff < MinDiff) {
		MinDiff = MaxDiff;
		ClosestColor = i_color;
	  }
	}
	return color_names[ClosestColor];
}

//Rutinas de interrupcion

void TIMER0_IRQHandler(){
	//Interrupcion por Match
	//Cada TIME segundos cambio de color a leer y mando por puerto serie el resultado

	if(*t0ir & (1<<4)){ //Interrupcion por captura
			*t0ir |= (1<<4); //Bajo la bandera
			if(vuelta_captura==0){
				tiempo1 = *cr0;
			}
			else{
				tiempo2 = *cr0;
				suma_captura = (tiempo2 - tiempo1)/100;
				tiempo1 = tiempo2;
			}
			vuelta_captura++;
			return;
		}

	if (*t0ir & (1<<0)){ //Interrupcion por Match
		*t0ir |= (1<<0); //Bajo la bandera

		switch(color_leyendo){
			case 0:
				leer_rojo();
				rgb_values[2] = suma_captura;
				if(primeraVez){
					rgb_values[2] = 0;
				}
				break;
			case 1:
				leer_verde();
				rgb_values[0] = suma_captura;
				if(primeraVez){
					rgb_values[0] = 0;
				}
				break;
			case 2:
				leer_azul();
				rgb_values[1] = suma_captura;
				if(primeraVez){
					rgb_values[1] = 0;

				}
				break;
		}

		//decidir color
		char colorDetectado = classify();
		if (color_leyendo == 0 && !primeraVez){		//envio solo cuando tengo los 3 valores listos
			enviar(colorDetectado);
			actualizar_PWM();
		}

			//Reinicio las variables
			vuelta_captura = 0;
			suma_captura = 0;
			color_leyendo = (color_leyendo+1)%3; //Reinicio la lectura
			if(primeraVez && (color_leyendo == 0)){		//aca entro la primera vez que tengo los 3 colores listos para enviar
				primeraVez = 0;
			}

		return;
	}
}

void EINT3_IRQHandler(){
	*io0intclr |= (1<<PUL_EXT); //Bajo la bandera
	*t0tcr = (1<<1); //Reseteo y deshabilito el timer
	*t0mcr ^= (1<<0); //Toggle a la interrupcion por match
	*t0ccr ^= (1<<2); //Toggle a la interrupcion por captura
	*t0tcr = (1<<0); //Habilito nuevamente el Timer
	return;
}

