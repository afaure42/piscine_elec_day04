#include <avr/io.h>
#include <util/twi.h>

#define UART_BAUD_SETTING (((F_CPU / 8 / UART_BAUDRATE ) -1 ) / 2)


#define SCL_FREQUENCY 100000
#define TWI_BITRATE (((F_CPU / SCL_FREQUENCY) - 16) / 2)
#define TEMP_SLA 0x38
#define TW_WRITE 0
#define TW_READ 1
#define TW_ACK 0
#define TW_NACK 1
#define TW_MASTER_START 0x08
#define TW_MASTER_START_REPEAT 0x10
#define TW_MASTER_SLAW_ACK 0x18
#define TW_MASTER_SLAW_NACK 0x20
#define TW_MASTER_TRANSMIT_ACK 0x28
#define TW_MASTER_TRANSMIT_NACK 0x30
#define TW_MASTER_ARBITRATION_LOST 0x38


char uart_rx(void);
void uart_print_twi_status();
void uart_printstr(const char *str);
void uart_printbyte(uint8_t byte);
void uart_tx(char c);
void uart_init();
void i2c_stop(void);
void i2c_start(void);
void i2c_init(void);

void i2c_init(void)
{
	//setting SCL frequency in the bitrate register
	TWBR = TWI_BITRATE;

	//setting no prescaler 
	TWSR = 0;
}

void i2c_start(void)
{
	//sending START on the TWI ( TWSTA for start, TWEN to enable TW, 
	//TWINT to clear theinterrupt flag and start operating)
	TWCR = 1 << TWINT | 1 << TWSTA | 1 << TWEN;

	while ((TWCR & (1 << TWINT)) == 0); //wait end of operation
	uart_print_twi_status();

	//setting data register as temp sensor address and write mode
	TWDR = (TEMP_SLA << 1) + TW_WRITE;
	//sending SLA + W
	TWCR = 1 << TWINT | 1 << TWEN;

	while ((TWCR & (1 <<TWINT)) == 0); // wait for ACK
	uart_print_twi_status();
}

void i2c_stop(void)
{
	//sending STOP on the TWI ( TWSTO for stop, TWEN to enable TW,
	// TWINT to clear interrupt flag and start operating)
	TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);

	uart_print_twi_status();
	uart_printstr("exiting i2c_stop\r\n");
}

void uart_init()
{
	//set baudrate
	UBRR0H = UART_BAUD_SETTING >> 8;
	UBRR0L = UART_BAUD_SETTING;

	//enable transmitter and receiver
	UCSR0B = 1 << TXEN0 | 1 << RXEN0;

	//set frame format (8data, no parity, 1 stop)
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);

}

void uart_tx(char c)
{
	//writing data to send register
	while(((UCSR0A >> UDRE0) & 1) == 0);
	UDR0 = c;
}

void uart_printbyte(uint8_t byte)
{
	for(uint8_t i = 8; i > 0; i--)
		uart_tx(((byte >> (i - 1)) & 1) + '0');
}

void uart_printstr(const char *str)
{
	while(*str)
	{
		uart_tx(*str);
		str++;
	}
}

void uart_print_twi_status()
{
 	//extracting status code from TW status register
	uint8_t status = TWSR;
	status &= ~(1 << TWPS1 | 1 << TWPS0 | 1 << 2);

	switch (status)
	{
	case TW_MASTER_START: {
		uart_printstr("A start condition has been transmitted\r\n");
		break;
	}
	case TW_MASTER_START_REPEAT: {
		uart_printstr("A repreatede start condition has been transmitted\r\n");
		break;
	}
	case TW_MASTER_SLAW_ACK: {
		uart_printstr("SLA + W has been transmitted, ACK has been received\r\n");
		break;
	}
	case TW_MASTER_SLAW_NACK: {
		uart_printstr("SLA + W has been transmitted, NACK has been received\r\n");
		break;
	}
	case TW_MASTER_TRANSMIT_ACK: {
		uart_printstr("Data byte has been transmitted, ACK has been received\r\n");
		break;
	}
	case TW_MASTER_TRANSMIT_NACK: {
		uart_printstr("Data byte has been transmitted, NACK has been received\r\n");
		break;
	}
	case TW_MASTER_ARBITRATION_LOST: {
		uart_printstr("Arbitration lost in SLA + W or data bytes, a START\r\n");
		break;
	}
	default: {
		uart_printstr("Unkown status, status code is:");
		uart_printbyte(status);
		uart_printstr("\r\n");
		break;
	}
	}
}

char uart_rx(void)
{
	while (((UCSR0A >> RXC0) & 1) == 0);

	char ret = UDR0;
	return (ret);
}

int main()
{
	uart_init();
	i2c_init();


	i2c_start();
	i2c_stop();
}
