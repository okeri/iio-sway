# iio-sway
Listen iio-sensor-proxy and auto change sway output orientation

## installing 
1. Make sure iio-sensor-proxy running
2. add exec iio-sway to your sway config file

## running
./iio-sway [sway-output to rotate, default=eDP-1], run **swaymsg -t get_outputs** to list.
