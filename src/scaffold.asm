
%include "./src/define.inc"

section .scaf align=16 progbits alloc
scaffold_table_num:
    u64 0x2
scaffold_table:
    u64 l1 - scaffold_table_num
    u64 l2 - scaffold_table_num

    u64 scaffold_start - scaffold_table_num
    u64 scaffold_end - scaffold_table_num

scaffold_start:
    syscall
scaffold_end:

l1:
    mov rax, SYS_EXIT
    mov rdi, 42
l2: