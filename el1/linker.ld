ENTRY(_Reset)
SECTIONS
{
    . = 0x81000000; 
    .text : { *(.text) }
    .data : { *(.data) }
    .bss : { *(.bss COMMON) }
    .vbar_table : {*(.exception_table)}
    . = ALIGN(8);
    . = . + 0x2000; /* 2 page of stack memory */
    stack_top = .;
}