#--------------------------------------Makefile-------------------------------------

CFILES = $(wildcard ./kernel/*.c)
OFILES = $(CFILES:./kernel/%.c=./build/%.o)
GCCFLAGS = -Wall -O2 -ffreestanding -nostdinc -nostdlib

all: clean uart1_build kernel8.img run1
uart1: clean uart1_build kernel8.img run1
uart0: clean uart0_build kernel8.img run0

#./build/uart.o: ./uart/uart1.c
#	aarch64-elf-gcc $(GCCFLAGS) -c ./uart/uart1.c -o ./build/uart.o

uart1_build: ./uart/uart1.c
	aarch64-elf-gcc $(GCCFLAGS) -c ./uart/uart1.c -o ./build/uart.o

uart0_build: ./uart/uart0.c
	aarch64-elf-gcc $(GCCFLAGS) -c ./uart/uart0.c -o ./build/uart.o

./build/boot.o: ./kernel/boot.S
	aarch64-elf-gcc $(GCCFLAGS) -c ./kernel/boot.S -o ./build/boot.o

./build/%.o: ./kernel/%.c
	aarch64-elf-gcc $(GCCFLAGS) -c $< -o $@

kernel8.img: ./build/boot.o ./build/uart.o $(OFILES)
	aarch64-elf-ld -nostdlib ./build/boot.o ./build/uart.o $(OFILES) -T ./kernel/link.ld -o ./build/kernel8.elf
	aarch64-elf-objcopy -O binary ./build/kernel8.elf build/kernel8.img

clean:
	rm -f .\build\kernel8.elf .\build\*.o *.img

# Run emulation with QEMU
run1: 
	qemu-system-aarch64 -M raspi3b -kernel kernel8.img -serial null -serial stdio

run0: 
	qemu-system-aarch64 -M raspi3b -kernel kernel8.img -serial stdio
