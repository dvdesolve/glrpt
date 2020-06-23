# glrpt
Originally developed by [Neoklis Kyriazis](http://www.5b4az.org/), `glrpt` provides all-in-one solution to receive, demodulate and decode LRPT images on Linux without tinkering with audio pipes and 3rd-party software as with SDR# and LRPT-decoder.

![Screenshot of version 2.5.0](screen_2.5.0.png "glrpt version 2.5.0")

## Requirements

### Hardware
`glrpt` uses [SoapySDR](https://github.com/pothosware/SoapySDR) library to communicate with SDR hardware. So in principle any hardware supported by SoapySDR should work. However, only [RTL-SDR](https://www.rtl-sdr.com/buy-rtl-sdr-dvb-t-dongles/), [Airspy Mini](https://airspy.com/airspy-mini) and [Airspy R2](https://airspy.com/airspy-r2) units were tested quite well.

### Software
In order to use `glrpt` one should have the following dependencies satisfied:
- `gtk+` (3.22.0 or higher)
- `glibc`
- `glib2`
- `SoapySDR` (install proper modules to get support for your hardware)
- `libjpeg-turbo`
- `libconfig`

To compile `glrpt` by hand be sure to have the following packages installed:
- `gcc` (4.8 or higher) / `clang` (3.1 or higher)
- `make`
- `cmake` (3.12 or higher)

## Installation
First of all check if `glrpt` is already in your distro repository. For example, on Arch Linux you can install it [from AUR](https://aur.archlinux.org/packages/glrpt/). If there is no package for your distro then you must compile it by hand.

### Building from source code
Download latest stable release or clone `master` branch directly:
```
git clone https://github.com/dvdesolve/glrpt.git
cd glrpt
```

Prepare your build (for example, the following will install `glrpt` into `/usr`):
```
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=/usr ..
```

Build and install `glrpt`:
```
make
make install
```

Now you're ready to use `glrpt`. You can run it from your favorite WM's menu or directly from terminal (recommended if something goes wrong because there will be additional debug info).

## Usage

### Config files
`glrpt` comes with a set of sample config files that are usually stored in `/usr/share/glrpt/config` (depends on installation). For more usability you can store your own config files in `$XDG_CONFIG_HOME/glrpt` (or `$HOME/.config/glrpt` if XDG vars are not set). `glrpt`'s config files use [libconfig](http://hyperrealm.github.io/libconfig/) syntax. Example config files are well-documented so consult them for details.

Maximum length of string parameters in config files is limited to the 80 characters. Please use ASCII symbols only in strings! Also there are a bunch of mandatory options that should be presented in config files. They are listed below.

#### SDR receiver frequency (kHz)
Option name: `freq`; option group: `receiver`; valid values: `136000 <= freq <= 138000`.

#### QPSK modulation mode
Option name: `mode`; option group: `demodulator`; valid values: `"QPSK", "DOQPSK", "IDOQPSK"`.

#### QPSK transmission symbol rate (Sym/s)
Option name: `rate`; option group: `demodulator`; valid values: `50000 <= rate <= 100000`.

#### Active APIDs
Option name: `apids`; option group: `decoder`; valid values: `[64-69, 64-69, 64-69]`.

All missing/invalid optional settings will be set to their defaults. So minimal working configuration would be like that (Meteor-M2 example):
```
receiver: {
    freq = 137100
}

demodulator: {
    mode = "QPSK"
    rate = "72000"
}

decoder: {
    apids = [66, 65, 64]
}
```

### Decoding images
Use [GPredict](https://github.com/csete/gpredict) to get passes list for the satellite of interest. Connect your SDR receiver and run `glrpt`. Select proper config via right-clicking in LRPT image area (system-wide configs are separated from user's configs and followed by them). Wait until satellite rises over the horizon to the decent angle and press "Start button". You can tweak gain settings during reception to get the best SNR. When the pass is over or you decided to stop click that button once again. Decoded images will be saved into `$XDG_CACHE_HOME/glrpt` (or in `$HOME/.cache/glrpt` if `$XDG_CACHE_HOME` is not set).

## Troubleshooting

### `glrpt` fails to initialize device
`glrpt` requires connected SDR before starting decoding. Connect your receiver and repeat.

### PLL never locks
Try to play with gain settings and find reasonable value to get highest SNR (use FFT waterfall as a reference). Also you can try to increase filter bandwidth in config file to somewhat higher value such as 140 kHz. One more thing you could try is to increase PLL lock threshold.

### Poor signal quality
Be sure to properly install and tune your antenna. [V-dipole](https://lna4all.blogspot.com/2017/02/diy-137-mhz-wx-sat-v-dipole-antenna.html) setup is the most simplest solution. Also you can try turnstile, double cross and QFH antennas. Switching to manual gain setting can help you to get decent SNR.

### My images are rotated upside-down!
If you've catched South-to-North satellite pass you will end up with flipped image. Use invert feature that `glrpt` provides: right-click on image area in GUI and select menu entry "Invert image".

## Additional information

### Current Meteor status
You can monitor current status of Meteor satellites and operational characteristics [here](http://happysat.nl/Meteor/html/Meteor_Status.html).

### More info about Meteor-M2 satellite
[This page](https://directory.eoportal.org/web/eoportal/satellite-missions/m/meteor-m-2) contains quite detailed characteristics of the Meteor-M2 mission including description of instruments installed on the satellite and its operational parameters.

### Meteor-3M satellite programme
You could learn about past, current and future missions of Meteor-3M satellite programme [here](https://www.wmo-sat.info/oscar/satelliteprogrammes/view/100).
