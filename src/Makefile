# Add filename.o to include in compilation
OBJECTS=main.o system_stm32f4xx.o startup_stm32f411xe.o clock.o usart.o i2c.o spi.o tasks.o leds.o flash.o
OBJECTS+= ringbuffer.o usart.o

all: image.elf

%.o: %.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

%.o: %.s
	$(AS) -c $(ASFLAGS) $< -o $@

  
image.elf: $(OBJECTS)
	cd STPeriph && make
	$(CC) $(CFLAGS) $(CPPFLAGS) -T ../stm32f411re.ld -o image.elf *.o STPeriph/*.o 

clean:
	rm -f *.o
	cd STPeriph && make clean
	rm -f image.elf
