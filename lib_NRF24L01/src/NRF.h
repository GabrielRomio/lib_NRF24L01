
//********************
//*   DEFINICOES  *
//********************

#include <xc.h>
#include <spi.h>
#include <delays.h>
#include <pic18f4550.h>

// PORTB
#define RF_IRQ        PORTBbits.RB2 //Variavel pode ser usada para gerar uma interrupção quando há um novo dado à ser lido. Neste caso não foi utilizado.

// PORTC
#define   RF_CS       PORTCbits.RC1 //Selecao do chip SPI
#define   RF_CE       PORTCbits.RC2 //Ativa modo RX ou TX
#define   SCK         PORTBbits.RB1 //Variavel referente ao CLOCK do SPI
#define   SDI         PORTBbits.RB0 //Variavel referente à entrada do SPI
#define   SDO         PORTCbits.RC7 //Variavel referente à saída do SPI
                      

//*****************
//*   VARIAVEIS   *
//*****************
/*#BYTE TRISA     =  0x85
#BYTE TRISB     =  0x86
#BYTE TRISC     =  0x87
#BYTE INTCON    =  0x0B*/

//Variaveis auxiliares
static char interRF;
static int noRF;
static char RCVNW=0;
static char DATA_N_SND=0;
static char DATA_N_RCV=0;

//Variaveis configuraveis
static char RF_DATA[8];//Variavel referente à transmissão/recepção de dados. Inserindo 8 no tamanho da variavel permite uma comunicação de até 8 bytes. Permite inserção de valores de 1 a 32 bytes.
static char RF_DIR;


//**************
//*   CÓDIGO   *
//**************

#define _XTAL_FREQ 8000000 // Usado como base para função __delay_ms()

//*****************************************************
//*               RF_CONFIG_SPI()                     *
//*****************************************************
//Inicializa SPI

void RF_CONFIG_SPI()
{
   //Configuração das entradas e saídas
   TRISBbits.RB1=0; //Define RB1 "SCK" como saída
   TRISBbits.RB0=1; //Define RB0 "SDI" como entrada
   TRISC = TRISC & 0b01111111; //Define RC7 "SDO" como saída. Declarado dessa forma para não gerar conflito com a biblioteca SPI.h

   OpenSPI(SPI_FOSC_4, MODE_00, SMPEND); //Configura comunicação SPI, com CLOCK = (clock interno)/4
                                         //Define clock com polaridade 0 e pulso gerado no começo do período
                                         //Lê o dado de entrada no final do período
}


//*****************************************************
//*                 RF_INT_EN()                       *
//*****************************************************
//Configura interrupção gerada por RB2-INT2 quando há um novo dado para ser lido
//Neste programa não foi utilizado, pois o programa ficará lendo os dados continuamente

void RF_INT_EN()
{
   INTCON = INTCON & 0b00111111;//Desabilita todas as interrupções
   INTCON2 = INTCON2 | 0b10000000; //Habilita pull-ups
   INTCON2 = INTCON2 & 0b11101111; //Borda de descida para int2 - RB2
   INTCON3 = INTCON3 | 0b10010000; //Habilita interrupção int2
   TRISBbits.RB2=1;
   INTCON = INTCON | 0b11000000;//Habilita todas interrupções
}


//*****************************************************
//*        RF_CONFIG(int canal, int dir)              *
//*****************************************************
//Função responsável pela configuração dos parametros referentes ao canal e direção pelos quais serão transmitidos/recebidos os dados

