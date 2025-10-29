
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
    glibc.dev
    valgrind
  ];

  shellHook = ''
    export DEV_ENV=1

    sudo sysctl -w vm.mmap_min_addr=0
  '';
}
