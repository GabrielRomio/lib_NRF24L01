
// PIC18F4550 Configuration Bit Settings

// 'C' source line config statements

#include <xc.h>
#include <plib/xlcd.h>
#include <delays.h>
#include <usart.h>
#include <pic18f4550.h>
#include <spi.h>
#include <NRF.h>
#include "lcd.h" //Biblioteca para o LCD conectado à PORTD ao invés de PORTB, devido à utilização desses pinos para outras funções

#define _XTAL_FREQ 8000000 // Usado como base para função __delay_ms()

// CONFIG1L
#pragma config PLLDIV = 3       // PLL Prescaler Selection bits (No prescale (4 MHz oscillator input drives PLL directly))
#pragma config CPUDIV = OSC1_PLL2// System Clock Postscaler Selection bits ([Primary Oscillator Src: /1][96 MHz PLL Src: /2])
#pragma config USBDIV = 1       // USB Clock Selection bit (used in Full-Speed USB mode only; UCFG:FSEN = 1) (USB clock source comes directly from the primary oscillator block with no postscale)

// CONFIG1H
#pragma config FOSC = INTOSC_HS    // Oscillator Selection bits (EC oscillator, CLKO function on RA6 (EC))
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enable bit (Fail-Safe Clock Monitor disabled)
#pragma config IESO = OFF       // Internal/External Oscillator Switchover bit (Oscillator Switchover mode disabled)

// CONFIG2L
#pragma config PWRT = ON       // Power-up Timer Enable bit (PWRT disabled)
#pragma config BOR = ON         // Brown-out Reset Enable bits (Brown-out Reset enabled in hardware only (SBOREN is disabled))
#pragma config BORV = 1         // Brown-out Reset Voltage bits (Minimum setting 2.05V)
#pragma config VREGEN = OFF     // USB Voltage Regulator Enable bit (USB voltage regulator disabled)

// CONFIG2H
#pragma config WDT = OFF         // Watchdog Timer Enable bit (WDT enabled)
#pragma config WDTPS = 32768    // Watchdog Timer Postscale Select bits (1:32768)

// CONFIG3H
#pragma config CCP2MX = OFF      // CCP2 MUX bit (CCP2 input/output is multiplexed with RC1)
#pragma config PBADEN = OFF      // PORTB A/D Enable bit (PORTB<4:0> pins are configured as analog input channels on Reset)
#pragma config LPT1OSC = OFF    // Low-Power Timer 1 Oscillator Enable bit (Timer1 configured for higher power operation)
#pragma config MCLRE = OFF       // MCLR Pin Enable bit (MCLR pin enabled; RE3 input pin disabled)

// CONFIG4L
#pragma config STVREN = OFF      // Stack Full/Underflow Reset Enable bit (Stack full/underflow will cause Reset)
#pragma config LVP = OFF         // Single-Supply ICSP Enable bit (Single-Supply ICSP enabled)
#pragma config ICPRT = OFF      // Dedicated In-Circuit Debug/Programming Port (ICPORT) Enable bit (ICPORT disabled)
#pragma config XINST = OFF      // Extended Instruction Set Enable bit (Instruction set extension and Indexed Addressing mode disabled (Legacy mode))

// CONFIG5L
#pragma config CP0 = OFF        // Code Protection bit (Block 0 (000800-001FFFh) is not code-protected)
#pragma config CP1 = OFF        // Code Protection bit (Block 1 (002000-003FFFh) is not code-protected)
#pragma config CP2 = OFF        // Code Protection bit (Block 2 (004000-005FFFh) is not code-protected)
#pragma config CP3 = OFF        // Code Protection bit (Block 3 (006000-007FFFh) is not code-protected)

// CONFIG5H
#pragma config CPB = OFF        // Boot Block Code Protection bit (Boot block (000000-0007FFh) is not code-protected)
#pragma config CPD = OFF        // Data EEPROM Code Protection bit (Data EEPROM is not code-protected)

// CONFIG6L
#pragma config WRT0 = OFF       // Write Protection bit (Block 0 (000800-001FFFh) is not write-protected)
#pragma config WRT1 = OFF       // Write Protection bit (Block 1 (002000-003FFFh) is not write-protected)
#pragma config WRT2 = OFF       // Write Protection bit (Block 2 (004000-005FFFh) is not write-protected)
#pragma config WRT3 = OFF       // Write Protection bit (Block 3 (006000-007FFFh) is not write-protected)

