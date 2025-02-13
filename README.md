# iio-sway

Listen iio-sensor-proxy and auto change sway output orientation

## installing

1. Make sure iio-sensor-proxy running
2. add exec iio-sway to your sway config file

## running

./iio-sway [sway-output to rotate, default=eDP-1], run **swaymsg -t
get_outputs** to list.

## NixOS with Home Manager

In your flake.nix

```nix
inputs.iio-sway.url = "github:tbaumann/iio-sway";
```

Use it in a home manager module

```nix
{ config, pkgs, ... }:

{
  imports = [ inputs.iio-sway.homeManagerModules.default ];

  programs.iio-sway = {
    enable = true;
    display = "HDMI-1"; # Optional
  };
}
```
