{
  inputs = {
    nixpkgs = {
      url = "github:NixOS/nixpkgs?ref=nixos-unstable";
    };
  };
  outputs =
    { self, nixpkgs, ... }@flakeInputs:
    let
      forAllSystems = nixpkgs.lib.genAttrs nixpkgs.lib.systems.flakeExposed;
    in
    {
      inherit nixpkgs;
      overlays = {
        default = import ./overlay.nix;
      };
      legacyPackages = forAllSystems (
        system: nixpkgs.legacyPackages.${system}.appendOverlays (builtins.attrValues self.overlays)
      );
      packages = forAllSystems (system: {
        inherit (self.legacyPackages.${system})
          imageApproximator
          ;
        default = self.legacyPackages.${system}.imageApproximator;
      });
      apps = forAllSystems (system: {
        imageApproximator = {
          type = "app";
          program = "${self.legacyPackages.${system}.imageApproximator}/bin/imageApproximator";
        };
        default = self.apps.${system}.imageApproximator;
      });
    };
}
