#include <avr/io.h>
#include <util/delay.h>
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
#define TW_MASTER_SLAR_ACK 0x40
#define TW_MASTER_SLAR_NACK 0x48
#define TW_MASTER_RECEIVE_ACK 0x50
#define TW_MASTER_RECEIVE_NACK 0x58


#define TEMP_INIT_COMMAND		0xbe
#define TEMP_INIT_COMMAND_PARAM1 0x08
#define TEMP_INIT_COMMAND_PARAM2 0x00

#define TEMP_TRIGGER_READING	0xAC
#define TEMP_TRIGGER_READING_PARAM1 0x33
#define TEMP_TRIGGER_READING_PARAM2 0x00

char uart_rx(void);
void uart_print_twi_status();
void uart_printstr(const char *str);
void uart_printbyte(uint8_t byte);
void uart_tx(char c);
void uart_init();
void i2c_stop(void);
void i2c_start(void);
void i2c_write(unsigned char data);
char i2c_read(void);
void print_hex_value(unsigned char c);
void i2c_init(void);



const char hex_chars[] = "0123456789ABCDEF";

void i2c_init(void)
{
	//setting SCL frequency in the bitrate register
	TWBR = TWI_BITRATE;

	//setting no prescaler 
	TWSR = 0;
}

void i2c_start_write(void)
{
	//sending START on the TWI ( TWSTA for start, TWEN to enable TW, 
	//TWINT to clear theinterrupt flag and start operating)
	TWCR = 1 << TWINT | 1 << TWSTA | 1 << TWEN;

	while ((TWCR & (1 << TWINT)) == 0); //wait end of operation
	// uart_print_twi_status();

	//setting data register as temp sensor address and write mode
	TWDR = (TEMP_SLA << 1) | TW_WRITE;
	//sending SLA + W
	TWCR = 1 << TWINT | 1 << TWEN;

	while ((TWCR & (1 <<TWINT)) == 0); // wait for ACK
}

void i2c_start_read(void)
{

	//sending START on the TWI ( TWSTA for start, TWEN to enable TW, 
	//TWINT to clear theinterrupt flag and start operating)
	TWCR = 1 << TWINT | 1 << TWSTA | 1 << TWEN;

	while ((TWCR & (1 << TWINT)) == 0);

	TWDR = (TEMP_SLA << 1) | TW_READ;
	TWCR = 1 << TWINT | 1 << TWEN;

	while((TWCR & (1 << TWINT)) == 0);
}

void i2c_stop(void)
{
	//sending STOP on the TWI ( TWSTO for stop,
	// TWINT to clear interrupt flag and start operating)
	TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);

}

void i2c_write(unsigned char data)
{
	//moving data into data register
	TWDR = data;

	//sending data
	TWCR = 1 << TWINT | 1 << TWEN;

	while ((TWCR & (1 << TWINT)) == 0); // wait until it is done
}

char i2c_read(void)
{
	//setting TWI in receive mode
	TWCR = 1 << TWINT | 1 << TWEA | 1 << TWEN;

	//waiting to receive data
	while((TWCR & (1 << TWINT)) == 0);
	// uart_print_twi_status();

	return (TWDR);
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
	status &= TW_STATUS_MASK;

	switch (status)
	{
	case TW_MASTER_START: {
		uart_printstr("A start condition has been transmitted\r\n");
		break;
	}
	case TW_MASTER_START_REPEAT: {
		uart_printstr("A repeated start condition has been transmitted\r\n");
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
		uart_printstr("Arbitration lost\r\n");
		break;
	}
	case TW_MASTER_SLAR_ACK: {
		uart_printstr("SLA + R has been transmitted, ACK has been received\r\n");
		break;
	}
	case TW_MASTER_SLAR_NACK: {
		uart_printstr("SLA + R has been transmitted, NACK has been received\r\n");
		break;
	}
	case TW_MASTER_RECEIVE_ACK: {
		uart_printstr("Data byte has been received, ACK has been returned\r\n");
		break;
	}
	case TW_MASTER_RECEIVE_NACK: {
		uart_printstr("Data byte has been received, NACK has been returned\r\n");
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

void print_hex_value(unsigned char c)
{
	uint8_t to_print = c >> 4;
	uart_tx(hex_chars[to_print]);
	to_print = c & 0b1111;
	uart_tx(hex_chars[to_print]);
}

void temp_init()
{
	_delay_ms(40);

	// uart_printstr("going into write mode\r\n");
	//if write mode isnt on, go in write mode
	if ((TWSR & TW_STATUS_MASK) != TW_MASTER_TRANSMIT_ACK
	&& (TWSR & TW_STATUS_MASK) != TW_MASTER_SLAW_ACK)
		i2c_start_write();

	i2c_write(TEMP_INIT_COMMAND);
	i2c_write(TEMP_INIT_COMMAND_PARAM1);
	i2c_write(TEMP_INIT_COMMAND_PARAM2);
	i2c_stop();
	_delay_ms(10);
}

void read_temp()
{
	//if write mode isnt on go in write mode
	if ((TWSR & TW_STATUS_MASK) != TW_MASTER_TRANSMIT_ACK
	&& (TWSR & TW_STATUS_MASK) != TW_MASTER_SLAW_ACK)
	{
		// uart_print_twi_status();
		i2c_start_write();
	}

	//send create reading command		
	i2c_write(TEMP_TRIGGER_READING);
	i2c_write(TEMP_TRIGGER_READING_PARAM1);
	i2c_write(TEMP_TRIGGER_READING_PARAM2);
	i2c_stop();


	//then wait 80 ms as the datasheet says to do 
	_delay_ms(80);
	i2c_start_read();
	uint8_t status = i2c_read();

	while ((status >> 7) & 1)
	{
		uart_printstr("Busy, waiting again\r\n");
		_delay_ms(80);
		status = i2c_read();
	}
	print_hex_value(status);
	uart_tx(' ');
	for (uint8_t i = 0; i < 6; i++)
	{
		print_hex_value(i2c_read());
		uart_printstr(" ");
	}
	uart_printstr("\r\n");
}

int main()
{
	uart_init();
	i2c_init();

	i2c_start_write();
	temp_init();

	// for(uint8_t i = 0; i < 255; i++)
	// {
	// 	print_hex_value(i);
	// 	uart_tx(' ');
	// 	if ((i + 1) % 16 == 0)
	// 		uart_printstr("\r\n");
	// }

	while (1)
	{
		read_temp();
		_delay_ms(1000);
	}
	i2c_stop();
}
