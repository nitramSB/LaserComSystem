/*
 * LaserCom_Transmitter.c
 *
 * Created: 04.03.2022 18:51:41
 * Author : Martin
 */ 

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>


#define F_CPU 1000000UL // Assumes 1MHz clock speed


#define NumCharPatterns 18

void InititializeGeneral();
void Initialize_HW_Interrupts();
void SetSevenSegmentCharacterPatternsInArray();
void InitialiseTimer1();
void USART0_SETUP_9600_BAUD_ASSUME_1MHz_CLOCK();
void USART0_TX_SingleByte(unsigned char);

unsigned char g_CharacterArray[NumCharPatterns]; // To hold display patterns for characters '0' - '9' and 'A' - 'F' and decimal point
unsigned char uDisplayDigitValue;
unsigned char uLED_value;

volatile unsigned char uDisplayPattern;
volatile unsigned char KeyValue;
unsigned char read;


int main(void)
{
    /* Replace with your application code */
	
	InititializeGeneral();
	Initialize_HW_Interrupts();
	//InitialiseTimer1();
	USART0_SETUP_9600_BAUD_ASSUME_1MHz_CLOCK();

	_delay_ms(1000);
	
	read = PIND & 0b00001000;
	
	if (read == 0)
	{
		PORTD = (read & 0b11101111) |  (0b00010000);

	while(1){
	USART0_TX_SingleByte(0b01010101);
	_delay_ms(1000);
	}
	
	}
	
	
    while (1) 
    {
		
	if (KeyValue < 18){
	uDisplayDigitValue = KeyValue;
	// Look up how to display the required character
	uDisplayPattern = g_CharacterArray[uDisplayDigitValue];
		
	PORTD = (PIND & ~0b11100000) | (uDisplayPattern & 0b11100000);
	PORTB = (PINB & ~0b00011111) | (uDisplayPattern & 0b00011111);
	
	}
	else
	{
		KeyValue = 0;
	}
	
	_delay_ms(200);
	
	}
	}
	



void InititializeGeneral(){
	
	sei();		// Enable interrupts at global level set Global Interrupt Enable (I) bit


	DDRD = 0b11110000;
	PORTD = 0b00001100; // Pull interrupts high
	
	DDRB = 0b00011111;
	
	KeyValue = 0;
	SetSevenSegmentCharacterPatternsInArray();
		
	uLED_value = 0;
	
	
}

void Initialize_HW_Interrupts() // Set up INT0 and INT1
{
	EICRA = 0b00001010;		// External Interrupt Control Register A: Interrupt Sense falling edge
	EIMSK = 0b00000011;		// External Interrupt Mask Register: Enable INT0 and INT1
	EIFR = 0b00000011;		// External Interrupt Flag Register: Clear INT0 interrupt flag (in case a spurious interrupt has occurred during chip startup)
}

ISR(INT0_vect){ 
	
	for(int i = 0; i < 100; i++)
	{
		for(int j = 0; j < 255; j++);
	}
	
	USART0_TX_SingleByte(uDisplayPattern);
	//USART0_TX_SingleByte(0b11010010);
	
	

}

ISR(INT1_vect){
	
	for(int i = 0; i < 100; i++)
	{
		for(int j = 0; j < 250; j++);
	}
		
	KeyValue++;
		
			
	

}

void SetSevenSegmentCharacterPatternsInArray()
{	

	g_CharacterArray[0] = 0b11101110;		// character code for '0'
	g_CharacterArray[1] = 0b00101000;		// character code for '1'
	g_CharacterArray[2] = 0b01110110;		// character code for '2'
	g_CharacterArray[3] = 0b11010110;		// character code for '3'
	g_CharacterArray[4] = 0b10011010;		// character code for '4'
	g_CharacterArray[5] = 0b11011100;		// character code for '5'
	g_CharacterArray[6] = 0b11111100;		// character code for '6'
	g_CharacterArray[7] = 0b10000110;		// character code for '7'
	g_CharacterArray[8] = 0b11111110;		// character code for '8'
	g_CharacterArray[9] = 0b10011110;		// character code for '9'
	g_CharacterArray[10] = 0b10111110;	// character code for 'A'
	g_CharacterArray[11] = 0b11111000;	// character code for 'B'
	g_CharacterArray[12] = 0b01101100;	// character code for 'C'
	g_CharacterArray[13] = 0b11110010;	// character code for 'D'
	g_CharacterArray[14] = 0b01111100;	// character code for 'E'
	g_CharacterArray[15] = 0b00111100;	// character code for 'F'
	g_CharacterArray[16] = 0b00000001;	// character code for '.'
	g_CharacterArray[17] = 0b00000000;	// character code for  blank
}




