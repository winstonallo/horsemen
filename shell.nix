
{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
  name = "dev-shell";

  buildInputs = with pkgs; [
    git
    nasm
    gnumake
    gdb
    xxd
    python3
    clang-tools
    valgrind
    glibc
    llvmPackages_21.clang
    llvmPackages_21.clang-tools
    gcc
  ];

  shellHook = ''
    export PS1="\[\e[0;32m\]\W>\[\e[0m\] "
  '';
}
