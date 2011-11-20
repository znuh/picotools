# PICOTOOLS - Picoscope PS5203/5204 tools

Linux tool(s) for working with the PicoScope(R) PS5203/5204.

This is some ugly crap: 100% kludges, but no time for a rewrite yet

![screen-00](https://raw.github.com/koppi/picotools/master/screen-00.png "picotools gnuplot screenshot 00")

# Build / Install - Ubuntu 11.04

## Build dependencies (64 bit)

```
 $ sudo apt-get install alien fakeroot
```

### libps5000_R1_RA - PicoScope Linux drivers

 * Go to http://labs.picotech.com/#linux
 * Download the 64–bit PS5000 Linux driver:

```
 $ wget http://labs.picotech.com/software/libps5000_R1_3_6_6_3.x86_64.tgz
 $ tar xvfz libps5000_R1_3_6_6_3.x86_64.tgz
 $ cd libps5000-1.3.6.6-3.x86_64
 $ sudo alien libps5000-1.3.6.6-3.x86_64.rpm
 $ sudo dpkg -i libps5000_1.3.6.6-4_amd64.deb
 $ dpkg -L libps5000
```

### libraries

```
 $ sudo apt-get install libsdl-ttf2.0-dev libsdl-gfx1.2-dev libglade2-dev
```

## Compile / Install

```
 $ sudo make install
```

# Authors

 * znuh - https://github.com/znuh

