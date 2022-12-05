{ pkgs ? (import <nixpkgs> {}).pkgsCross.riscv64-embedded }:
  pkgs.mkShell {
    # nativeBuildInputs is usually what you want -- tools you need to run
    nativeBuildInputs = [ pkgs.buildPackages.verilog pkgs.buildPackages.verilator pkgs.buildPackages.gtkwave pkgs.buildPackages.python3 pkgs.buildPackages.python3Packages.parse ];
}
