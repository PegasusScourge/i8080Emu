# i8080 Emulator project
This is an emulator for the intel 8080 processor, written in C.

Command line switches:
 - ```-h``` displays the help message
 - ```-l <file> <memIndex>``` loads a ROM into memory starting at memIndex
 - ```-s <mhz>``` sets the clock speed of the emulator
 - ```-va <address>``` sets the start of VRAM for the emulator
 - ```-vd <width> <height>``` sets the dimensions of the output display
 - ```--test``` performs a self-test diagnostic and outputs the result in ```i8080_test.log```
 - ```--help``` alias for ```-h```
 - ```--load``` alias for ```-l```
 - ```--speed``` alias for ```-s```
 - ```--video:address``` alias for ```-va```
 - ```--video:dimensions``` alias for ```-vd```

### Sources
 - logging utility: https://github.com/rxi/log.c
 - opcode table: http://www.pastraiser.com/cpu/i8080/i8080_opcodes.html
 - another opcode table: http://www.emulator101.com/reference/8080-by-opcode.html
 - basis for implementation: http://emulator101.com/
 - example implementation: https://github.com/superzazu/8080
 - i8080 manual: http://www.nj7p.info/Manuals/PDFs/Intel/9800153B.pdf

### Notes
 - Little endian system, always check byte orders
