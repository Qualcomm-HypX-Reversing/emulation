ENTRY(_Reset)
SECTIONS
{
    . = 0x6942000000; /* funny */
    .text : { *(.text) }
    .data : { *(.data) }
    .bss : { *(.bss COMMON) }
    . = ALIGN(8);
    . = . + 0x2000; /* 2 page of stack memory */
    stack_top = .;
}