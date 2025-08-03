global _start
; syscalls
SYS_READ:       equ 0
SYS_WRITE:      equ 1
SYS_OPEN:       equ 2
SYS_CLOSE:      equ 3
SYS_MPROTECT:   equ 10
SYS_EXIT:       equ 60

; open flags
O_RDONLY:   equ 0
O_WRONLY:   equ 1
O_RDWR:     equ 2

; mprotect flags
PROT_READ:  equ 0x01
PROT_WRITE: equ 0x02
PROT_EXEC:  equ 0x04

; ELF
; e_ident
EI_NIDENT:      equ 16
EI_MAG0:        equ 0
ELFMAG0:        equ 0x7f
EI_MAG1:        equ 1
ELFMAG1:        equ 'E'
EI_MAG2:        equ 2
ELFMAG2:        equ 'L'
EI_MAG3:        equ 3
ELFMAG3:        equ 'F'
EI_CLASS:       equ 4
ELFCLASS64:     equ 2
EI_DATA:        equ 5
ELFDATANONE:    equ 0
EI_VERSION:     equ 6
EV_CURRENT:     equ 1
EI_PAD:         equ 9

; e_type
ET_NONE:    equ 0
ET_REL:     equ 1
ET_EXEC:    equ 2
ET_DYN:     equ 3
ET_CORE:    equ 4
ET_NUM:     equ 5

struc Elf64_Ehdr
    .e_ident        resb EI_NIDENT
    .e_type         resw 1
    .e_machine      resw 1
    .e_version      resd 1
    .e_entry        resq 1
    .e_phoff        resq 1
    .e_shoff        resq 1
    .e_flags        resd 1
    .e_ehsize       resw 1
    .e_phentsize    resw 1
    .e_phnum        resw 1
    .e_shentsize    resw 1
    .e_shnum        resw 1
    .e_shstrndx     resw 1
endstruc

struc Elf64_Shdr
    .sh_name        resd 1
    .sh_type        resd 1
    .sh_flags       resq 1
    .sh_addr        resq 1
    .sh_offset      resq 1
    .sh_size        resq 1
    .sh_link        resw 1
    .sh_info        resw 1
    .sh_addralign   resq 1
    .sh_entsize     resq 1
endstruc

_start:
    call get_base_address

    mov rax, SYS_OPEN
    mov rsi, O_RDONLY
    lea rdi, [rel file_path]
    syscall
    cmp eax, 0
    jl error

    mov r8, rax

    sub rsp, 16

    mov rax, SYS_READ
    mov rdi, r8
    lea rsi, [rsp]
    mov rdx, 16
    syscall
    cmp eax, 0
    jl error

    lea rdi, [rsp]
    call elf64_ident_check

    add rsp, 16

    mov rax, SYS_CLOSE
    mov rdi, r8
    syscall

    mov rax, SYS_EXIT
    mov rdi, 0
    syscall

; elf64_ident_check
elf64_ident_check:
    mov eax, DWORD [rdi]
    cmp eax, (ELFMAG3 << 24) | (ELFMAG2 << 16) | (ELFMAG1 << 8) | (ELFMAG0)
    jne not_elf

    cmp BYTE [rdi + EI_CLASS], ELFCLASS64
    jne not_64_bit
    cmp BYTE [rdi + EI_DATA], ELFDATANONE
    je not_elf
    cmp BYTE [rdi + EI_VERSION], EV_CURRENT
    jne not_elf

    mov rcx, 7
    lea rsi, [rdi + EI_PAD]
check_padding:
    cmp BYTE [rsi], 0
    jne not_elf
    inc rsi
    loop check_padding

    ret
; elf64_ident_check

; get_base_address
get_base_address:
    mov rsi, O_RDONLY
    lea rdi, [rel proc_self_maps]
    mov rax, SYS_OPEN
    syscall
    cmp eax, 0
    jl error

    push rax

    sub sp, 16

    xor r10, r10
    xor r8, r8
    xor rdi, rdi
    xor rdx, rdx
    xor rbx, rbx

    mov rdx, 1
    lea rsi, [rsp]
    mov edi, eax
read_proc_map:
    mov rax, SYS_READ
    syscall

    cmp eax, 1
    jl error

    cmp BYTE [rsp], '-'
    je break
    inc r10b
    mov r8b, BYTE [rsp]

    cmp r8b, '9'
    jle num
alpha:
    sub r8b, 'a' - 10
    jmp load
num:
    sub r8b, '0'
load:
    shl rbx, 4
    or rbx, r8
    add rsp, 1
    lea rsi, [rsp]
    jmp read_proc_map
break:
    sub sp, r10w
    add sp, 16

    pop rdi
    mov rax, SYS_CLOSE
    syscall
    cmp eax, 0
    jl error

    ret
; get_base_address

; error
error:
    mov rax, SYS_EXIT
    mov rdi, 1
    syscall
; error

; success
success:
    mov rax, SYS_EXIT
    mov rdi, 0
    syscall
; success

; not_elf
not_elf:
    add rsp, 16
    mov rax, SYS_WRITE
    mov rdi, 1
    lea rsi, [rel not_elf_msg]
    mov rdx, 11
    syscall

    jmp success
; not_elf

; not_64_bit
not_64_bit:
    add rsp, 16
    mov rax, SYS_WRITE
    mov rdi, 1
    lea rsi, [rel not_64_bit_msg]
    mov rdx, 17
    syscall

    jmp success
; not_64_bit

proc_self_exe:
    db "/proc/self/exe", 0
proc_self_maps:
    db "/proc/self/maps", 0
not_64_bit_msg:
    db "Not a 64 bit ELF", 10
not_elf_msg:
    db "Not an ELF", 10
file_path:
    db "Famine", 0