// CONFIG6H
#pragma config WRTC = OFF       // Configuration Register Write Protection bit (Configuration registers (300000-3000FFh) are not write-protected)
#pragma config WRTB = OFF       // Boot Block Write Protection bit (Boot block (000000-0007FFh) is not write-protected)
#pragma config WRTD = OFF       // Data EEPROM Write Protection bit (Data EEPROM is not write-protected)

// CONFIG7L
#pragma config EBTR0 = OFF      // Table Read Protection bit (Block 0 (000800-001FFFh) is not protected from table reads executed in other blocks)
#pragma config EBTR1 = OFF      // Table Read Protection bit (Block 1 (002000-003FFFh) is not protected from table reads executed in other blocks)
#pragma config EBTR2 = OFF      // Table Read Protection bit (Block 2 (004000-005FFFh) is not protected from table reads executed in other blocks)
#pragma config EBTR3 = OFF      // Table Read Protection bit (Block 3 (006000-007FFFh) is not protected from table reads executed in other blocks)

// CONFIG7H
#pragma config EBTRB = OFF      // Boot Block Table Read Protection bit (Boot block (000000-0007FFh) is not protected from table reads executed in other blocks)

//Entradas
#define CLEAR PORTEbits.RE0

char posic=0; //Variavel global com a posição do cursor no display
char CapsLockAtivo=0; //Indica se o Caps Lock está ativo

void DelayFor18TCY( void )
{
  Nop();
  Nop();
  Nop();
  Nop();
  Nop();
  Nop();
  Nop();
  Nop();
  Nop();
  Nop();
  Nop();
  Nop();
}

void DelayPORXLCD (void)
{
  Delay1KTCYx(60); // Delay of 15ms
                   // Cycles = (TimeDelay * Fosc) / 4
                   // Cycles = (15ms * 16MHz) / 4
                   // Cycles = 60,000
  return;
}

void DelayXLCD (void)
{
  Delay1KTCYx(20); // Delay of 5ms
                   // Cycles = (TimeDelay * Fosc) / 4
                   // Cycles = (5ms * 16MHz) / 4
                   // Cycles = 20,000
  return;
}
 
 void InicializaLCD()
 {
    OpenXLCD(FOUR_BIT & LINES_5X7);
    while(BusyXLCD()); //Aguarda controlador do display livre
    WriteCmdXLCD(0x01); //Limpa display
    while(BusyXLCD());
    WriteCmdXLCD(CURSOR_ON & BLINK_OFF);
    SetDDRamAddr(0x00);//Posição do cursor no display
 }
 
 void LimpaLCD()
 {
    while(BusyXLCD()); //Aguarda controlador do display livre
    WriteCmdXLCD(0x01); //Limpa display
    posic=0;
 }

 void Delay_ms(int tempo)
{
   int i;
   for (i=0;i<tempo;i++)
   {
      __delay_ms(1);
   }
}
 
 void CorrigePosicao()
 {
    if (posic>0x0F && posic <0x40){
        posic=0x40;
        SetDDRamAddr(posic);
    }
    else if (posic>0x4F){
        posic=0;
        SetDDRamAddr(posic);
    }
}
 
 char EscreveCaracter(char tecla)
 {
     static char digitoAnterior=0; //Ultimo caracter inserido no display
     static char teclaAnterior=0; //Ultima tecla pressionada
     char auxTroca=1;
     
     if(tecla!=0 && tecla!=10 && tecla!=12 && tecla!=teclaAnterior && teclaAnterior!=0 && teclaAnterior!=10 && teclaAnterior!=12){
            posic++;
            SetDDRamAddr(posic);
            CorrigePosicao();
     }
     
     teclaAnterior=tecla;
     
     switch (tecla){
         case 0: //"Reset"
             teclaAnterior=0;
             digitoAnterior=0;
             auxTroca=0;
             
             break;
             
         case 1: //Tecla 1
                digitoAnterior=49;
            
            break;
             
         case 2: //Tecla 2
            if (digitoAnterior>=97 && digitoAnterior<99)
                digitoAnterior++;
            else if (digitoAnterior==99)
                digitoAnterior=tecla+48;
            else
                digitoAnterior=97;
            
            break;
          
         case 3: //Tecla 3
             if (digitoAnterior>=100 && digitoAnterior<102)
                digitoAnterior++;
            else if (digitoAnterior==102)
                digitoAnterior=tecla+48;
            else
                digitoAnterior=100;
            
            break;
             
         case 4: //Tecla 4
            if (digitoAnterior>=103 && digitoAnterior<105)
                digitoAnterior++;
            else if (digitoAnterior==105)
                digitoAnterior=tecla+48;
            else
                digitoAnterior=103;
            
            break;
             
         case 5: //Tecla 5
            if (digitoAnterior>=106 && digitoAnterior<108)
                digitoAnterior++;
            else if (digitoAnterior==108)
                digitoAnterior=tecla+48;
            else
                digitoAnterior=106;
            
            break;
         
         case 6: //Tecla 6
            if (digitoAnterior>=109 && digitoAnterior<111)
                digitoAnterior++;
            else if (digitoAnterior==111)
                digitoAnterior=tecla+48;
            else
                digitoAnterior=109;
            
            break;
             
         case 7: //Tecla 7
            if (digitoAnterior>=112 && digitoAnterior<115)
                digitoAnterior++;
            else if (digitoAnterior==115)
                digitoAnterior=tecla+48;
            else
                digitoAnterior=112;
            
            break;
             
         case 8: //Tecla 8
            if (digitoAnterior>=116 && digitoAnterior<118)
                digitoAnterior++;
            else if (digitoAnterior==118)
                digitoAnterior=tecla+48;
            else
                digitoAnterior=116;
            
            break;
             
         case 9: //Tecla 9
            if (digitoAnterior>=119 && digitoAnterior<122)
                digitoAnterior++;
            else if (digitoAnterior==122)
                digitoAnterior=tecla+48;
            else
                digitoAnterior=119;
            
            break;
            
         case 10: //Tecla 10
             if (posic==0)
                posic=0x4F;
             else if (posic==0x40)
                 posic=0x0F;
             else
                 posic--;
             auxTroca=0;
             digitoAnterior=0;
             SetDDRamAddr(posic);
             
             break;
             
         case 11: //Tecla 11
            if (digitoAnterior==32)
                digitoAnterior=48;
            else
                digitoAnterior=32;
            
            break;
         
         case 12: //Tecla 12
             auxTroca=0;
             digitoAnterior=0;
             posic++;
             SetDDRamAddr(posic);
             
             break;
     }
     
     if ((tecla>=1 && tecla<=9) || (tecla==11)){ //Imprime caracter no display
        if (digitoAnterior>=97 && digitoAnterior<=122) //Imprime sempre letras maiusculas
            putcXLCD(digitoAnterior-32);
        else
            putcXLCD(digitoAnterior);
        SetDDRamAddr(posic);
     }
             
     CorrigePosicao();
     return auxTroca;
 }

