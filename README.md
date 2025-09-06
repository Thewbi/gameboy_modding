# Introduction

This repository is about 

- adding a 2 inch LCD Module (ST7789V Controller, 240x320 pixel) to the Gameboy DMG-01
- adding external controllers (NES Controllers) to the Gameboy DMG-01
- converting the image data that the Gameboy outputs to VGA and HDMI for display on modern monitors

# Credit

The techniques used in this repository are basically the same as outlined in the element14 present, hackster.io implementation by Andi West:
https://www.hackster.io/news/making-a-game-boy-far-less-portable-93192b6b92b9
The youtube video is here: https://www.youtube.com/watch?v=ypGMU5lLjeU&t=1s

Also the main inspiration and source code comes from "What's Ken' Making"
Video is here: https://www.youtube.com/watch?v=Q3DdKJIdbBk&t=573s
Git Repository is here: https://github.com/whatskenmaking/Pico-DMG-LCD
I used his source code but I mainly only used the .pio code, the LCD library and the main application code.
The build environment in this repository is a little simpler than Ken's code. 
You need to setup the Pico SDK yourself using the Visual Studio Code extension whereas Ken's build system 
tries to download the SDK automatically.

# Implementation

Get one of the Gameboy DMG-01 tool sets from ebay to open the Gameboy. https://www.ebay.de/itm/155692369305?_skw=Gameboy+repair+tools&itmmeta=01K4EWMD1A75A58V3QPFZJE40R&hash=item243ffd0599:g:GvUAAOSwuMdkxLwn&itmprp=enc%3AAQAKAAAA8FkggFvd1GGDu0w3yXCmi1dH%2FrRsNmD0ErAsBM9w4AO4O7RD4m5zVMbWQSJ4gdWijUZQj%2FCBn5i0GQ7jgvpJVCJXl7cEuVqzO5r5eIOwKkTuWRE11F6hbE%2Bx2TeWePYeZfmMxfhCM6RfZ00ohJYdSydVtyxtOmhXLgc9ll6I2NLJbnduFsbO44MWuJdA3q5di7rMtDNB%2BRmK8xhUCOvhPeMk3VgZ7%2Bz0V69Cr8HUJ2Z4vzZ3%2FHnusgT02RXh2BcOaAXcke25FOlzf5ik4vA9NGP17hVuAs9nF%2BN%2FCFvI%2Buc3nF%2FuTTvf%2FuPw99k8W6vLNg%3D%3D%7Ctkp%3ABFBMmNHR3KNm&pfm=1
Inside the Gameboy there are two PCBs that communicate via a white ribbon cable. 
The front PCB contains the LCD Display and the buttons. The back PCB contains the CPU.
The ribbon cable transfers image data from the CPU to the LCD and Input controls from the buttons to the CPU.
The idea is to solder cables to the ribbon connector pins and read and write the signals using a Raspberry Pi Pico.

## Level Shifting (Convert 5V to 3v3)

The Gameboy DMG-01 is a 5 Volt System and the Pico is a 3v3 system. To not damage the pico, a level shifter is used
which is a bidirectional converter between the two logic levels. Once correctly connected, basically you can forget
that the level shifter is even part of the system because it performs it's task as if it was not there at all.
Just imagine the Gameboy was connected to the Pico directly.

The level shifter is a TXS0108E from TI. Here is an excellent video on how it works: https://www.youtube.com/watch?v=f7aySy_0URE
The way I set up the system is that the Gameboy runs on batteries.
An Arduino provides GND, 3v3 and 5V. 

The TXS0108E has an A-side which inputs/outputs signals on the lower voltage (1.2V up to 3.6V).
The B-side on the TXS0108E is for the higher voltage (1.7V up to 5.5V).
Keep in mind to not connect the high voltage to the A-side or the low voltage to the B-side!
The pico goes on the A-side. The GameBoy DMG-01 goes on the B-side!

The A-side and the B-side do have eight lines running between each other. A1 and B1 are forming a line/pair
and are connected to each other. A2 and B2 form another line/pair. the pair 1 and the pair 2 however are not
connected to each other! Signals are only forwarded per line/pair. This means the shifter can shift up to eight
signals in parallel! The shifter has an internal pull-up resistor on each pin. This means that when the pin
is not driven low by any side, then the pins will normally have a HIGH signal level. Do not get confused when
testing the TXS0108E shifter. If you intend to measure a LOW signal, you have to actively pull the pin LOW!

