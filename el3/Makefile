CROSS_PREFIX=aarch64-linux-gnu-

all: el3_firmware.elf

entry.o : src/entry.S
	$(CROSS_PREFIX)as -g -c $< -o $@

el3_firmware.elf : entry.o
	$(CROSS_PREFIX)ld -T linker.ld $^ -o $@

clean :
	rm -f el3_firmware.elf entry.o

