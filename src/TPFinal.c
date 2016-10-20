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
#endif

#include <cr_section_macros.h>


// TODO: insert other definitions and declarations here
//Pin Registers
unsigned int volatile *const fio0dir = (unsigned int *) 0x20009C000;
unsigned int volatile *const fio0pin = (unsigned int *) 0x20009C014;
unsigned int volatile *const fio0set = (unsigned int *) 0x20009C018;
unsigned int volatile *const fio0clr = (unsigned int *) 0x20009C01C;
//Timer Registers
unsigned int volatile *const t0ir = (unsigned int *) 0x40004000;
unsigned int volatile *const t0tcr = (unsigned int *) 0x40004004; //Habilitar Timer
unsigned int volatile *const t0pr = (unsigned int *) 0x4000400C;
unsigned int volatile *const t0mcr = (unsigned int *) 0x40004014;
unsigned int volatile *const t0mr0 = (unsigned int *) 0x40004018;
unsigned int volatile *const t0ctcr = (unsigned int *) 0x40004000; //Seleccionar modo(TMR o Counter)
//Timer1
unsigned int volatile *const t1ir = (unsigned int *) 0x40008000;
unsigned int volatile *const t1tcr = (unsigned int *) 0x40008004; //Habilitar Timer
unsigned int volatile *const t1pr = (unsigned int *) 0x4000800C;
unsigned int volatile *const t1mcr = (unsigned int *) 0x40008014;
unsigned int volatile *const t1mr0 = (unsigned int *) 0x40008018;
unsigned int volatile *const t1ctcr = (unsigned int *) 0x40008070; //Seleccionar modo(TMR o Counter)
//Interrupt Registers
unsigned int volatile *const iser0 = (unsigned int *) 0xE000E100;
unsigned int volatile *const icer0 = (unsigned int *) 0xE000E180;
unsigned int volatile *const iabr0 = (unsigned int *) 0xE000E300;
unsigned int volatile *const io0intenr = (unsigned int *)0x40028090;
unsigned int volatile *const io0intclr = (unsigned int *)0x4002808C;

//Puerto Serie Registers
unsigned int volatile *const pconp = (unsigned int *) 0x400FC0C0;
unsigned int volatile *const u0dll = (unsigned int *) 0x4000C000;
unsigned int volatile *const u0dlm = (unsigned int *) 0x4000C004;
unsigned int volatile *const u0lcr = (unsigned int *) 0x4000C00C;
unsigned int volatile *const u0lsr = (unsigned int *) 0x4000C014;
unsigned int volatile *const u0thr = (unsigned int *) 0x4000C000;
unsigned int volatile *const pinsel0 = (unsigned int *) 0x4002C000;

//otros punteros y variables globales
uint8_t volatile transmitir = 0;

void config_pines();
void config_timer0();
void config_puerto_serie();

int main(void) {

	config_pines();
	config_timer0();
	config_puerto_serie();

    while(1) {

    }
    return 0 ;
}

void config_timer0(){ //Cada 1seg interrumpe para mandar dato
	*t0ctcr = 0; //Timer 0 como timer y no contador(ya viene asi por defecto)
	*t0mr0 = 25000000; //Interrumpir cada 1 segundo
	*t0pr = 0;  //Preescaler register en 0
	*t0mcr |= (1<<1) | (1<<0);  //Configuramos para que interrumpa al llegar al match y resetee el TC
	*iser0 |= (1<<1); //Habilito interrupciones por TMR0
	return;
}

void config_puerto_serie(){
	*pconp |= (1<<3); //Prendo UART
	//Dejo pclock para el UART en 25MHZ
	*u0lcr |= (1<<7); //Habilito acceso a los bits del divisor
	*u0dll = 163; //Baud_rate = 9585
	*u0dlm = 0;
	*u0lcr &= ~(1<<7); //Deshabilito acceso a bits divisor
	*pinsel0 |= (1<<4); //Confiuro pin 0.2
	*u0lsr |= (1<<0) | (1<<1); //8 bits
	*u0lsr &= ~(1<<2 | 1<<3); //1 bit de stop y sin pariedad
	return;
}

void TIMER0_IRQHandler(){
	while(!(*u0lsr & (1<<5))){ //Espero a que el buffer este vacio

	}
	*u0thr = transmitir; //Cargo el dato a transmitir
	transmitir++;
	return;
}

void config_pines(){
	return;
}
