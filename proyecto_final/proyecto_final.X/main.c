
/*
 * Includes
 */

#include <xc.h>
#include <pic18.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "system_config.h"
#include "FM_Lcd_Easy.h"

/*
 * Variables globales
 */

char msj[16];

/*
 * Macros
 */

#define Led_Sys    3
#define Led_Sys_Tris    TRISF
#define Led_Sys_Lat     LATF
#define Led_Sys_Ansel   ANSELF

/*
 * Declaracion de funciones
 */

void Init_Internal_Oscillator(void);
void Init_Gpio_System(void);
void Init_ADCC_Module(void);

/*
 * Main
 */
int main(void) {
    /* Configurar el Oscilador Interno */
    Init_Internal_Oscillator();
    /* Configurar el ADC */
    Init_ADCC_Module();
    /* Configurar el GPIO */
    Init_Gpio_System();
    /* Configurar el LCD */
    FM_Lcd_Easy_Init();
    /* Mensaje por el LCD */
    FM_Lcd_Set_Cursor(ROW_1, COL_3);
    FM_Lcd_Send_String("PF MOTOR RPM");
    /* Bucle principal */
    while (true) 
    {
        /* Iniciar la conversi�n */
        ADCON0 |= (1 << _ADCON0_GO_NOT_DONE_POSITION);
        /* Esperar a que termine la conversi�n */
        while(ADCON0 & (1 << _ADCON0_GO_NOT_DONE_POSITION));
        uint16_t adc_val = (ADRESH << 8) | ADRESL;
        uint8_t val_percent = ((float)adc_val / 4095.0) * 100.0;
        sprintf(msj, "V=%-3u%%", val_percent);
        FM_Lcd_Set_Cursor(ROW_2, COL_1);
        FM_Lcd_Send_String(msj);
        Led_Sys_Lat ^= (1 << Led_Sys);
        __delay_ms(100);
    }
    return (EXIT_SUCCESS);
}

/*
 * Definicion de funciones
 */

void Init_ADCC_Module(void) {
    /* Limpiar los registros */
    ADCON0 = 0x00;
    ADCON1 = 0x00;
    ADCON2 = 0x00;

    /* Configurar el ADC */
    ADCON0 &= ~(1 << _ADCON0_ON_POSITION); // ADC Apagado
    ADCON0 &= ~(1 << _ADCON0_CONT_POSITION); // ADC Continuo
    ADCON0 &= ~(1 << _ADCON0_CS_POSITION); // Fadc = Fosc
    ADCON0 &= ~(1 << _ADCON0_GO_POSITION); // No se inicia la conv.

    ADCLK = 0x01; // ADCLK = 1 / FOSC / 4
    /* La justificaci�n hace referencia a la como deseamos
     * colocar los 12 bits del ADC en los registros del 
     * ADRESH y ADRESL, registros de 8 bits que componen
     * el registro ADRES de 16bits
     * Considerando:
     *         ADRESH                       ADRESL  
     * ------------------------   -------------------------
     * |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
     * -------------------------  -------------------------
     * 
     * FM = 1 - Justificado a la derecha
     *         ADRESH                       ADRESL  
     * -------------------------  -------------------------
     * |  |  |  |  | X| X| X| X|  | X| X| X| X| X| X| X| X|
     * -------------------------  -------------------------
     * 
     * FM = 0 - Justificado a la izquierda
     *        ADRESH                       ADRESL  
     * ------------------------   -------------------------
     * | X| X| X| X| X| X| X| X|  | X| X| X| X|  |  |  |  |
     * -------------------------  -------------------------
     */
    ADCON0 |= (1 << _ADCON0_FM_POSITION); // Right Justify

    /* Los registros ADCON1 ADCON2 ADCON3 son para el 
     * manejo de las caracter�sticas del m�dulo computacional
     * por ahora no lo usaremos */

    /* Elegimos los voltajes de referencia */
    ADREF &= ~(1 << _ADREF_NREF_POSITION); // NREF a VSS
    ADREF &= ~(0b11 << _ADREF_PREF0_POSITION); // PREF a VDD

    /* Elegimos el canal de conversi�n del ADCC */
    ADPCH = 0x00; // Elegimos la entrada anal�gica 0 del puerto A

    /* Como vamos a usar el RA0 como entrada anal�gica configuramos */
    TRISA |= (1 << _TRISA_TRISA0_POSITION); // RA0 como entrada
    ANSELA |= (1 << _ANSELA_ANSELA0_POSITION); // RA0 como an�logo

    /* Los registros 
     * ADPRE = ADC Precharge Time Control Register
     * ADACQ = ADC Acquisition Time Control Register
     * No ser�n usados por ahora */
    ADPRE = 0x00;
    ADACQ = 0x00;

    /* Encendemos el ADC */
    ADCON0 |= (1 << _ADCON0_ON_POSITION); // M�dulo ADC prendido
}

void Init_Gpio_System(void) {
    /* Configurar el GPIO */
    Led_Sys_Tris &= ~(1 << Led_Sys);
    Led_Sys_Ansel &= ~(1 << Led_Sys);
    Led_Sys_Lat &= ~(1 << Led_Sys);
}

void Init_Internal_Oscillator(void) {
    /* INTOSC a 8mhz */
    _clock_hfintosc_params_t my_clock;
    my_clock.frecuencia_clock = freq_clk_16MHZ;
    my_clock.divisor_clock = clock_div_1;
    FM_Hfintosc_Init(&my_clock);
}