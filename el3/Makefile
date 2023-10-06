CROSS_PREFIX := aarch64-linux-gnu-
INCLUDE := include 
ASMFLAGS := -I $(INCLUDE) -g
CFLAGS := -I $(INCLUDE) -g -ffreestanding



all: 
	mkdir -p bin
	make el3_firmware.elf

bin/entry.o : src/entry.S $(INCLUDE)/assembly-constants.h
	$(CROSS_PREFIX)as $(ASMFLAGS) -c $< -o $@

bin/printf.o : src/printf.c
	$(CROSS_PREFIX)gcc $(CFLAGS) -c $< -o $@

bin/main.o : src/main.c
	$(CROSS_PREFIX)gcc $(CFLAGS) -c $< -o $@

bin/page_tables.o : src/page_tables.c
	$(CROSS_PREFIX)gcc $(CFLAGS) -c $< -o $@

$(INCLUDE)/assembly-constants.h :
	gcc -I $(INCLUDE) -o bin/gen_asm_constants tools/gen_asm_constants.c 
	bin/gen_asm_constants

bin/excp.o : src/excp.S 
	$(CROSS_PREFIX)as $(ASMFLAGS) -c $< -o $@

el3_firmware.elf : bin/entry.o bin/printf.o bin/main.o bin/excp.o bin/page_tables.o
	$(CROSS_PREFIX)ld -T linker.ld $^ -o $@

clean :
	rm -rf el3_firmware.elf bin include/assembly-constants.h

