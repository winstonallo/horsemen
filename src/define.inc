
%define u8  db
%define u16 dw
%define u32 dd
%define u64 dq

%define OFFSET_SCAFF 0x1000

%define SYS_EXIT 60
%define SYS_MMAP 9 
%define SYS_OPEN 2

%define O_RDONLY  0x0     ; Open for reading only
%define O_WRONLY  0x1     ; Open for writing only
%define O_RDWR    0x2     ; Open for reading and writing
%define O_DIRECTORY 0x100000 

%define MAP_PRIVATE   0x2        ; Create a private copy-on-write mapping
%define MAP_ANONYMOUS 0x20       ; The mapping is not backed by any file; its contents are initialized to zero

%define PROT_READ     1   ; Pages may be read.
%define PROT_WRITE    2   ; Pages may be written.
%define PROT_EXEC     4   ; Pages may be executed.
%define PROT_NONE     0   ; Pages may not be accessed.
