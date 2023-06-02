CROSS_PREFIX := aarch64-linux-gnu-
ASMFLAGS := -g
INCLUDE := include 
CFLAGS := -I $(INCLUDE) -g -ffreestanding



all: 
	mkdir -p bin
	make el3_firmware.elf

bin/entry.o : src/entry.S
	$(CROSS_PREFIX)as $(ASMFLAGS) -c $< -o $@

bin/printf.o : src/printf.c
	$(CROSS_PREFIX)gcc $(CFLAGS) -c $< -o $@

bin/main.o : src/main.c
	$(CROSS_PREFIX)gcc $(CFLAGS) -c $< -o $@

el3_firmware.elf : bin/entry.o bin/printf.o bin/main.o
	$(CROSS_PREFIX)ld -T linker.ld $^ -o $@

clean :
	rm -rf el3_firmware.elf bin

