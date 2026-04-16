# EECE372 Bare-Metal Programming Examples

Minimal bare-metal examples for the TI MSPM0C1104 (ARM Cortex-M0+), written entirely with direct register access and without DriverLib or SysConfig.

Written by Hanseo Ryu  
Contact: hsryu@postech.ac.kr

Some code in this repository was generated with Claude (Anthropic, 2026). Any such sections are explicitly marked in the source.

## Build

```sh
# Compile
arm-none-eabi-gcc -mcpu=cortex-m0plus -mthumb -O0 -ffreestanding -fno-builtin \
    -c startup_mspm0c1104.c -o startup_mspm0c1104.o

arm-none-eabi-gcc -mcpu=cortex-m0plus -mthumb -O0 -ffreestanding -fno-builtin \
    -c main.c -o main.o

# Link
arm-none-eabi-gcc -mcpu=cortex-m0plus -mthumb -O0 -ffreestanding -fno-builtin \
    -T mspm0c1104.ld -nostdlib \
    startup_mspm0c1104.o main.o -o dma.elf

# Convert to a raw binary
arm-none-eabi-objcopy -O binary dma.elf dma.bin
```

Build flag summary:

- `-mcpu=cortex-m0plus`: Target the Arm Cortex-M0+ CPU.
- `-mthumb`: Generate Thumb instruction set code.
- `-O0`: Disable optimization for easier debugging.
- `-ffreestanding`: Build for a bare-metal environment without a hosted runtime.
- `-fno-builtin`: Do not replace standard functions with compiler built-ins.
- `-c`: Compile only and produce an object file.
- `-T mspm0c1104.ld`: Use the specified linker script.
- `-nostdlib`: Do not link the standard C library or default startup files.
- `-O binary`: Write the output as a raw binary image.

## Debug

Disassemble the ELF file to inspect the generated machine code and assembly.

```sh
arm-none-eabi-objdump -d {name}.elf
```

## Flash

Use OpenOCD to program the ELF file:

```sh
openocd -f mspm0c1104_xds110.cfg 
```

Open another shell.

```sh
nc localhost 4444
init
reset halt
flash write_image erase dma.bin 0x00000000
verify_image dma.bin 0x00000000
reset run
```

## Repository Structure

### `mspm0c1104.ld`

The linker script defines the MSPM0C1104 memory layout: 16 KB of flash at `0x00000000` and 1 KB of SRAM at `0x20000000`. It also places the `.isr_vector`, `.text`, `.data`, and `.bss` sections.

### `startup_mspm0c1104.c`

The startup file defines the interrupt vector table and `Reset_Handler`. On reset, it copies `.data` from flash to SRAM, clears `.bss`, and then calls `main()`.

Because each example may require different interrupt handlers, check the vector table in the example you are building.

### Document References

- `TRM` in the code refers to:
  Texas Instruments, *MSPM0 C-Series Microcontrollers Technical Reference Manual* (2025)  
  https://www.ti.com/kr/lit/pdf/slau893
- `Datasheet` in the code refers to:
  Texas Instruments, *MSPM0C110x, MSPS003 Mixed-Signal Microcontrollers* (2026)  
  https://www.ti.com/kr/lit/gpn/mspm0c1104

### Additional Resources

- MSPM0 Development Guide  
  https://www.ti.com/kr/lit/pdf/slaaed1
- Arm Cortex-M0+ User Guide  
  https://developer.arm.com/documentation/dui0662/latest/


## Reference

Anthropic. (2026). Claude (claude-sonnet-4-6) [Large language model]. https://claude.ai/