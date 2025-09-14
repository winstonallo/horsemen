SECTIONS {
  .scaffolding : {
    *(.scaf)
    scaf_start = .;
    *(.mysection)
    . = scaf_start + 0x10000; /* This value is just very big so that the whole code fits in it it doesn't matter for the execution*/
    FILL(0x42)  
  }
  .text : {
    *(.text)
  }
  .data : {
    *(.data)
  }
  .bss : {
    *(.bss)
  }
}

