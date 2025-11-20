# horsemen

An educational exploration of ELF binary structure, code-caves, and staged payload reconstruction techniques.

Horsemen is a research project focused on investigating the structural limitations of Linux ELF binaries, and exploring how staged execution can be used for program instrumentation, analysis tooling, and security research. 

This project **does not provide or endorse malicious usage**. Its goal is to understand:
* How ELF layout and memory constraints influence binary-level tooling
* How tiny stubs can bootstrap larger functionality

## Motivation

The segments in ELF binaries are page-aligned, therefore the executable space available for instrumentation without making changes to the file structure cannot exceed 4095 bytes. This project was created to study this problem and evaluate how a staged loader design can overcome these limitations in a controlled, educational context.

## Overview

> A code cave is a series of unused bytes in a process' memory. The code cave inside a process's memory is often a reference to a section that has capacity for injecting custom instructions.

Horsemen uses a two-stage approach designed to fit into very small code caves while still enabling complex behavior.

### Builder (Bootstrap Stub)
A minimal (~200 bytes) stub that fits comfortably within typical code caves.

Responsibilities:
1. Reads a fragment table embedded elsewhere in the binary
2. Allocates executable memory at runtime using system calls
3. Copies and reassembles scattered code fragments into contiguous memory
4. Transfers execution to the reconstructed payload

This shows how a tiny footprint can orchestrate larger functionality by treating the binary itself as distributed storage.

### Scaffolding (Stage 2: full logic)
The full program logic, reassembled in memory by [the Builder](#builder-bootstrap-stub).

In this POC, this is used to:
* Inspect ELF metadata
* Validate structural properties
* Demonstrate self-propagation by instrumenting the host binary

## Configuring GDB

In order to use our GDB configuration, run the following command while in the directoryin which you cloned the repository:

```sh
echo "add-auto-load-safe-path $(pwd)/.gdbinit" >> ~/.gdbinit
```

_`~/.gdbinit` is read by GDB at every startup. This configuration allows it to trust the [`.gdbinit`](.gdbinit) file in your project directory, which sets the default disassembly flavor to the Intel syntax._

