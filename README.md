# Introduction

This repository is about 

- adding a 2 inch LCD Module (ST7789V Controller, 240x320 pixel) to the Gameboy DMG-01
- adding external controllers (NES Controllers) to the Gameboy DMG-01
- converting the image data that the Gameboy outputs to VGA and HDMI for display on modern monitors

# Credit

The techniques used in this repository are basically the same as outlined in the element14 present, hackster.io implementation by Andi West:
https://www.hackster.io/news/making-a-game-boy-far-less-portable-93192b6b92b9
The youtube video is here: https://www.youtube.com/watch?v=ypGMU5lLjeU

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
An Arduino provides GND, 3v3 and 5V and is not use to run any source code beside that.
The Arduino is just used as a power supply and to provide GND.

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

This section is about getting input into the Gameboy DMG-01 using an external controller.

HINT: I tried to provide input to the Gameboy DMG 01 using an Arduino UNO using code from 
the Arduino IDE and using the API functions (Not using assembler programming techniques!)
The Arduino Code did not work! I was not able to provide input to the Gameboy. I did not 
figure out why! Maybe the Pins cannot switch fast enough or the code is too slow without using
assembler. I do not know. I switched over to an Raspberry Pi Pico wich was able to provide
input even without the PIO feature! My hint is to not use an Arduino UNO but use a Raspberry
Pi PICO (including a voltage level shifter!)

First lets use an Raspberry Pi Pico (without connecting external controllers) to send signals into the Gameboy.
Here is an explanation of the way input works on the Gameboy: https://youtu.be/ypGMU5lLjeU?t=605

Here is a project that also provides input to the gameboy: https://bitbuilt.net/forums/threads/consolized-gameboy-guide-wiring-a-nes-controller-to-a-dmg.2499/

Here are the schematics: https://gbdev.gg8.se/wiki/articles/DMG_Schematics

## Pinout

The pinout on the Gameboy DMG-01 ribbon cable connector is:
Pin 1 is closest to the Gameboy's power switch. Pin 21 farthest away.

* Pin 9 -- Select "Other"		(Purple, White)
* Pin 8 -- Right / A			(Blue)
* Pin 7 -- Up / Select			(Green)
* Pin 6 -- Down / Start			(Yellow)
* Pin 5 -- Select "D-Pad"      	(Orange, Black)
* Pin 4 -- Left / B          	(Red)

## Workings

There are eight buttons on the Gameboy that a game can read. They are organized into two groups called "D-Pad" and "Others".

* D-Pad: Left, Right, Up, Down
* Others: A, B, Start, Select

The Pins 4, 6, 7 and 8 are connected to both groups at the same time within the controller!
One or more pins can be asserted at the same time. Every button that is pressed asserts it's line.
The lines are asserted active LOW! Meaning when a button is pressed, the line goes to GND.

The Pins 4, 6, 7 and 8 are "in" pins seen from the perspective of the Gameboy's CPU. The game will read from those pins.
This means that the game has to tell the controller which group to activate so that the controller can present the
button state for the activated group and so that the game can then read that part of the controller's state.
After the game has read the first half of the buttons, it will switch over and activate the second group.
The controller will then provide the state of the other half of the buttons on the pins 4, 6, 7 and 8.
The game will then read the second half.
This is how four pins are reused to accomodate eight buttons.

The Pins 5 and 9 are "out" pins seen from the perspective of the Gameboy's CPU. The game will write to those pins
in order to select the group.

When the Game running on the Gameboy CPU wants to read the "D-Pad" group, it will pull PIN 5 (Select "D-Pad") LOW!
When the Game running on the Gameboy CPU wants to read the "Others" group, A, B, Start, Select buttons, it will pull Pin 9 (Purple, White) LOW!

Selecting none of the two groups by either setting Pins 5 and 9 both HIGH or LOW at the same time is a case
that makes no sense from the input management perspective but happens in some games on purpose in order to reuse
registers inside the CPU that are normally reserved for input processing! So this means in case both PINS 5 and 9
are either pulled HIGH or LOW at the same time, the save thing for the controller to do is to set the input pins 4, 6, 7 and 8 to HIGH!

The pins are pulled LOW for 18 microseconds and 40 microseconds.
Lets assume we want to sample every 10 microseconds.
There are 10^6 = a million microsends in a second.
This means we need to sample with 0.1 megahertz or 100 kHz = 100000 Hz.

Again, something caused the Arduino UNO to not be able to act as a input provider for the GameBoy DMG-01.
I strongly advise on using a Raspberry Pi Pico, an STM32 or even an FPGA instead!

Sample Code: https://gist.github.com/uXeBoy/5e9ec52823b7d73a187370573bdbda1b

Sample Code for the Raspberry Pi PICO from https://youtu.be/ypGMU5lLjeU?t=680