First, lets discuss how to connect the control pins on the TXS0108E so that it starts shifting signals.

* OE (output enable) on the TXS0108E goes to the Arduino's 5V.
* GND (ground) on the TXS0108E goes to the Arduino's GND.
* VB (Voltage B-side) on the TXS0108E goes to the Arduino's 5V.
* VA (Voltage A-side) on the TXS0108E goes to the Arduino's 3v3.

Next are the pins on the TXS0108E that actually carry the LCD pixel data. (Controller inputs are discussed later).

At first I though GND from the ribbon cable is normal GND such as any other GND.
This is not the case! We have to treat GND from the ribbon cable like a signal!
If you do not use the same GND on the Gameboy and the Pico, then the image on your LCD screen
while be a flickering mess! The image only stabilizes if the same GND is used!
This means, connect the ribbon cable pin 21 (GND) to the TXS0108E (choose any free data pin) then
take the corresponding A-side pin for that GND signal and connect it to the Pico's GND pin! (This is very important!)

Also connect the ribbon cable pins for data0, data1, pixel clock and vertial sync to the TXS0108E.

A pinout of how the pins on the ribbon cable are numbered is available here: https://gbdev.gg8.se/forums/viewtopic.php?id=80

Basically PIN 1 is closest to the power switch of the Gameboy. Pin 21 is farthest away. The pins are just numbered
sequentially. Pin 21 is GND. Pin 16 is dataout 0, pin 15 is dataout 1, Pin 14 is the pixelclock, Pin 12 is vertical sync.

## Connections on the Pico

"What'S Ken Making"'s code uses the PIO feature of the Pico to interpret the pixel data that the Gameboy sends.
What his excellent video in order to learn how the system works and to understand why he designed the code the way he did.

He makes use of the Pico PIO feature which is perfect for the job. The PIO state machine inspects the pixel clock signal
and when it goes low, it will store the current dataout 0 and dataou 1 values. When a vertical sync takes place,
a new row in the pixel buffer is started. The pixel buffer is then output to a LCD or can be used as the source for upscaling.

The pixel buffer is managed by the application that the Pico is executing. This means you have a PIO application running on 
the PIO statemachine parsing the signals. The data that the PIO selects needs to be transferred to the Pico application.
To do so, the the pio uses a start pin. In the source code, the state machine uses start index 2. This is hard coded in the
.pio source code file: 

```
sm_config_set_in_pins(&config, 2);
```

The start index 2 stands for GP2 (General Purpose IO Pin 2, GPIO 2) which is GP2 on the Pico!

The PIO state machine will then execute the PIO assembler file and eventually execute this statement:

```
in pins, 4
```

Here, the input on the next four pins starting at the hardcoded offset 2 from earlier are latched and stored into the
PIO queue from where the application can read the bits. The start index 2 stands for GP2 (General Purpose IO Pin 2, GPIO 2)
on the pico. This means starting at pin GP2 and reading the next four pins, the data on the pins GP2, GP3, GP4 and GP5 are
latched into the queue.

This in turn means that you have to connect the signals that come out of the ribbon cable over the level shifter to the
PICO in the correct manner. 

Here is how to connect the cables:

* GP2 is connected to Ribbon Pin 14 (pixel clock).
* GP3 is connected to Data 0
* GP4 is connected to Data 1
* GP5 is connected to Vertical Sync

Now inside the application, a 32 bit value is read from the PIO state machine:

```
uint32_t result = pio_sm_get_blocking(pio, state_machine_id);
```

Next, there is some bit shifting necessary to get back the bits that the PIO statemachine has inserted into the queue:

```
vSync = (result >> 31) & 1;
data0 = (result >> 29) & 1;
data1 = (result >> 30) & 1;
```

I think that the pixel clock is not even used although it is part of the 32-bit data at index 28.

Given this information, the rest of the application decides what color to insert at the pixel location into the
pixel buffer. The pixel buffer is then output on the LCD screen using the API of the LCD library.

And Bada Bing Bada Boom the gameboy image will appear on the LCD!

# Input Controls

TODO

# Upscaling and HDMI output

TODO
