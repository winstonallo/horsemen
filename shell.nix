let
 pkgs = import (builtins.fetchTarball {
    url = "https://github.com/NixOS/nixpkgs/archive/6faeb062ee4cf4f105989d490831713cc5a43ee1.tar.gz";
  }) {};
in
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
    clamav
  ];

  shellHook = ''
    export PS1="\W> "

  '';
}
