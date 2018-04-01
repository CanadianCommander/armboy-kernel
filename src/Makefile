OBYCP = arm-none-eabi-objcopy
CC = arm-none-eabi-gcc
CFLAGS = --std=gnu11 -O0 -mcpu=cortex-m3 -mthumb -fno-builtin  --specs=nosys.specs -ffreestanding -Tgcc.ld -Wl,--entry=main
INCLD = -I ../../../tools/ATMEL/ -I ../../../tools/CMSIS/include

SRC_CC =  main.c UART/uart.c Timers/timers.c startup_ARMCM3.S

kernel.bin : kernel.elf
	$(OBYCP) -O binary kernel.elf kernel.bin

kernel.elf : $(SRC_CC)
	$(CC) $(CFLAGS) -o kernel.elf $^ $(INCLD)

clean:
	rm kernel.elf kernel.bin

reset:
	stty -f /dev/cu.usbmodem14111 1200
	sleep 1
	stty -f /dev/cu.usbmodem14111 hupcl
	sleep 1

upload : kernel.bin reset
	bossac -i --port=cu.usbmodem14111 -U=false -e -w -b kernel.bin
	stty -f /dev/cu.usbmodem14111 9600 # restar board
	#cat /dev/cu.usbmodem14111
