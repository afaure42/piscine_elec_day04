DEVICE=atmega328p
F_CPU = 16000000UL
BAUDRATE=115200
PROGRAMMER= -c arduino -P /dev/ttyUSB0 -b$(BAUDRATE) -p $(DEVICE)

SRCS = main.c

BINS = $(SRCS:.c=.bin)
HEXES = $(BINS:.bin=.hex)

CC = avr-gcc


CFLAGS = -Wall -D F_CPU=$(F_CPU) -D UART_BAUDRATE=$(BAUDRATE) -mmcu=$(DEVICE) -O3

all: flash



$(BINS): $(SRCS)
	$(CC) $(CFLAGS) $< -o $@

$(HEXES): $(BINS)
	avr-objcopy -O ihex $< $@



hex: $(HEXES)

flash: hex
	avrdude $(PROGRAMMER) -U flash:w:$(HEXES):i

clean:
	rm -rf $(BINS) $(HEXES)
	