void main() //Programa principal
{
   OSCCON = 0b01000111; //Habilita oscilador interno
   int tempo=0;
   int testaZero=0;
   char teclaOk;
   char auxTroca=0; //Retorno da função escreve caracter que permitirá 
                    //ou bloqueará o avanço automático da posição após temporização
   ADCON1 = 0x0F;
   TRISEbits.RE0 = 1; //Porta E0 como entrada

   RF_CONFIG_SPI();          // Inicia comunicação SPI
   RF_CONFIG(0x40,0x08);     // Configurar módulo RF (canal e direção).
   RF_ON();                  // Ativar o módulo RF.
   
   InicializaLCD();
   
   unsigned char ret1=0;
   unsigned char tecla=0;
   
   teclaOk=1;
   
   while(1){       
    ret1 = RF_RECEIVE();
    if ( (ret1 == 0) || (ret1 == 1) )
    {
          tecla=RF_DATA[0];  // Recebe o dado enviado pelo transmissor através da variável RF_DATA[0].
    }
        
        if (tecla==0){ //Aguarda tecla ser desacionada para permitir entrar com novo caracter no display
            if(testaZero>=2){
                testaZero=0;
                teclaOk=1;
            }
            else
                testaZero=testaZero+1;
        }
        else
            testaZero=0;
        
        if (tempo>=200 && auxTroca==1){ //Avança posição no display após tempo sem pressionar a mesma tecla
            posic++;
            SetDDRamAddr(posic);
            CorrigePosicao();
            auxTroca=EscreveCaracter(0); //Condições iniciais da escrita de caracteres
        }
        
        if (tecla!=0 && teclaOk==1){ //Escreve caracter na tela se houve mudança na tecla e o debouncer estiver concluido
            auxTroca=EscreveCaracter(tecla);
            tempo=0;
            teclaOk=0;
        }
        
        while (CLEAR==1){ //Limpa display
            LimpaLCD();
            auxTroca=EscreveCaracter(0); //Condições iniciais da escrita de caracteres
        }
        
        __delay_ms(10);
        tempo=tempo+10;
    }
}