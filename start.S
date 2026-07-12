.section .text
.global _start
_start:
    li sp, 0x00010000  # Set stack pointer to 64KB boundary
    call main          # Jump to your C code
loop:
    j loop             # Infinite loop if main() accidentally exits