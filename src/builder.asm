default rel ; makes it so that label references are relative not absolute
%include "./src/define.inc"

section .text
_start:
    ; open /proc/self/exe
    mov rax, SYS_OPEN
    lea rdi, [rel path_proc_self_exe]
    mov rsi, O_RDONLY
    syscall

    push rax ; rax == fd

    ; mmap /proc/self/exe for easy access
    // mmap for dest
    mov rax, SYS_MMAP
    mov rdi, 0x0
    mov rsi, 0x100000 ; 0x100_000 100 kb should be enough - change if more needed
    mov rdx, PROT_READ | PROT_WRITE | PROT_EXEC
    mov r10, 0x2
    pop r8
    mov r9, 0x0
    syscall

    push rax
    ; xor r13, r13 ; outer i

    // mmap for src
    mov rax, SYS_MMAP
    mov rdi, 0x0
    mov rsi, 0x100000
    mov rdx, 0x7
    mov r10, 0x2 | 0x20
    xor r8, r8
    mov r9, 0x0
    syscall

    mov r8, rax ; ADDR r8 = src
    pop r9 ; ADDR r9 = dest

    mov r10, r9 ; ADDR r10 == pointer for writing dst

    xor rcx, rcx ; counter of table entry


.loop_sections:
    ; loop condition
    mov r11, [rel scaffold_table_num] 
    cmp rcx, r11
    je loop_sections_end

    xor rdx, rdx ; counter of bytes in entry for copying

    ; Read table into regiseters
    mov r11, [rel scaffold_table_offset] 
    add r11, 8
    imul 
.loop_memcpy
    cmp rcx, r12
    je loop_sections_end
; loop_memcpy:
;     mov rax, r10
;     add rax, 8 ; set to start of table
;
;     mov rbx, rcx
;     imul rbx, 16
;     add rax, rbx ; rax += rcx * 16 ; rax is an ADDR
;
;     mov rsi, [rax] ; this is now the offset relative to the start of the scaf section
;     add rsi, r8 ; add mmap_file
;     add rsi, OFFSET_SCAFF ; add section_offset
;     ; rsi is now pointing to the start of the to be read thing
;
;     mov rbx, [rax + 8] ; this is not the offset of the end of the scaf block
;     add rbx, r8 ; add mmap_file
;     add rbx, OFFSET_SCAFF ; add section_offset
;     ; rbx is now the HALT for the copying
;
; loop_inner:
;     mov al, [rsi]
;     mov [rdi], al
;     inc rdi
;     inc rsi
;     cmp rsi, rbx
;     jne loop_inner
;
;
;     inc rcx
;     cmp rcx, [r10] ; if (rcx == *r10)
;     jne loop_memcpy
;
;     mov rax, _start
;     mov rbx, _end
;
jmp loop_memcpy
.loop_memcpy_end

jmp loop_section
.loop_sections_end:


jmp  r9; jump to build up to execute

path_proc_self_exe: u8 "/proc/self/exe", 0x0
old_e_entry: u64 0x0
scaffold_table_offset: u64 OFFSET_SCAFF; this is the offset in the file where the table is which has first a u64 with the num entries and then the entries with u64
scaffold_table_num: u64 0x1;
_end:

section .scaf align=16 progbits alloc # Important makes it so that ld actually creates the padding for scaffolding
