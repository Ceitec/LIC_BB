/*
Autor:	Lukas
Soubor:	Hlavicka s daty
*/


//////////////////////////////////////////////////////////////////////////
/************************************************************************/
/*				Nutné definovat #define MicroControler dle uC           */
/*																		*/
/*#define MicroController Atmega8										*/
/*#define MicroController Atmega16										*/
/*#define MicroController Atmega32										*/
/*#define MicroController Atmega64										*/
/*#define MicroController Atmega128										*/
/*#define MicroController Atmega644p									*/
/*																		*/
/*																		*/
/*																		*/
/************************************************************************/
//////////////////////////////////////////////////////////////////////////


#ifndef AllInit_H_
#define AllInit_H_

#define F_CPU 14745600UL
//#define MicroController Atmega128


///////////////////////////////////////////////////////////////////////////
// Definování rychlosti sériové linky pro rychlou editaci #CPU 14745600UL.
///////////////////////////////////////////////////////////////////////////

#define	RS232_2400		383
#define RS232_4800		191
#define RS232_9600		95
#define RS232_14400		63
#define RS232_19200		47
#define RS232_28800		31
#define RS232_38400		23
#define RS232_57600		15
#define RS232_76800		11
#define RS232_115200	7
#define RS232_234000	3

//Definovani TRUE a FALSE
#define TRUE	1
#define	FALSE	0

//Definování uC
#define		Atmega8		0
#define		Atmega16	1
#define		Atmega32	2
#define		Atmega64	3
#define		Atmega128	4
#define		Atmega644p	5



//Deklarace procedur
void HB_LED_RXC(); /* Negace diody pro indikaci komunikace*/
void Text_to_Buffer(char Buffer); /*Uklada do Bufferu chary */
void Null_Buffer(); /* Vynuluje buffer */


#endif /* AllInit_H_ */