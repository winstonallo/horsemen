default rel ; makes it so that label references are relative not absolute
%define u8  db
%define u16 dw
%define u32 dd
%define u64 dq
%define O_RDONLY 0x0
%define OFFSET_SCAFF 0x1000

_start:
    ; push rax
    ; open /proc/self/exe
    mov rax, SYS_OPEN
    lea rdi, [rel path_proc_self_exe]
    mov rsi, O_RDONLY
    syscall

    push rax


    ; mmap /proc/self/exe for easy access
    mov rax, SYS_MMAP
    mov rdi, 0x0
    mov rsi, 0x100000 ; 0x100_000 100 kb should be enough change if more needed
    mov rdx, 0x7
    mov r10, 0x2
    pop r8
    mov r9, 0x0
    syscall

    push rax
    ; xor r13, r13 ; outer i

    mov rax, SYS_MMAP
    mov rdi, 0x0
    mov rsi, 0x100000
    mov rdx, 0x7
    mov r10, 0x2 | 0x20
    xor r8, r8
    mov r9, 0x0
    syscall

    pop r8 ; ADDR r8 = mmap_file 
    mov r9, rax ; ADDR r9 = mmap_exe 

    xor rsi, rsi ; ADDR rsi pointer for reading ins from mmap_file
    mov rdi, r9 ; ADDR rdi == pointer for writing ins to mmap_exe

    xor rcx, rcx ; counter of table entry
    xor rdx, rdx ; counter of bytes in entry for copying
    
    ; ADDR r10 = addr mmap_file - scaffold
    mov r10, r8
    add r10, [rel scaffold_table_offset] 



loop_memcpy:
    mov rax, r10
    add rax, 8 ; set to start of table

    mov rbx, rcx
    imul rbx, 16
    add rax, rbx ; rax += rcx * 16 ; rax is an ADDR

    mov rsi, [rax] ; this is now the offset relative to the start of the scaf section
    add rsi, r8 ; add mmap_file
    add rsi, OFFSET_SCAFF ; add section_offset
    ; rsi is now pointing to the start of the to be read thing

    mov rbx, [rax + 8] ; this is not the offset of the end of the scaf block
    add rbx, r8 ; add mmap_file
    add rbx, OFFSET_SCAFF ; add section_offset
    ; rbx is now the HALT for the copying

loop_inner:
    mov al, [rsi]
    mov [rdi], al
    inc rdi
    inc rsi
    cmp rsi, rbx
    jne loop_inner
    

    inc rcx
    cmp rcx, [r10] ; if (rcx == *r10)
    jne rel loop_memcpy

    jmp r9 ; jump to build up mmap_exe to execute
    

path_proc_self_exe:    db "/proc/self/exe", 0x0
scaffold_table_offset: u64 OFFSET_SCAFF; this is the offset in the file where the table is which has first a u64 with the num entries and then the entries with u64
builder_end:

%include "../syscalls.inc"


; section .scaffolding
section .scaf align=16 progbits alloc
scaffold_table_num:
    u64 0x1
scaffold_table:
    u64 scaffold_start - scaffold_table_num
    u64 test2 - scaffold_table_num

scaffold_start:
    mov rax, SYS_EXIT
    mov rdi, 42
    syscall
test2: