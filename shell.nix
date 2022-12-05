with (import <nixpkgs> {});
mkShell {
  nativeBuildInputs = [ 
    verilog 
    verilator 
    gtkwave
    python3 
    python3Packages.parse
    ];
}
