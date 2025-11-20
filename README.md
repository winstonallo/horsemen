# horsemen Self-replicating virus.

This program is tested in controlled lab environments as part of a binary analysis course and serves educational purposes only.

## Configuring GDB

GDB uses AT&T syntax by default. If you prefer Intel syntax for debugging, follow these steps for GDB to use the latter:

```
sh echo "add-auto-load-safe-path /.gdbinit" >> ~/.gdbinit
```

### Explanation

`~/.gdbinit` is read by GDB at every startup. This configuration allows it to trust the [`.gdbinit`](.gdbinit) file in your project directory, which sets the default disassembly flavor to the Intel syntax.

#### fixes

`.bss` does not point to anything _in the binary_, it only specifies how big the loaded `.bss` section should be. We were using it as an actual section, which then had an influence on the code caves we found.
As of right now (14.11.2025), we just do not infect binaries if we reach the `.bss` when looking for the code caves. This works, we are just running into size issues now. The next step would be to find out how to make the injected code smaller.
