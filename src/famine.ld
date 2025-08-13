SECTIONS {
  .scaffolding : {
    *(.scaf)
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