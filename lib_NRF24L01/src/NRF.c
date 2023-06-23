//********************************************************************************
//*    lib_rf2gh4_10.h                                                           *
//*     version: 1.0                                                             *
//*     Esta librería contiene las funciones necesarias para gestionar el módulo *
//*    RF2GH4 con programas del compilador CCS                                   *
//*    Copyright (C) 2007  Bizintek Innova S.L.                                  *
//********************************************************************************
//*    This program is free software; you can redistribute it and/or modify      *
//*    it under the terms of the GNU General Public License as published by      *
//*    the Free Software Foundation; either version 2 of the License, or         *
//*    (at your option) any later version.                                       *
//*                                                                              *
//*    This program is distributed in the hope that it will be useful,           *
//*    but WITHOUT ANY WARRANTY; without even the implied warranty of            *
//*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
//*    GNU General Public License for more details.                              *
//*                                                                              *
//*    You should have received a copy of the GNU General Public License along   *
//*    with this program; if not, write to the Free Software Foundation, Inc.,   *
//*    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.               *
//********************************************************************************


//********************
//*   DEFINICIONES   *
//********************

#include <spi.h>

// PORTB
#define RF_IRQ        PORTBbits.RB2//PIN_B2 //#define RF_IRQ        PIN_B2
//#define RF_IRQ_TRIS   TRISB,0 //#define RF_IRQ_TRIS   TRISB,2

// PORTC
#define   RF_CS       PORTCbits.RC1//PIN_C1
#define   RF_CE       PORTCbits.RC2//PIN_C2
#define   SCK         PORTBbits.RB1//PIN_B1
#define   SDI         PORTBbits.RB0//PIN_B0
#define   SDO         PORTCbits.RC7//PIN_C7
                      
/*#define   RF_CS_TRIS  TRISC,1
#define   RF_CE_TRIS  TRISC,2
#define   SCK_TRIS    TRISC,3
#define   SDI_TRIS    TRISC,4
#define   SDO_TRIS    TRISC,5*/



//*****************
//*   VARIABLES   *
//*****************
/*#BYTE TRISA     =  0x85
#BYTE TRISB     =  0x86
#BYTE TRISC     =  0x87
#BYTE INTCON    =  0x0B*/

//Variables internas
static char interRF;//int1        interRF;
static int noRF;//int16       noRF;
static char RCVNW=0;//int1        RCVNW=0;
static char DATA_N_SND=0;//int8        DATA_N_SND=0;
static char DATA_N_RCV=0;//int8        DATA_N_RCV=0;

//Variables configurables
static char RF_DATA[8];//int8        RF_DATA[8];
static char RF_DIR;//int8        RF_DIR;


//**************
//*   CÓDIGO   *
//**************

//*****************************************************
//*               RF_CONFIG_SPI()                     *
//*****************************************************
//*Descripción: La función configura el módulo SPI del*
//*microcontrolador.En ella se especifica como salida *
//*SDO y como entrada SDI entre otros parámetros del  *
//*protocolo SPI.                                     *
//*****************************************************
//*Variables de entrada:                              *
//*Variables de salida:                               *
//*****************************************************
void RF_CONFIG_SPI()
{
   //Configuración I/O.
   TRISBbits.RB1=1;//bit_clear(SCK_TRIS);
   TRISBbits.RB0=1;//bit_set(SDI_TRIS);
   TRISCbits.RC7=1;//bit_clear(SDO_TRIS);

   //Configuración módulo comunicaciones
   OpenSPI(SPI_FOSC_16, MODE_00, SMPEND); //Para o pic18f4550 usar clock div 16
   //setup_spi(SPI_MASTER|SPI_L_TO_H|SPI_XMIT_L_TO_H
   //|SPI_CLK_DIV_4|SPI_SAMPLE_AT_END);
}
//*****************************************************





