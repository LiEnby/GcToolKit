# GcDumper

Create 1:1 backups of PSV Game Carts, with authentication data.

This writes *.vci* files, (vita cartridge image) which is essentially
cmd56 authentication data + raw image of gc

Allows you to backup a vita game cart to a USB Device via an OTG cable on Vita2K, USB Port on VitaTV, 
or an offical memory card, or over the local network.

-- Difference between .vci and .psv formats: 

The main difference is how the keys stored. 

in psvgamesd, the result of gc_auth_mgr_sm function 0x20 is stored,
this is the key required to decrypt the .RIF file

however this key is *actually* derived from the result of SHA256 hash functions
of some constants exchanged in packet20 and packet18 of gc authentication.

in .VCI the *input* to these functions are included instead.

SHA256 is a one-way function and so you cannot go backwards from 
the data captured in psvgamesd to the packet20 and packet18 constants.

with VCI it would be thereotically possible to create a vita flash cartridge, 

this, also means that .VCI can be easily converted to .PSV,  
but .PSV cannot be converted back to VCI.


![gc authentication diagram](https://silica.codes/Li/GcDumper/raw/branch/main/diagram.png)