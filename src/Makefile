OBYCP = arm-none-eabi-objcopy
CC = arm-none-eabi-gcc
CFLAGS = --std=gnu11 -Ofast -mcpu=cortex-m3 -mthumb -fno-builtin  --specs=nosys.specs --specs=nano.specs -ffreestanding -Tgcc.ld -Wl,--entry=main
INCLD = -I ./hardware/sam3x8eHardwareAPI/3rdParty/ATMEL/ -I ./hardware/sam3x8eHardwareAPI/3rdParty/CMSIS/Include
TARGET ?= sam3x8e # target board

SRC_CC = *.c processManager/*.c flashManager/*.c memoryManager/*.c util/*.c kernelMonitor/*.c coreFunctions/*.c startup_ARMCM3.S hardware/api.o
SRC_O = *.o

kernel.bin : kernel.elf
	$(OBYCP) -O binary kernel.elf kernel.bin

kernel.elf : $(SRC_CC)
	$(CC) $(CFLAGS) -o kernel.elf $^ $(INCLD)

.PHONY: hardware/api.o
hardware/api.o : hardware/Makefile
	(TARGET=$(TARGET); export TARGET ; make -C hardware ;)

clean:
	rm kernel.elf kernel.bin

reset:
	stty -F /dev/ttyACM0 1200
	sleep 1
	stty -F /dev/ttyACM0 hupcl
	sleep 1

upload : kernel.bin reset
	bossac -i --port=ttyACM0 -e -w -b kernel.bin
	stty -F /dev/ttyACM0 9600 # restar board
	#cat /dev/cu.usbmodem14111

#Note to self use "-Wl,--just-symbols=<.elf file>" to link in super yolo mode to the kernel
