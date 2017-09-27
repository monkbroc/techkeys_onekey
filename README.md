# Techkeys keyboard

[Forum post on compiling the Techkeys firmware](https://geekhack.org/index.php?topic=53378.0)

Download AVR GCC from http://andybrown.me.uk/2012/04/28/avr-gcc-4-7-0-and-avr-libc-1-8-0-compiled-for-windows/

Add to path
```
set PATH=C:\tools\avr-gcc\bin;c:\tools\dfu-programmer\;%PATH%
```

Run `make`

## Flash

Install dfu-programmer

Short the 2 pads next to the USB connector. In the Device Manager,
you'll see at the top Atmel USB Devices -> ATmega16U2

Run `make flash`

