## RF Control

Currently I only own a few Taranis RC transmitters. They are all using the 2.4 GHz band, which does not penetrate water at all. They do support very advanced features, including scripting, and support plugin modules that can transmit on other frequencies.

According to [this forum post on SubCommittee.com](https://subcommittee.com/forum/showthread.php?32640-433mhz-links-and-info-\(Tim-s-Regatta-Seminar&highlight=orangerx), a 433 MHz transmitter outputting only 6 mW can penetrate 1 meter of water. The [OrangeRX OpenLRS module](https://hobbyking.com/en_us/orangerx-open-lrs-433mhz-transmitter-1w-jr-turnigy-compatible.html) can be purchased and used with my Taranis transmitters.

For the receiver, it's better for me to simply buy a HopeRF RFM22B, and attach my own microcontroller to it. The antenna length for the 433 MHz wavelength is 173mm (or 307mm long for 244 MHz).

## Microcontroller

Teensy is a good choice, there's enough pins to drive 3 motors and also run the SPI bus for the RFM22B. The [OpenLRS firmware](https://github.com/openLRSng/openLRSng) can be compiled to run on the Teensy.

## Motor Driver

Each motor in the system uses a MC33926 as the motor driver. It can deliver 3A continuously which is plenty good for the 280 sized DC motors that are used to drive the propellers and pump.

Brushed ESCs have proven to be very unreliable and not worth the convenience.

## Power

A LiPo 2S pack is used to power the submarine. Both a 5V and a 3.3V voltage regulator will be required.

A power switch is installed in the front of the WTC, the [NKK Switches ‎S6AW‎](https://www.digikey.com/en/products/detail/S6AW/360-1928-ND/1007006?itemSeq=366826186). It is waterproof and rated for 20A of current.