```
// set GPIO input mode for input pins
pinMode(BUTTONS_DPAD_PIN, INPUT);
pinMode(BUTTONS_OTHER_PIN, INPUT);

// set GPIO output mode for output pins
pinMode(BUTTONS_RIGHT_A_PIN, OUTPUT);
pinMode(BUTTONS_LEFT_B_PIN, OUTPUT);
pinMode(BUTTONS_UP_SELECT_PIN, OUTPUT);
pinMode(BUTTONS_DOWN_START_PIN, OUTPUT);

while (true) {

	uint dpad_selected_value = gpio_get(BUTTONS_DPAD_PIN);
	uint other_selected_value = gpio_get(BUTTONS_OTHER_PIN);
	
	if (dpad_selected_value == 0) {
	
		gpio_put(BUTTONS_RIGHT_A_PIN, button_states[BUTTON_RIGHT]);
		gpio_put(BUTTONS_LEFT_B_PIN, button_states[BUTTON_LEFT]);
		gpio_put(BUTTONS_UP_SELECT_PIN, button_states[BUTTON_UP]);
		gpio_put(BUTTONS_DOWN_START_PIN, button_states[BUTTON_DOWN]);
		
	} else if (other_selected_value == 0) {
	
		gpio_put(BUTTONS_RIGHT_A_PIN, button_states[BUTTON_A]);
		gpio_put(BUTTONS_LEFT_B_PIN, button_states[BUTTON_B]);
		gpio_put(BUTTONS_UP_SELECT_PIN, button_states[BUTTON_SELECT]);
		gpio_put(BUTTONS_DOWN_START_PIN, button_states[BUTTON_START]);
	
	} else {

		gpio_put(BUTTONS_RIGHT_A_PIN, 1);
		gpio_put(BUTTONS_LEFT_B_PIN, 1);
		gpio_put(BUTTONS_UP_SELECT_PIN, 1);
		gpio_put(BUTTONS_DOWN_START_PIN, 1);

	}
	
}
```

## Explanation of the Source Code

The Control input source code is stored in the GameBoyInput project (contained in the GameBoyInput subfolder of this repo).
You can open the project using Visual Studio Code and the Raspberry Pi Pico extension.
The Code is running on the Raspberry Pi Pico.

The Raspberry Pi Pico has two cores.
One core is used for consuming input from an NES controller and to store the input into an array.
The second core is used to respond to the GameBoy polling for input. The response will provide the 
input data that the first core has polled from the NES controller.

The idea and design is credited to Andi West from Element14: https://www.hackster.io/news/making-a-game-boy-far-less-portable-93192b6b92b9

In order to use both of the Raspberry Pi Pico's cores, edit the CMakeLists.xml file and add the multicore library to the build.

```
target_link_libraries(GameBoyInput
        pico_stdlib
        pico_multicore)
```

Next, include the multicore header file into your source code file.

```
#include "pico/multicore.h"
```

The idea is that the existing main function runs on core 0 and the user can start a second function 
which is explicitly run on core1 using a call to multicore_launch_core1();
multicore_launch_core1(); expects a function pointer as a parameter. It will execute the provided
function on core1. The provided function typically contains an endless loop for the core1 to run
indefinitely.

The call to multicore_launch_core1() is the first thing that the main function will perform. 
Once multicore_launch_core1() was called, main just continues as normal with the source code for
core0. This means that core1 is kicked of by code running on core0.

The code below will blink a LED in the function running on core1 in parallel with the main function
which is run on core0 from main().

```
#include "pico/multicore.h"

#ifndef LED_DELAY_MS
#define LED_DELAY_MS 500
#endif

...

void core1_main()
{
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    while (true) {
        gpio_put(PICO_DEFAULT_LED_PIN, true);
        sleep_ms(LED_DELAY_MS);
        gpio_put(PICO_DEFAULT_LED_PIN, false);
        sleep_ms(LED_DELAY_MS);
    }
}

int main()
{
    //stdio_init_all();

    multicore_launch_core1(&core1_main);
	
	...
```

In the following I will call the functions running on each core a task function or just a task!

A global variable can be read and written by both tasks!
This can lead to hard to detect bugs and task synchronization is necessary to get the application 
right.

In our very specific example, there is a one-way relationship between the tasks.
One of the task fills the button_states array with button states polled from the NES controller.
The other task will read from the button_states array (but never read) to adapt the information
to the format that the GambBoy understands.

Since only one of the task writes the button_states array an the other task only reads and because
we do not care about the overall quality of the data (best effort) there is really no need
for any specific task synchronization.

In more complex applications you need to take inter task communication very seriously!


# Upscaling and HDMI output

TODO

# Cartridge Simulation

https://www.youtube.com/watch?v=ix5yZm4fwFQ

TODO
