# GcDumper

Create 1:1 backups of PSV Game Carts, with authentication data.

This writes *.vci* files, (vita cartridge image) which is essentially
cmd56 authentication data + raw image of gc

Allows you to backup a vita game cart to a USB Device via an OTG cable on Vita2K, USB Port on VitaTV, 
or an offical memory card, or over the local network.


-- OTG Compatiblity --

This program allows backup vita GCs with a USB device connected via an OTG cable
however this only works with OTG cables with an external power source; or "Y-Cable"
for example this one for the Amazon Fire Stick https://www.amazon.com/ANDTOBO-Micro-Adapter-Power-Devices/dp/B083M1S6QT will work.

-- Difference between .vci and .psv formats --

The main difference is how the keys stored. 
in psvgamesd, the result of gc_auth_mgr_sm function 0x20 is stored,
this is the key required to decrypt the .RIF file
however this key is *actually* derived from the result of SHA256 hash functions
of some constants exchanged in packet20 and packet18 of gc authentication.
in .VCI the *input* to the SHA256 function are included instead.
SHA256 is a one-way function and so you cannot go backwards from 
the data captured in psvgamesd to the packet20 and packet18 constants.

main advantage is that with VCI it would be thereotically possible to create a vita flash cartridge. .
this, also means that .VCI can be easily converted to .PSV, but .PSV cannot be converted back to VCI.


![gc authentication diagram](https://silica.codes/Li/GcDumper/raw/branch/main/diagram.png)