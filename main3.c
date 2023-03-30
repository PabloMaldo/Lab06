/*
 * File:   main.c
 * Author: pablo
 *
 * Created on March 24, 2023, 9:57 PM
 */

// CONFIG1
#pragma config FOSC = INTRC_NOCLKOUT// Oscillator Selection bits (INTOSCIO oscillator: I/O function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF      // RE3/MCLR pin function select bit (RE3/MCLR pin function is digital input, MCLR internally tied to VDD)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF      // Brown Out Reset Selection bits (BOR disabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal/External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)
#pragma config LVP = OFF        // Low Voltage Programming Enable bit (RB3 pin has digital I/O, HV on MCLR must be used for programming)

// CONFIG2
#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits (Write protection off)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

// LIBRERIAS
#include <xc.h>
#include <stdint.h>

/*
 * Variables globales
 */
int flags;
int unidades;
int decenas;
int centenas;
const unsigned char segmentos[] = { 0b00111111, // 0
                                    0b00000110, // 1
                                    0b01011011, // 2
                                    0b01001111, // 3
                                    0b01100110, // 4
                                    0b01101101, // 5
                                    0b01111101, // 6
                                    0b00000111, // 7
                                    0b01111111, // 8
                                    0b01100111};// 9

/*
 * Constantes
 */
#define _tmr0_value 200
#define _XTAL_FREQ 8000000

/*
 * Prototipos de funcion
 */
void setup(void);
void conversor(int num);

/*
 * Interrupciones
 */

void __interrupt() isr(void)
{
    if(T0IF)
    {
        PORTD = 0;
        
        if (flags == 0)
        {
            PORTC = segmentos[centenas];
            PORTD = 0b0001;
//            PORTD = 0b0100;
            flags = 1;
        }
        else if (flags == 1)
        {
            PORTC = segmentos[decenas];
            PORTD = 0b0010;
            flags = 2;
        }
        else if (flags == 2)
        {
            PORTC = segmentos[unidades];
            PORTD = 0b0100;
//            PORTD = 0b0001;
            flags = 0;
        }
        
        INTCONbits.T0IF = 0;
        TMR0 = _tmr0_value;
    }
    return;
}

void main(void) {
    
    setup();
    while(1){
        ADCON0bits.CHS = 0; 
        ADCON0bits.GO = 1; // Iniciar la conversión
        while(ADCON0bits.GO); // Esperar a que termine la conversión
        int adc_value_1 = ADRESH; // Leer el valor convertido
        PORTB = (char) adc_value_1;
        __delay_ms(10);
        
        ADCON0bits.CHS = 1; // Seleccionar el canal AN1 para la segunda entrada analógica
        ADCON0bits.GO = 1; // Iniciar la conversión
        while(ADCON0bits.GO); // Esperar a que termine la conversión
        int adc_value_2 = ADRESH; // Leer el valor convertido
        int volt = (adc_value_2 * 2);
        conversor((int) volt);
        __delay_ms(10);
    }
    return;
}

void setup(void){
    // CONFIGURACION DE ENTRADAS Y SALIDAS
    // Pines digitales
    ANSEL = 0;
    ANSELH = 0;
    // Puerto A como salida
    // TRISA = 0;
    TRISB = 0;
    TRISC = 0;
    TRISD = 0;
    
    // Configuracion oscilador interno
    OSCCONbits.IRCF = 0b111; // 8MHz
    OSCCONbits.SCS = 1;
    
    //Configuracion TMR0
    OPTION_REGbits.T0CS = 0; //Se selecciona el timer como temporizador
    OPTION_REGbits.PSA = 0; // Prescaler activado para TMR0
    OPTION_REGbits.PS = 0b111; // Prescaler 1:256
    TMR0 = _tmr0_value;
    
    //Configuracion de las interrupciones
    INTCONbits.T0IF = 0; // Bandera tmr0
    INTCONbits.T0IE = 1; // Enable tmr0
    INTCONbits.PEIE = 1; // Interrupciones perifericas
    INTCONbits.GIE = 1; // Interrupciones globales
    
    // Configuracion ADC
    ANSELbits.ANS0 = 1;
    ANSELbits.ANS1 = 1;
    TRISAbits.TRISA0 = 1;
    TRISAbits.TRISA1 = 1;
    ADCON0bits.ADCS = 0b10; // FOSC/32
    __delay_ms(1);
    ADCON1bits.ADFM = 0; //Justificado a la izquierda
    ADCON1bits.VCFG0 = 0; //Voltaje de ref. a GND
    ADCON1bits.VCFG1 = 0; //Voltaje de ref. a 5v
    ADCON0bits.ADON = 1; // ADC is enabled
    ADIF = 0;
    
    return;
}

void conversor (int num) {
    centenas = num / 100;
    num %= 100;
    decenas = num / 10;
    unidades = num % 10;
    
    if (centenas == 5){
        if (decenas >= 0){
            if (unidades >= 0){
                decenas = 0;
                unidades = 0;
            }
        }
    }
    return;
}