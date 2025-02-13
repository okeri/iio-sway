{
  description = "A Meson-based project built with Nix";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/release-24.11";
    flake-utils.url = "github:numtide/flake-utils";
    home-manager.url = "github:nix-community/home-manager";
  };

  outputs = inputs @ {self, ...}:
    with inputs; let
      inherit (home-manager.lib) homeManagerModules;
    in
      flake-utils.lib.eachDefaultSystem (
        system: let
          pkgs = import nixpkgs {inherit system;};
        in {
          packages.default = pkgs.stdenv.mkDerivation {
            pname = "iio-sway";
            version = "1.0";

            src = ./.; # Local source directory

            nativeBuildInputs = with pkgs; [meson ninja pkg-config];
            buildInputs = with pkgs; [dbus];
          };

          devShells.default = pkgs.mkShell {
            buildInputs = with pkgs; [meson ninja pkg-config dbus];
          };
        }
      )
      // {
        homeManagerModules.default = {
          config,
          lib,
          pkgs,
          ...
        }:
          with lib; {
            options.programs.iio-sway = {
              enable = mkEnableOption "Enable iio-sway in Sway";
              display = mkOption {
                type = types.nullOr types.str;
                default = null;
                description = "Optional display parameter for iio-sway.";
              };
            };

            config = mkIf config.programs.iio-sway.enable {
              wayland.windowManager.sway.config.startup = [
                {
                  command =
                    "${self.packages.${pkgs.system}.default}/bin/iio-sway"
                    + optionalString (config.programs.iio-sway.display != null) " ${config.programs.iio-sway.display}";
                }
              ];
            };
          };
      };
}