//*****************************************************
//*                    RF_INT_EN()                    *
//*****************************************************
//*Descripción:Se encarga de habilitar la interrupción*
//*externa (RB0) utilizada por el módulo de RF en la  *
//*recepción de datos.                                *
//*****************************************************
//*Variables de entrada:                              *
//*Variables de salida:                               *
//*****************************************************
void RF_INT_EN()
{
   //Habilitar interrupciones externas con flanco de
   //bajada.
   INTCON = INTCON & 0b00111111;//disable_interrupts(global);//desabilita todas as interrupções
   INTCON2 = INTCON2 | 0b10000000; //habilita pull-ups
   INTCON2 = INTCON2 & 0b11101111; //borda de descida para int2 - RB2
   INTCON3 = INTCON3 | 0b10010010;
   //INTCON3 = INTCON3 & 0b11110111;
   enable_interrupts(int_ext);//interrupção externas //para o pic18f4550 habilitar interrupção gerada por rb2 (int_ext2)
   ext_int_edge( H_TO_L );//interrupções com borda de descida //para o pic18f4550 (2, H TO L)
   TRISBbits.RB2=1;//bit_set(RF_IRQ_TRIS);
   INTCON = INTCON | 0b11000000;//enable_interrupts(global);//habilita todas interrupções
}
//*****************************************************





//*****************************************************
//*        RF_CONFIG(int canal, int dir)              *
//*****************************************************
//*Descripción:Esta función se encarga de configurar  *
//*el transceptor habilitando su propia dirección de  *
//*escucha y el canal entre otros parámetros.         *
//*****************************************************
//*Variables de entrada:- Canal                       *
//*                     - Direccion                   *
//*Variables de salida:                               *
//*****************************************************
void RF_CONFIG(int canal, int dir)
{

   TRISCbits.RC1=0;//bit_clear(RF_CS_TRIS);
   TRISBbits.RB2=1;//bit_set(RF_IRQ_TRIS);
   TRISCbits.RC2=0;//bit_clear(RF_CE_TRIS);

   RF_CE=0;//output_low(RF_CE);

   // TX_ADDR (0xFF)
   //Configuración de la dirección de envio aleatoria.
   //En la función de enviar se configura la direccion
   //deseada por el usuario.
   RF_CS=0;//output_low(RF_CS);
   WriteSPI(0x30);
   WriteSPI(0xFF);
   WriteSPI(0xC2);
   WriteSPI(0xC2);
   WriteSPI(0xC2);
   WriteSPI(0xC2);
   RF_CS=1;//output_high(RF_CS);

   // RX_ADDR_P0 (0xFF) ACK
   //Configuración de la direccióndel Pipe0 para la
   //recepción de ACK.
   RF_CS=0;//output_low(RF_CS);
   WriteSPI(0x2A);
   WriteSPI(0xFF);
   WriteSPI(0xC2);
   WriteSPI(0xC2);
   WriteSPI(0xC2);
   WriteSPI(0xC2);
   RF_CS=1;//output_high(RF_CS);

   // RX_ADDR_P1 (dir)
   //Configuración de la direccióndel Pipe1 para la
   //recepción de tramas.
   RF_CS=0;//output_low(RF_CS);
   WriteSPI(0x2B);
   WriteSPI(dir);
   WriteSPI(0xC2);
   WriteSPI(0xC2);
   WriteSPI(0xC2);
   WriteSPI(0xC2);
   RF_CS=1;//output_high(RF_CS);

   // RX_ADDR_P2 (0x00) BROADCAST
   //Configuración de la direccióndel Pipe2 para la
   //recepción de tramas
   RF_CS=0;//output_low(RF_CS);
   WriteSPI(0x2C);
   WriteSPI(0x00);
   RF_CS=1;//output_high(RF_CS);

   // EN_AA
   //Habilitar AutoAck en los Pipe0,Pipe1 y Pipe2.
   RF_CS=0;//output_low(RF_CS);
   WriteSPI(0x21);
   WriteSPI(0x07);
   RF_CS=1;//output_high(RF_CS);

   // EN_RXADDR
   //Habilitar los Pipe0,Pipe1 y Pipe2.
   RF_CS=0;//output_low(RF_CS);
   WriteSPI(0x22);
   WriteSPI(0x07);
   RF_CS=1;//output_high(RF_CS);

   // SETUP_AW
   //Configuración de la longitud de las direcciones.
   //Direcciones de 5 bytes.
   RF_CS=0;//output_low(RF_CS);
   WriteSPI(0x23);
   WriteSPI(0x03);
   RF_CS=1;//output_high(RF_CS);

   //SETUP_RETR
   //Configuración de las retrasmisiones en la transmisión.
   //Diez retransmisiones cada 336us.
   RF_CS=0;//output_low(RF_CS);
   WriteSPI(0x24);
   WriteSPI(0x0A);
   RF_CS=1;//output_high(RF_CS);

   //RF_CH
   //Configuración del canal.
   //Canal elegido por el usuario (0x01 - 0x7F).
   RF_CS=0;//output_low(RF_CS);
   WriteSPI(0x25);
   WriteSPI(canal);
   RF_CS=1;//output_high(RF_CS);

   //RF_SETUP
   //Configuración aspectos RF.
   //Ganancia máxima de LNA, 0dBm potencia de salida y 1Mbps de velocidad.
   RF_CS=0;//output_low(RF_CS);
   WriteSPI(0x26);
   WriteSPI(0x07);
   RF_CS=1;//output_high(RF_CS);

   //STATUS
   //Reseteo del registro STATUS
   RF_CS=0;//output_low(RF_CS);
   WriteSPI(0x27);
   WriteSPI(0x70);
   RF_CS=1;//output_high(RF_CS);

   //RX_PW_P0
   //Nº de bytes en Pipe0.
   //1 byte (ACK).
   RF_CS=0;//output_low(RF_CS);
   WriteSPI(0x31);
   WriteSPI(0x01);
   RF_CS=1;//output_high(RF_CS);

   //RX_PW_P1
   //Nº de bytes en Pipe1.
   //10 byte (Direccion emisor y trama).
   RF_CS=0;//output_low(RF_CS);
   WriteSPI(0x32);
   WriteSPI(0x0A);
   RF_CS=1;//output_high(RF_CS);

   //RX_PW_P2
   //Nº de bytes en Pipe2.
   //10 byte (Direccion emisor y trama).
   RF_CS=0;//output_low(RF_CS);
   WriteSPI(0x33);
   WriteSPI(0x0A);
   RF_CS=1;//output_high(RF_CS);
}
//*****************************************************








