
{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
  name = "dev-shell";

  # Replace with dev tools you actually need:
  buildInputs = with pkgs; [
    git
    nasm
    gnumake
    gdb
    xxd
    python3
    clang-tools   # already includes clang + clangd + correct wrappers
    glibc.dev

    # tools for coding
    valgrind
  ];

  shellHook = ''
    export DEV_ENV=1

#    sudo sysctl -w vm.mmap_min_addr=0
  '';
}
