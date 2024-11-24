# GcToolKit

Tool to create 1:1 backups of PSV Game Cartridges
including CMD56 authenticationd data.

This can also be used to format, backup and restore the writable grw0 and mediaid sections of gamecarts.

This writes *.vci* files, (vita cartridge image) which is essentially
cmd56 authentication data + raw image of gc

Can run a backup over the network, to a offical memory card,
a USB device connected to a Vita TV or OTG on Vita 2k, or Accessory Port on Vita 1K
or host0 on Development Kits.

# NOTE: you have to disable YAMT or other SD2Vita drivers before using this (as they disable GC Authentication)

-- Credit
-  <sup>The Crystal System</sup> Li- Programming the thing, VCI Format, Reverse engineering gamecart CMD56
- olebeck - CMD56 helps
- Robots System - Selecting music, choosing port numbers, ~~emotional support~~
- Princess of Sleeping - ExFAT Format code, CMD56 helps
- SKGLeba - psp2spl for F00D Code execution
- dots_tb - USB OTG
- EA Games 1997 - BGM Music from Dungeon Keeper 1 https://www.youtube.com/watch?v=RXfUV_z7i0c

-- OTG Compatiblity

This program allows backup vita GCs with a USB device connected via an OTG cable
however this only works with OTG cables with an external power source; or "Y-Cable"
for example this one for the Amazon Fire Stick https://www.amazon.com/ANDTOBO-Micro-Adapter-Power-Devices/dp/B083M1S6QT will work.

-- Difference between .vci and .psv formats

The main difference is how the keys stored. 
in psvgamesd, the result of gc_auth_mgr_sm function 0x20 is stored,
this is the key required to decrypt the .RIF file
however this key is *actually* derived from the result of SHA256 hash functions
of some constants exchanged in packet20 and packet18 of gc authentication.
in .VCI the *input* to the SHA256 function are included instead.
SHA256 is a one-way function and so you cannot go backwards from 
the data captured in psvgamesd to the packet20 and packet18 constants.

main advantage is that with VCI it would be thereotically possible to create a vita flash cartridge. .
this, also means that .VCI can be easily converted to .PSV, but .PSV cannot be converted back to VCI

for tools to convert to/from VCI format to others, see: https://silica.codes/Li/VCI-TOOLS

![gc authentication diagram](https://silica.codes/Li/GcDumper/raw/branch/main/diagram.png)