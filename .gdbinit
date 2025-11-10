# b scaffolding.c:65
# r
# define us
# c
# print cur_file
# print &dir_buf
# end
set disassembly-flavor intel

# b *0x4ef1d3

define one
c 
info registers
end

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
#
#
define irip
x/100i $rip
end
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