void RF_CONFIG(int canal, int dir)
{

   TRISC = TRISC & 0b11111101; //Define RC1 "RF_CS" como saída. Declarado dessa forma para não gerar conflito com a biblioteca SPI.h
   TRISBbits.RB2=1; //Define RB2 "RF_IRQ" como entrada
   TRISC = TRISC & 0b11111011; //Define RC2 "RF_CS" como saída. Declarado dessa forma para não gerar conflito com a biblioteca SPI.h

   RF_CE=0;

   //Para configurar os parametros deve-se:
   //1º-Resetar a saída RF_CS	
   //2º-Entrar com o endereço do parametro, enviando o dado através da comunicação SPI
   //3º-Enviar os novos valores para o parametro, através da comunicação SPI
   //4º-Setar novamente a saída RF_CS
   //OBS. Os endereços e valores padrões dos parametros estão contidos no datasheet da placa NRF24L01
   //OBS2. Para gravação deve-se adicionar 20(decimal) ao endereço do parametro desejado contido no datasheet. Para leitura usa-se o valor presente no manual.

   // TX_ADDR (0xFF)
   RF_CS=0;
   WriteSPI(0x30);
   WriteSPI(0xFF);
   WriteSPI(0xC2);
   WriteSPI(0xC2);
   WriteSPI(0xC2);
   WriteSPI(0xC2);
   RF_CS=1;

   // RX_ADDR_P0 (0xFF) ACK
   RF_CS=0;
   WriteSPI(0x2A);
   WriteSPI(0xFF);
   WriteSPI(0xC2);
   WriteSPI(0xC2);
   WriteSPI(0xC2);
   WriteSPI(0xC2);
   RF_CS=1;

   // RX_ADDR_P1 (dir)
   RF_CS=0;
   WriteSPI(0x2B);
   WriteSPI(dir);
   WriteSPI(0xC2);
   WriteSPI(0xC2);
   WriteSPI(0xC2);
   WriteSPI(0xC2);
   RF_CS=1;

   // RX_ADDR_P2 (0x00) BROADCAST
   RF_CS=0;
   WriteSPI(0x2C);
   WriteSPI(0x00);
   RF_CS=1;

   // EN_AA
   RF_CS=0;
   WriteSPI(0x21);
   WriteSPI(0x07);
   RF_CS=1;

   // EN_RXADDR
   RF_CS=0;
   WriteSPI(0x22);
   WriteSPI(0x07);
   RF_CS=1;

   // SETUP_AW
   RF_CS=0;
   WriteSPI(0x23);
   WriteSPI(0x03);
   RF_CS=1;

   //SETUP_RETR
   RF_CS=0;
   WriteSPI(0x24);
   WriteSPI(0x0A);
   RF_CS=1;

   //RF_CH
   RF_CS=0;
   WriteSPI(0x25);
   WriteSPI(canal);
   RF_CS=1;

   //RF_SETUP
   RF_CS=0;
   WriteSPI(0x26);
   WriteSPI(0x07);
   RF_CS=1;

   //STATUS
   RF_CS=0;
   WriteSPI(0x27);
   WriteSPI(0x70);
   RF_CS=1;

   //RX_PW_P0
   RF_CS=0;
   WriteSPI(0x31);
   WriteSPI(0x01);
   RF_CS=1;

   //RX_PW_P1
   RF_CS=0;
   WriteSPI(0x32);
   WriteSPI(0x0A);
   RF_CS=1;

   //RX_PW_P2
   RF_CS=0;
   WriteSPI(0x33);
   WriteSPI(0x0A);
   RF_CS=1;
}


//*****************************************************
//*                    RF_ON()                         *
//*****************************************************
//Função responsável por habilitar a transmissão da placa NRF24L01

void RF_ON()
{

   RF_CE=0;

   RF_CS=0;
   WriteSPI(0x20);
   WriteSPI(0x07);
   RF_CS=1;

   __delay_ms(2);
   RF_CE=1;
   __delay_us(150);
}


//*****************************************************
//*                 RF_OFF()                         *
//*****************************************************
//Função responsável por desabilitar a transmissão  da placa NRF24L01

void RF_OFF()
{
   RF_CE=0;

   // CONFIG - Desabilita o módulo
   RF_CS=0;
   WriteSPI(0x20);
   WriteSPI(0x0C);
   RF_CS=1;
}


//*****************************************************
//*                 RF_SEND()                         *
//*****************************************************
//Função responsável pelo envio dos dados do emissor para o receptor

