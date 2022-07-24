MEMORY
{
  FLASH (rx) : ORIGIN = 0x8000000, LENGTH = 1024K
  SRAM (xrw) : ORIGIN = 0x20000000, LENGTH = 128K
  AHBB0 (xrw) : ORIGIN = 0x10000000, LENGTH = 32K
  AHBB1 (xrw) : ORIGIN = 0x10008000, LENGTH = 32K
}

SECTIONS
{
    __STACKSIZE__ = 16384; 
    __HEAPSIZE__  = 16384; 

    .text :
    {
        _text = .;
        KEEP(*(.isr_vector))
        *(.text*)
        *(.rodata*)
        /* _etext = .; */
    } > FLASH

    ARM.extab : 
    { 
        *(.ARM.extab* .gnu.linkonce.armextab.*) 
    } >  FLASH 

    __exidx_start = .; 
    .ARM.exidx : 
    { 
        *(.ARM.exidx* .gnu.linkonce.armexidx.*) 
    } >  FLASH
    __exidx_end = .; 

    _etext = .; 
    _sidata = _etext;

    .data : 
    {
        _data = .;
        *(.data*)
        *(COMMON)
        _edata = .;
    } > SRAM AT > FLASH

    .bss (NOLOAD) :
    {
        _bss = .;
        *(.bss*)
        _ebss = .;
    } > SRAM

    .heap (NOLOAD) : 
    {
      _heap = .;
      _end = .;
      end = .;
      *(.heap*)
      . = ALIGN(MAX(_heap + __HEAPSIZE__ , .), 4);
    } > AHBB0
    _eheap = _heap + SIZEOF(.heap);

    .stack (NOLOAD) : 
    {
      _stack = .;
      *(.stack*)
      . = ALIGN(MAX(_stack + __STACKSIZE__ , .), 4);
    } > AHBB1
    _estack = _stack + SIZEOF(.stack);

    /* remove the debugging information from the standard libraries */
    DISCARD :
    {
     libc.a ( * )
     libm.a ( * )
     libgcc.a ( * )
     }
  
  
    /* Stabs debugging sections.  */
    .stab          0 : { *(.stab) }
    .stabstr       0 : { *(.stabstr) }
    .stab.excl     0 : { *(.stab.excl) }
    .stab.exclstr  0 : { *(.stab.exclstr) }
    .stab.index    0 : { *(.stab.index) }
    .stab.indexstr 0 : { *(.stab.indexstr) }
    .comment       0 : { *(.comment) }
    /* DWARF debug sections.
       Symbols in the DWARF debugging sections are relative to the beginning
       of the section so we begin them at 0.  */
    /* DWARF 1 */
    .debug          0 : { *(.debug) }
    .line           0 : { *(.line) }
    /* GNU DWARF 1 extensions */
    .debug_srcinfo  0 : { *(.debug_srcinfo) }
    .debug_sfnames  0 : { *(.debug_sfnames) }
    /* DWARF 1.1 and DWARF 2 */
    .debug_aranges  0 : { *(.debug_aranges) }
    .debug_pubnames 0 : { *(.debug_pubnames) }
    /* DWARF 2 */
    .debug_info     0 : { *(.debug_info .gnu.linkonce.wi.*) }
    .debug_abbrev   0 : { *(.debug_abbrev) }
    .debug_line     0 : { *(.debug_line) }
    .debug_frame    0 : { *(.debug_frame) }
    .debug_str      0 : { *(.debug_str) }
    .debug_loc      0 : { *(.debug_loc) }
    .debug_macinfo  0 : { *(.debug_macinfo) }
    /* SGI/MIPS DWARF 2 extensions */
    .debug_weaknames 0 : { *(.debug_weaknames) }
    .debug_funcnames 0 : { *(.debug_funcnames) }
    .debug_typenames 0 : { *(.debug_typenames) }
    .debug_varnames  0 : { *(.debug_varnames) }    
}
