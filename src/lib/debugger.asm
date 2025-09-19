in_debugger:
    ; ptrace(PTRACE_TRACEME, 0, 1, 0)
    mov rax, 101            ; SYS_PTRACE
    mov rdi, 0              ; PTRACE_TRACEME
    mov rsi, 0              ; pid
    mov rdx, 1              ; addr
    mov r10, 0              ; data
    syscall
    cmp rax, -1
    je .true
    mov rax, 0
    jmp .ret
    .true:
        mov rax, 1
    .ret:
        ret
