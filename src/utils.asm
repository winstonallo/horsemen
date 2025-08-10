; void *get_base_address()
; Gets the runtime base address by parsing /proc/self/maps.
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
    .read_proc_map:
        mov rax, SYS_READ
        syscall

        cmp eax, 1
        jl exit

        cmp BYTE [rsp], '-'
        je .break
        inc r10b
        mov r8b, BYTE [rsp]

        cmp r8b, '9'
        jle .num
    ; read_proc_map
    .alpha:
        sub r8b, 'a' - 10
        jmp .load
    ; alpha
    .num:
        sub r8b, '0'
    ; num
    .load:
        shl rbx, 4
        or rbx, r8
        add rsp, 1
        lea rsi, [rsp]
        jmp .read_proc_map
    ; load
    .break:
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
