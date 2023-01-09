{ pkgs ? import <nixpkgs> {} }:
pkgs.mkShell {
  nativeBuildInputs = with pkgs; [ 
    verilog 
    verilator 
    gtkwave
    python3 
    python3Packages.parse
    ];
}