//*****************************************************
//*                    RF_ON()                         *
//*****************************************************
//*Descripción:Esta rutina activa el módulo de        *
//*radiofrecuencia en modo escucha para poder recibir *
//*datos enviados a su dirección.                     *
//*****************************************************
//*Variables de entrada:                              *
//*Variables de salida:                               *
//*****************************************************
void RF_ON()
{

   RF_CE=0;//output_low(RF_CE);

   // CONFIG
   //Se activa el modulo, se pone en recepción,
   //se activa el CRC para que utilice 2 bytes.
   RF_CS=0;//output_low(RF_CS);
   WriteSPI(0x20);
   WriteSPI(0x07);  //WriteSPI(0x0F); //para o PIC18F4550 usar 0x07
   RF_CS=1;//output_high(RF_CS);

   __delay_ms(2);
   RF_CE=1;//output_high(RF_CE);
   __delay_us(150);
}
//*****************************************************









//*****************************************************
//*                 RF_OFF()                         *
//*****************************************************
//*Descripción:Este procedimiento desactiva el módulo *
//*de radiofrecuencia.                                *
//*****************************************************
//*Variables de entrada:                              *
//*Variables de salida:                               *
//*****************************************************
void RF_OFF()
{
   RF_CE=0;//output_low(RF_CE);

   // CONFIG
   //Se desactiva el modulo
   RF_CS=0;//output_low(RF_CS);
   WriteSPI(0x20);
   WriteSPI(0x0C);
   RF_CS=1;//output_high(RF_CS);
}
//*****************************************************








