# b scaffolding.c:65
# r
# define us
# c
# print cur_file
# print &dir_buf
# end
set disassembly-flavor intel

define us
s
info registers
x/200xh $r9
end
define uc
c
info registers
x/200xh $r9
end
# b jmp_to_scaffold
# r
#
# break *$r9
#
define ui
si
x/1i $rip
end
#
# us
#
# watch $r15
# c
#
# info registers
