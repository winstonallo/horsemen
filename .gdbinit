set disassembly-flavor intel

b jmp_to_scaffold
r

break *$r9

define us
si
x/1i $rip
end

us