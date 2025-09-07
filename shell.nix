
{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
  name = "dev-shell";

  # Replace with dev tools you actually need:
  buildInputs = with pkgs; [
    git
    nasm
    gnumake
    gdb
    # tools for coding
    valgrind
  ];

  # Optional: inherited dependencies from your build package
  # inputsFrom = [ (import ./default.nix).build ];

  shellHook = ''
    echo "🛠 Entered development shell"
    export DEV_ENV=1
    sudo sysctl -w vm.mmap_min_addr=0
  '';
}