//*****************************************************
//*                 RF_SEND()                         *
//*****************************************************
//*Descripción:Esta función envía 8 Bytes de datos a  *
//*la dirección indicada informando de la correcta    *
//*recepción en el destinatario.                      *
//*****************************************************
//*Variables de entrada:- RF_DATA[]                   *
//*                     - RF_DIR
//*Variables de salida: -                             *
//*Salida:              - 0: Envío correcto (ACK OK)  *
//*                     - 1: No recepcibido (NO ACK)  *
//*                     - 2: No enviado               *
//*****************************************************
int RF_SEND()
{
   int i;
   int estado;

   //Comparacoes para testar bit_test(INTCON,7)
   char aux1;
   aux1=INTCON | 0b01111111;
   //------------------------------------------

   if(aux1==255)//if(bit_test(INTCON,7))
      interRF=1;
   else
      interRF=0;

   disable_interrupts(GLOBAL);

   // INICIO
   RF_CE=0;//output_low(RF_CE);

   //STATUS
   //Reseteo del registro STATUS
   RF_CS=0;//output_low(RF_CS);
   WriteSPI(0x27);
   WriteSPI(0x70);
   RF_CS=1;//output_high(RF_CS);

   // EN_RXADDR
   //Se habilita el Pipe0 para la recepción del ACK
   RF_CS=0;//output_low(RF_CS);
   WriteSPI(0x22);
   WriteSPI(0x01);
   RF_CS=1;//output_high(RF_CS);

   // TX_ADDR
   //Se configura la dirección de transmisión=RF_DIR
   RF_CS=0;//output_low(RF_CS);
   WriteSPI(0x30);
   WriteSPI(RF_DIR);
   WriteSPI(0xC2);
   WriteSPI(0xC2);
   WriteSPI(0xC2);
   WriteSPI(0xC2);
   RF_CS=1;//output_high(RF_CS);

   // RX_ADDR_P0
   //Para la recepción del ACK se debe configurar el Pipe0 con
   //la misma dirección a trasmitir.
   RF_CS=0;//output_low(RF_CS);
   WriteSPI(0x2A);
   WriteSPI(RF_DIR);
   WriteSPI(0xC2);
   WriteSPI(0xC2);
   WriteSPI(0xC2);
   WriteSPI(0xC2);
   RF_CS=1;//output_high(RF_CS);

   // RX_ADDR_P1
   //Se mete en RF_DIR la direccion propia.
   //De esta manera el receptor sabe la dirección
   //del transmisor.
   RF_CS=0;//output_low(RF_CS);
   WriteSPI(0x0B);
   RF_DIR=ReadSPI(0);
   ReadSPI(0);
   ReadSPI(0);
   ReadSPI(0);
   ReadSPI(0);
   RF_CS=1;//output_high(RF_CS);

   // W_TX_PAYLOAD
   //Se manda los datos al transductor
   RF_CS=0;//output_low(RF_CS);
   WriteSPI(0xA0);

   DATA_N_SND++;
   WriteSPI(DATA_N_SND);
   WriteSPI(RF_DIR);
   for (i=0;i<8;i++)
      WriteSPI(RF_DATA[i]);

   RF_CS=1;//output_high(RF_CS);

   // CONFIG
   //Se pasa a modo transmisión.
   RF_CS=0;//output_low(RF_CS);
   WriteSPI(0x20);
   WriteSPI(0x0E);
   RF_CS=1;//output_high(RF_CS);

   // Pulso de comienzo de envío
   RF_CE=1;//output_high(RF_CE);
   __delay_us(15);
   RF_CE=0;//output_low(RF_CE);

   noRF=0;

   while (RF_IRQ==1) {//while (input(RF_IRQ)==1) {
      noRF++;
      //Si no da respuesta en 7ms, no se ha enviado.
      if(noRF==500){
        break;
      }
   }


   // STATUS
   //Lectura del estado en el registro estatus.
   RF_CS=0;//output_low(RF_CS);
   estado=ReadSPI(0x27);
   WriteSPI(0x70);
   RF_CS=1;//output_high(RF_CS);


   // EN_RXADDR
   //Habilitar los Pipe0,Pipe1 y Pipe2.
   RF_CS=0;//output_low(RF_CS);
   WriteSPI(0x22);
   WriteSPI(0x07);
   RF_CS=1;//output_high(RF_CS);

      // TX_FLUSH
   //Limpieza de la FIFO de salida
   RF_CS=0;//output_low(RF_CS);
   WriteSPI(0xE1);
   RF_CS=1;//output_high(RF_CS);

   // CONFIG
   //Paso a modo recepción
   RF_CS=0;//output_low(RF_CS);
   WriteSPI(0x20);
   WriteSPI(0x07); //WriteSPI(0x0F); //para o PIC18F4550 usar 0x07
   RF_CS=1;//output_high(RF_CS);

   // FIN
   RF_CE=1;//output_high(RF_CE);

   __delay_us(150);

   //Si no da respuesta en 7ms, no se ha enviado.
   if(noRF==500){
        if(interRF==1)
        enable_interrupts(GLOBAL);
        clear_interrupt(int_ext);
        return(2);
   }

   //Comparacoes para testar (bit_test(estado,4)==0) && (bit_test(estado,5)==1)
   char aux2, aux3;
   aux2=estado & 0b00010000;
   aux3=estado | 0b11011111;
   //------------------------------------------

   //estado
   //Chequeo de los bit del registro STATUS que indican si se ha recibido
   //ACK y si se ha terminado las retrasmisiones sin ningun ACK.
   if ((aux2==0) && (aux3==255)){ //if ((bit_test(estado,4)==0) && (bit_test(estado,5)==1)){
      if(interRF==1)
      enable_interrupts(GLOBAL);
      clear_interrupt(int_ext);
      return(0);
      }
   else{
      if(interRF==1)
      enable_interrupts(GLOBAL);
      clear_interrupt(int_ext);
      return(1);
      }
}
//*****************************************************









