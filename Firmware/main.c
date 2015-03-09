/*
    10-1-09
	Copyright Spark Fun Electronics© 2009
    Aaron Weiss
	aaron@sparkfun.com
	
	PCM1803A 24bit ADC
    
	*ATMega168 running @ 20MHz (High fuse = 0xDF, Low fuse = 0xE6)
	*Analog inputs are 3VPP tollerant
	*Using ATMega SPI interface in slave mode
	*This code only works on one channel, i.e. when LRCK = 0
	*Must have analog inputs connected if using one channel
	*ADC hardware configuration
	 Master Mode (512f and 384f only work)
	 Mode0:1 
	 Mode1:0
	
	 Left-justified 24-bit
	 Format0:0
	 Format1:0
	
	 PWDN:1
	 BPAs:1
*/

#include <stdlib.h>
#include <stdio.h>
#include <avr/io.h>

#define FOSC 20000000
#define BAUD 9600
#define MYUBRR 129 

#define sbi(var, mask)   ((var) |= (uint8_t)(1 << mask))
#define cbi(var, mask)   ((var) &= (uint8_t)~(1 << mask))

#define STATUS_LED 5

///===========Main Prototypes=============================///////////////////////
long read_ADC(void); //grab the 24 bits and concatenate each byte
long SPI_SlaveReceive(void); //wait for a 8bit transmission to complete and read SPDR register
void wait_to_start(void); // make sure we take the correct bits

///============Initialize Prototypes=====================///////////////////////
void ioinit(void);   
static int uart_putchar(char c, FILE *stream);
uint8_t uart_getchar(void);
static FILE mystdout = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);
void delay(uint16_t x);

/////===========Global Vars=============================////////////////////////
long out, csb, lsb, msb, dumby;

////////////////////////////////////////////////////////////////////////////////
//=========MAIN================/////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
int main(void)
{	
	ioinit(); //Setup IO pins and defaults
	
	while(1)
	{	
		wait_to_start();
		read_ADC();
		
		printf("%lu \r\n",read_ADC());
	}
}

long read_ADC(void)
{
	dumby = SPI_SlaveReceive(); //start clocking back bits after the last transmission 
	msb = SPI_SlaveReceive(); //load msb first 
	csb = SPI_SlaveReceive(); //load second 8 bit segment
	lsb = SPI_SlaveReceive(); //load lsb last
	
	return out = lsb|(csb << 8)|(msb << 16); //concatenate each 8 bit segment
}

long SPI_SlaveReceive(void)
{
	while(!(SPSR & (1<<SPIF)));
	return SPDR;
}

void wait_to_start(void)
{
	while(!(PINB & (1 << PINB2)));//wait while LRCK is low
}

////////////////////////////////////////////////////////////////////////////////
///==============Initializations=======================================/////////
////////////////////////////////////////////////////////////////////////////////
void ioinit (void)
{
    //1 = output, 0 = input
    DDRB = 0b00010000; //PB4 = MISO 
    DDRC = 0b00110001; //Output on PORTC0, PORTC4 (SDA), PORTC5 (SCL), all others are inputs
    DDRD = 0b11110010; //PORTD (RX on PD0), input on PD2
	
    //USART Baud rate: 9600
    UBRR0H = MYUBRR >> 8;
    UBRR0L = MYUBRR;
    UCSR0B = (1<<RXEN0)|(1<<TXEN0);    
	
	stdout = &mystdout; //Required for printf init
	
	//initialize SPI slave
	SPCR = (1<<SPE);
}

static int uart_putchar(char c, FILE *stream)
{
    if (c == '\n') uart_putchar('\r', stream);
  
    loop_until_bit_is_set(UCSR0A, UDRE0);
    UDR0 = c;
    
    return 0;
}

uint8_t uart_getchar(void)
{
    while( !(UCSR0A & (1<<RXC0)) );
    return(UDR0);
}

void delay(uint16_t x)
{
  uint8_t y;
  for ( ; x > 0 ; x--){
    for ( y = 0 ; y < 50 ; y++){ 
    asm volatile ("nop");
    }
  }
}