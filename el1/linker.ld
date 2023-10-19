ENTRY(_Reset)
SECTIONS
{
    . = 0x81000000; 
    _text = .;
    .text : { *(.text) }
    _etext = .;
    _data = .;
    .data : { *(.data) }
    _edata = .;
    .bss : { *(.bss COMMON) }
    .vbar_table : {*(.exception_table)}
    . = ALIGN(8);
    . = . + 0x2000; /* 2 page of stack memory */
    stack_top = .;
    pgd = .;
    . = . + 0x1000;
    page_allocator_start = .;
    . = . + 0x10000; /*allocate some memory for the page allocator if needed*/
    page_allocator_end  = .;


}