//*****************************************************
//*                 RF_RECEIVE()                      *
//*****************************************************
//*Descripción: Esta rutina se encarga de comprobar si*
//*se ha producido una recepción y de ser así,        *
//*devuelve la trama recibida.                        *
//*****************************************************
//*Variables de entrada:-                             *
//*Variables de salida: - RF_DATA[]                   *
//*                     - RF_DIR                      *
//*Salida:         - 0: Recepción correcta y única    *
//*                - 1: Recepción correcta y múltiple *
//*                - 2: No se ha producido recepción  *
//*                - 3: No se ha producido recepción  *
//*****************************************************
int RF_RECEIVE()
{

   int i;
   int mas;
   int estado;

   if (RF_IRQ==1 && RCVNW==0){
      return (2);
      }

   //STATUS
   //Lectura y reseteo del registro STATUS
   RF_CS=0;//output_low(RF_CS);
   estado=ReadSPI(0x27);
   WriteSPI(0x70);
   RF_CS=1;//output_high(RF_CS);

   //Comparacoes para testar bit_test(estado,6)==0
   char aux1;
   aux1=estado & 0b01000000;
   //------------------------------------------

   //estado
   //Chequeo de la interrupción de recepción.
   if (aux1==0 && RCVNW==0){ //if (bit_test(estado,6)==0 && RCVNW==0){
      return(3);
      }

   //R_RX_PAYLOAD
   //Lectura de los datos recibidos.
   RF_CS=0;//output_low(RF_CS);
   WriteSPI(0x61);
   DATA_N_RCV=ReadSPI(0);
   RF_DIR=ReadSPI(0);
   for (i=0;i<8;i++)
   {
      RF_DATA[i]=ReadSPI(0);
   }
   RF_CS=1;//output_high(RF_CS);

   //FIFO_STATUS
   //Comprobación del estado de la FIFO de
   //recepción para comprobar si hay más datos
   RF_CS=0;//output_low(RF_CS);
   WriteSPI(0x17);
   mas=ReadSPI(0);
   RF_CS=1;//output_high(RF_CS);

   //Comparacoes para testar bit_test(mas,0)==0
   char aux2;
   aux2=mas & 0b00000001;
   //------------------------------------------

   if (aux2==0){ //if (bit_test(mas,0)==0){
      RCVNW=1;
      return(1);
   }
      RCVNW=0;
      return(0);
}
//*****************************************************









