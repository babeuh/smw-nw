{
  description = "SMW";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
  };

  outputs = { self, nixpkgs }:
  let
    pkgs = nixpkgs.legacyPackages.x86_64-linux;
  in {
    devShell.x86_64-linux =
      pkgs.mkShell {
        nativeBuildInputs = with pkgs; [
          pkg-config
          gnumake
          python3
        ];
        buildInputs = with pkgs; [
          SDL2
        ];
      };
  };
}
