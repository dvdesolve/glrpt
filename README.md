# glrpt
Originally developed by [Neoklis Kyriazis](http://www.5b4az.org/), `glrpt` provides all-in-one solution to receive, demodulate and decode LRPT images on Linux without tinkering with audio pipes and 3rd-party software as with SDR# and LRPT-decoder.

## Requirements
### Hardware
`glrpt` uses [SoapySDR](https://github.com/pothosware/SoapySDR) library to communicate with SDR hardware. So in principle any hardware supported by SoapySDR should work. However, only [RTL-SDR](https://www.rtl-sdr.com/buy-rtl-sdr-dvb-t-dongles/), [Airspy Mini](https://airspy.com/airspy-mini) and [Airspy R2](https://airspy.com/airspy-r2) units were tested quite well.

### Software
In order to use `glrpt` one should have the following dependencies satisfied:
- `gtk+` (3.22.0 or higher)
- `glibc`
- `glib2`
- `SoapySDR` (install RTL-SDR and/or Airspy modules to have support for your hardware)

Also if you want to compile `glrpt` by hand be sure to have the following installed:
- `gcc`
- `make`
- `automake`
- `autoconf`

## Installation
First of all check if `glrpt` is already in your distro repository. For example, on Arch Linux you can install it [from AUR](https://aur.archlinux.org/packages/glrpt/). If there is no package for your distro then you must compile it by hands.

### Building from source code
Download latest stable release or clone `master` branch directly:
```
git clone https://github.com/dvdesolve/glrpt.git
cd glrpt
```

Prepare your build (for example, the following will install `glrpt` into `/usr` instead of `/usr/local`):
```
./autogen.sh
./configure --prefix=/usr
```

Build and install `glrpt`:
```
make
make install
```

Now you're ready to use `glrpt`. You can run it from your favorite WM's menu or directly in console (preferred if something goes wrong because there are always additional debug info in case of troubles).