void InitialiseTimer1() // Configure to generate an interrupt after a 1-Second interval
{
	TCCR1A = 0b00000000; // Normal port operation (OC1A, OC1B, OC1C), Clear Timer on 'Compare Match' (CTC) waveform mode)
	TCCR1B = 0b00001101; // CTC waveform mode, use prescaler 1024
	TCCR1C = 0b00000000;
	// For 1 MHz clock (with 1024 prescaler) to achieve a 1 second interval:
	// Need to count 1 million clock cycles (but already divided by 1024)
	// So actually need to count to (1000000 / 1024 =) 976 decimal, = 3D0 Hex
	OCR1AH = 0x0B; // Output Compare Registers (16 bit) OCR1BH and OCR1BL
	OCR1AL = 0xB8;
	
	TCNT1H = 0b00000000; // Timer/Counter count/value registers (16 bit) TCNT1H and TCNT1L
	TCNT1L = 0b00000000;
	TIMSK1 = 0b00000010; // bit 1 OCIE1A Use 'Output Compare A Match' Interrupt, i.e. generate an interrupt when the timer reaches the set value (in the OCR1A register)
}


ISR(TIMER1_COMPA_vect) // TIMER1_CompareA_Handler (Interrupt Handler for Timer 1)
{ // Flip the value of the least significant bit of the 8-bit variable
	
	unsigned char tmp;
	tmp = PIND;
	
	if(0 == uLED_value)
	{
		PORTD = (tmp & 0b11101111) |  (0b00010000);
		uLED_value = 1;
		//PORTD = 0b00010000;
		
	} else if (1 == uLED_value)
	{
		PORTD = (tmp & 0b11101111);
		uLED_value = 0;
		//PORTD = 0b00000000;

	}
	
}


void USART0_SETUP_9600_BAUD_ASSUME_1MHz_CLOCK()
{
	//UCSR0A – USART Control and Status Register A
	// bit 7 RXC Receive Complete (flag)
	// bit 6 TXC Transmit Complete (flag)
	// bit 5 UDRE Data Register Empty (flag)
	// bit 4 FE Frame Error (flag) - programmatically clear this when writing to UCSRA
	// bit 3 DOR Data OverRun (flag)
	// bit 2 PE Parity Error
	// bit 1 UX2 Double the USART TX speed (but also depends on value loaded into the Baud Rate Registers)
	// bit 0 MPCM Multi-Processor Communication Mode
	UCSR0A = 0b00000000; 

	// UCSR0B - USART Control and Status Register B
	// bit 7 RXCIE Receive Complete Interrupt Enable
	// bit 6 TXCIE Transmit Complete Interrupt Enable
	// bit 5 UDRIE Data Register Empty Interrupt Enable
	// bit 4 RXEN Receiver Enable
	// bit 3 TXEN Transmitter Enable
	// bit 2 UCSZ2 Character Size (see also UCSZ1:0 in UCSRC)
	// 0 = 5,6,7 or 8-bit data
	// 1 = 9-bit data
	// bit 1 RXB8 RX Data bit 8 (only for 9-bit data)
	// bit 0 TXB8 TX Data bit 8 (only for 9-bit data)
	UCSR0B = 0b00001000;  // TX Enable, 8-bit data

	// UCSR0C - USART Control and Status Register C
	// *** This register shares the same I/O location as UBRRH ***
	// Bits 7:6 – UMSELn1:0 USART Mode Select (00 = Asynchronous)
	// bit 5:4 UPM1:0 Parity Mode
	// 00 Disabled
	// 10 Even parity
	// 11 Odd parity
	// bit 3 USBS Stop Bit Select
	// 0 = 1 stop bit
	// 1 = 2 stop bits
	// bit 2:1 UCSZ1:0 Character Size (see also UCSZ2 in UCSRB)
	// 00 = 5-bit data (UCSZ2 = 0)
	// 01 = 6-bit data (UCSZ2 = 0)
	// 10 = 7-bit data (UCSZ2 = 0)
	// 11 = 8-bit data (UCSZ2 = 0)
	// 11 = 9-bit data (UCSZ2 = 1)
	// bit 0 UCPOL Clock POLarity
	// 0 Rising XCK edge
	// 1 Falling XCK edge
	UCSR0C = 0b00000111;		// Asynchronous, No Parity, 1 stop, 8-bit data, Falling XCK edge

	// UBRR0 - USART Baud Rate Register (16-bit register, comprising UBRR0H and UBRR0L)
	
	UBRR0H = 0;
	UBRR0L = 31;
	
	//UBRR0H = 0b00000010; // 2
	//UBRR0L = 0b01110000; // 112
	
}



void USART0_TX_SingleByte(unsigned char cByte)
{
	while(!(UCSR0A & (1 << UDRE0)));	// Wait for Tx Buffer to become empty (check UDRE flag)
	UDR0 = cByte;	// Writing to the UDR transmit buffer causes the byte to be transmitted

}

