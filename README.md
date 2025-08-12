# horsemen
Self-replicating virus. This program is tested in controlled lab environments as part of a binary analysis course and serves educational purposes only.
## Configuring GDB
GDB uses AT&T syntax by default. If you prefer Intel syntax for debugging, follow these steps for GDB to use the latter by default:
```sh
echo "add-auto-load-safe-path <path-to-horsemen-directory>/.gdbinit" >> ~/.gdbinit
```
### Explanation
`~/.gdbinit` is read by GDB at every startup. This configuration allows it to trust the [`.gdbinit`](.gdbinit) file in your project directory, which sets the default disassembly flavor to the Intel syntax.
