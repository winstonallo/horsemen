default rel ; makes it so that label references are relative not absolute
%include "./src/define.inc"

section .text
_start:
    pushfq
    push    rax
    push    rbx
    push    rcx
    push    rdx
    push    rsi
    push    rdi
    push    rbp
    push    r12
    push    r13
    push    r14
    push    r15
    push    r8
    push    r9
    push    r10
    push    r11
    mov r15, rsp

    ; open /proc/self/exe
    mov rax, SYS_OPEN
    lea rdi, [rel path_proc_self_exe]
    mov rsi, O_RDONLY
    syscall

    push rax ; rax == fd

    ; mmap /proc/self/exe for easy access
    ; mmap for src
    mov rax, SYS_MMAP
    mov rdi, 0x0
    mov rsi, 0x100000 ; 0x100_000 100 kb should be enough - change if more needed
    mov rdx, PROT_READ
    mov r10, 0x2
    pop r8
    mov r9, 0x0
    syscall

    push rax
    ; xor r13, r13 ; outer i

    ; mmap for dest
    mov rax, SYS_MMAP
    mov rdi, 0x6969690000
    mov rsi, 0x100000
    mov rdx, 0x7
    mov r10, 0x2 | 0x20 | 0x10
    xor r8, r8
    mov r9, 0x0
    syscall

    mov [rax], r15
    xor r15, r15
    add rax, 8;
    pop r8 ; ADDR r8 = src
    mov r9, rax ; ADDR r9 = dst

    mov r10, r9 ; ADDR r10 == pointer for writing dst

    xor rcx, rcx ; counter of table entry


.loop_sections:
    ; loop condition
    mov r11, [rel scaffold_table_num] 
    cmp rcx, r11
    je .loop_sections_end

    xor rdx, rdx ; counter of bytes in entry for copying

    ; Read table into regiseters
    mov rsi, 16 
    imul rsi, rcx
    add  rsi, [rel scaffold_table_offset] 
    add rsi, r8
    ; now its the memory adress in rsi
    mov r11, [rsi];
    add r11, r8
    ; r11 == start in mem
    add rsi, 8
    mov r12, [rsi];
    ; r12 == size

.loop_memcpy:
    cmp rdx, r12
    je .loop_memcpy_end

    mov rbx, r11
    add rbx, rdx
    mov al, [rbx]

    mov [r10], al
    inc rdx
    inc r10

jmp .loop_memcpy
.loop_memcpy_end:

inc rcx

jmp .loop_sections
.loop_sections_end:

jmp  r9; jump to build up to execute

    mov rax, 0x6969690000
    mov rsp, [rax]
    pop     r11
    pop     r10
    pop     r9
    pop     r8
    pop     r15
    pop     r14
    pop     r13
    pop     r12
    pop     rbp
    pop     rdi
    pop     rsi
    pop     rdx
    pop     rcx
    pop     rbx
    pop     rax
    popfq

    jmp    [rel old_e_entry]   ; directly jump to the qword at that memory

path_proc_self_exe: u8 "/proc/self/exe", 0x0
old_rsp: u64 0x0
old_e_entry: u64 0x0
scaffold_table_offset: u64 OFFSET_SCAFF; this is the offset in the file where the table is which has first a u64 with the num entries and then the entries with u64
scaffold_table_num: u64 0x2;
_end:

section .scaf align=16 progbits alloc ; Important makes it so that ld actually creates the padding for scaffolding