int RF_SEND()
{
   int i;
   int estado;

   //Comparacoes para testar o bit 7 (interrupções habilitadas) da variavel INTCON sem causar conflitos entre bibliotecas
   char aux1;
   aux1=INTCON | 0b01111111;
   //------------------------------------------

   if(aux1==255)//if(bit_test(INTCON,7))
      interRF=1;
   else
      interRF=0;

   INTCON = INTCON & 0b00111111; //Desabilita todas as interrupções durante a transferencia de dados

   // INICIO
   RF_CE=0;

   //STATUS
   RF_CS=0;
   WriteSPI(0x27);
   WriteSPI(0x70);
   RF_CS=1;

   // EN_RXADDR
   RF_CS=0;
   WriteSPI(0x22);
   WriteSPI(0x01);
   RF_CS=1;

   // TX_ADDR
   RF_CS=0;
   WriteSPI(0x30);
   WriteSPI(RF_DIR);
   WriteSPI(0xC2);
   WriteSPI(0xC2);
   WriteSPI(0xC2);
   WriteSPI(0xC2);
   RF_CS=1;

   // RX_ADDR_P0
   RF_CS=0;
   WriteSPI(0x2A);
   WriteSPI(RF_DIR);
   WriteSPI(0xC2);
   WriteSPI(0xC2);
   WriteSPI(0xC2);
   WriteSPI(0xC2);
   RF_CS=1;

   // RX_ADDR_P1
   RF_CS=0;
   WriteSPI(0x0B);
   RF_DIR=ReadSPI();
   ReadSPI();
   ReadSPI();
   ReadSPI();
   ReadSPI();
   RF_CS=1;

//*********************************************
//Envio dos dados do transmissor
   // W_TX_PAYLOAD
   RF_CS=0;
   WriteSPI(0xA0);

   DATA_N_SND++;
   WriteSPI(DATA_N_SND);
   WriteSPI(RF_DIR);
   for (i=0;i<8;i++)
      WriteSPI(RF_DATA[i]);

   RF_CS=1;

   // CONFIG
   RF_CS=0;
   WriteSPI(0x20);
   WriteSPI(0x0E);
   RF_CS=1;

   // Pulso para iniciar transferencia
   RF_CE=1;
   __delay_us(15);
   RF_CE=0;

   noRF=0;

   while (RF_IRQ==1) {
      noRF++;
      if(noRF==1000){
        break;
      }
   }
//*********************************************


   // STATUS
   RF_CS=0;
   estado=ReadSPI();
   WriteSPI(0x70);
   RF_CS=1;

   // EN_RXADDR
   RF_CS=0;
   WriteSPI(0x22);
   WriteSPI(0x07);
   RF_CS=1;

   // TX_FLUSH
   RF_CS=0;
   WriteSPI(0xE1);
   RF_CS=1;

   // CONFIG
   RF_CS=0;
   WriteSPI(0x20);
   WriteSPI(0x07);
   RF_CS=1;

   // CONCLUSAO
   RF_CE=1;

   __delay_us(150);

   //Verifica se o dado foi enviado

   if(noRF==1000){//1000 para 8mhz - Deve-se usar 125 para cada 1mhz do clock
        if(interRF==1)
        INTCON = INTCON | 0b11000000; //Habilita interrupções
        INTCON3 = INTCON3 & 0b11111101; //Reseta flag da interrupção gerada por RB2
        return(2);
   }

   //Comparacoes para testar os bit 4 e 5 da variavel estado
   char aux2, aux3;
   aux2=estado & 0b00010000;
   aux3=estado | 0b11011111;
   //------------------------------------------

   if ((aux2==0) && (aux3==255)){ 
      if(interRF==1)
      INTCON = INTCON | 0b11000000; //Habilita interrupções
      INTCON3 = INTCON3 & 0b11111101; //Reseta flag da interrupção gerada por RB2
      return(0);
      }
   else{
      if(interRF==1)
      INTCON = INTCON | 0b11000000; //Habilita interrupções
      INTCON3 = INTCON3 & 0b11111101; //Reseta flag da interrupção gerada por RB2
      return(1);
      }
}


//*****************************************************
//*                 RF_RECEIVE()                      *
//*****************************************************
//Função responsável pela recepção dos dados

int RF_RECEIVE()
{

   int i;
   int mas;
   int estado;

   if (RF_IRQ==1 && RCVNW==0){
      return (2);
   }

   //STATUS
   RF_CS=0;
   estado=ReadSPI();
   WriteSPI(0x70);
   RF_CS=1;

   //Comparacoes para testar o bit 6 da variavel estado
   char aux1;
   aux1=estado & 0b01000000;
   //------------------------------------------

   //Testa bits da variavel estado
   if (aux1==0 && RCVNW==0){
      return(3);
   }

   //R_RX_PAYLOAD
   //Leitura dos dados recebidos
   RF_CS=0;
   WriteSPI(0x61);
   DATA_N_RCV=ReadSPI();
   RF_DIR=ReadSPI();
   for (i=0;i<8;i++)
   {
      RF_DATA[i]=ReadSPI();
   }
   RF_CS=1;

   //FIFO_STATUS
   RF_CS=0;
   WriteSPI(0x17);
   mas=ReadSPI();
   RF_CS=1;

   //Verifica se o dado foi recebido corretamente
   char aux2;
   aux2=mas & 0b00000001;

   if (aux2==0){
      RCVNW=1;
      return(1);
   }
      RCVNW=0;
      return(0);
}
//*****************************************************









