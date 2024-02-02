/*
 * File:   main.c
 * Author: Sabina
 *
 * Created on 18 luglio 2022, 11.37
 */

#pragma config FOSC = HS        // Oscillator Selection bits (HS oscillator)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = ON       // Power-up Timer Enable bit (PWRT enabled)
#pragma config BOREN = ON       // Brown-out Reset Enable bit (BOR enabled)
#pragma config LVP = ON         // Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit (RB3/PGM pin has PGM function; low-voltage programming enabled)
#pragma config CPD = OFF        // Data EEPROM Memory Code Protection bit (Data EEPROM code protection off)
#pragma config WRT = OFF        // Flash Program Memory Write Enable bits (Write protection off; all program memory may be written to by EECON control)
#pragma config CP = OFF         // Flash Program Memory Code Protection bit (Code protection off)

//LCD PORTS
#define L_RS 0X04
#define L_EN 0X02

// LCD COMMANDS
#define L_ON 0x0F
#define L_OFF 0x08
#define L_CLR 0x01
#define L_L1 0x80
#define L_L2 0xC0
#define L_CR 0x0F
#define L_NCR 0x0C
#define L_CFG 0x38
#define L_CUR 0x0E

#define PICID '9'       // ID del PIC
#define BUFMAX 32       // Dimensione massima del buffer
#define ENDCHAR '\n'    // Carattere di fine messaggio

#define _XTAL_FREQ 8000000 // Frequenza di clock

#include <xc.h>

int number = 0;                // Variabile per il conteggio del tempo
char timeCountFlag = 0;         // Flag per indicare il conteggio del tempo
int timeCount = 0;              // Contatore del tempo trascorso
char dataReceivedBuffer[BUFMAX]; // Buffer per i dati ricevuti
char messageReceived[BUFMAX];    // Buffer per il messaggio ricevuto
char indexRC = 0;                // Indice del buffer
char isReceived = 0;             // Flag per indicare la ricezione di un messaggio

// Funzioni
void init_PIC(void);
//  LCD
void LCD_Init(void);
void LCD_Send(char, char);
void LCD_Write(char *);
// Timer
void init_timer(void);
// UART
void UART_init(long int);
// Altre funzioni
void clearBuff(char *, char, char *);

// Funzione principale
void main(void) {
    init_PIC();
    while(1)
    {
        // Se è stato ricevuto un messaggio
        if (isReceived)
        {   
            // Se l'ID del PIC nel messaggio corrisponde a quello atteso
            if(dataReceivedBuffer[0] == PICID)
            {
                char indexMessage = 2; //Il primo byte è l'ID del PIC, il secondo byte è il tempo del messaggio LCD.
                // Copia il messaggio ricevuto nel buffer del messaggio
                while(dataReceivedBuffer[indexMessage] != ENDCHAR) //Finchè non trova il carattere di fine messaggio
                { 
                   messageReceived[indexMessage-2] = dataReceivedBuffer[indexMessage];
                   indexMessage++;
                }
                // Termina la stringa del messaggio ricevuto
                messageReceived[indexMessage-2] = '\0';
                // Pulisce il display LCD e mostra il messaggio
                LCD_Send(L_CLR,1);
                LCD_Write(messageReceived);
            }
            isReceived = 0;
        }
    }
    return;
}

// Inizializza il PIC
void init_PIC()
{    
    // Imposta tutti i pin come output
    TRISA = TRISB = TRISC = TRISD = TRISE = 0x00;
    // Inizializza il display LCD
    LCD_Init();
    // Pulisce il buffer dei dati ricevuti
    clearBuff(dataReceivedBuffer, BUFMAX, &indexRC);
    // Abilita le interruzioni globali e periferiche
    INTCONbits.GIE = 1;
    INTCONbits.PEIE = 1;
    // Inizializza il timer
    init_timer();
    // Inizializza la comunicazione UART
    UART_init(9600);
}

// Invia un comando al display LCD
void LCD_Send(char data, char mode){
    PORTE |= L_EN;
    PORTD = data;   
    (mode) ? (PORTE &= ~L_RS) : (PORTE |= L_RS);
    __delay_ms(3);
    PORTE &= ~L_EN;
    __delay_ms(3);
    PORTE |= ~L_EN;
}

// Inizializza il display LCD
void LCD_Init(){
    PORTE &= ~L_RS;
    PORTE &= ~L_EN;
    __delay_ms(20);
    PORTE |= L_EN;
    LCD_Send(L_CFG, 1);
    __delay_ms(5);
    LCD_Send(L_CFG, 1);
    __delay_ms(1);
    LCD_Send(L_CFG, 1);
    LCD_Send(L_OFF, 1);
    LCD_Send(L_ON, 1);
    LCD_Send(L_CLR, 1);
    LCD_Send(L_CUR, 1);
    LCD_Send(L_NCR, 1); //UPD
    LCD_Send(L_L1, 1);
}

// Scrive sul display LCD
void LCD_Write(char *text)
{
    for(int j = 0; text[j] != 0; j++)
    {
        if (j == 16){LCD_Send(L_L2, 1);}
        if (text[j] == '\n'){LCD_Send(L_L1, 1);}
        LCD_Send(text[j], 0);
    }
}

// Inizializza il timer
void init_timer()
{
    INTCONbits.TMR0IE = 1;
    OPTION_REG = 0x07;
    TMR0 = 6;
}

// Inizializza la comunicazione UART
void UART_init(long int baudRate)
{
    TRISCbits.TRISC6 = 0;
    TRISCbits.TRISC7 = 1;
    TXSTA |= 0x24;
    SPBRG = (_XTAL_FREQ/(long)(16UL*baudRate))-1;
    RCSTA |= 0x90;
    PIE1bits.RCIE = 1;
}

// Pulisce un buffer di dati
void clearBuff(char *buf, char size, char *index)
{
    for(char i = 0; i < size; i++)
    {
        buf[i] = 0;
    }
    *index = 0;
}

// Interrupt
void __interrupt() ISR()
{
    if(INTCONbits.TMR0IF)
    {
        TMR0 = 6;
        
        // Se è attivo il conteggio del tempo (il messaggio è arrivato)
        if (timeCountFlag){
            // Imposta il tempo di visualizzazione del messaggio sul display LCD
            switch(dataReceivedBuffer[1]) {
                case '1': number = 300; break;
                case '2': number = 1800; break;
                case '3': number = 18000; break;
                case '4': number = 54000; break; 
                default: number = 300;
            }
            timeCount++;
            // Se è trascorso il tempo necessario, pulisce il display LCD
            if(timeCount > number) 
            {
                // Pulisci LCD
                LCD_Send(L_CLR, 1);
                // Pulisci buffer
                clearBuff(dataReceivedBuffer, BUFMAX, &indexRC);
                timeCountFlag = 0;
                timeCount = 0;
            }
        }
        INTCONbits.TMR0IF = 0;
    }
    
    // Riceve i dati dalla porta seriale
    if(PIR1bits.RCIF)
    {
        dataReceivedBuffer[indexRC] = RCREG;
        // Se il buffer è pieno o è stato ricevuto il carattere di fine messaggio
        if (indexRC >= BUFMAX || dataReceivedBuffer[indexRC] == ENDCHAR)
        {
            isReceived = 1;
            timeCountFlag = 1;
        }
        indexRC++;
        PIR1bits.RCIF = 0;
    }
}
