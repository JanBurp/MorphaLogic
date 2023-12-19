# MorphaLogic

An Eurorack module based around a Arduino Nano.

With AND, OR, XOR logic functions, a Flip Flip (Q), a Clock Divider mode, and more.

<img src="pictures/FRONT PANEL.png" alt="Morgalogic Front Panel" height="400px">


## Inputs, Outputs & Buttons

<table>
<tr>
<td><b>LEFT</b></td><td style="text-align: right;"><b>RIGHT</b></td>
    <tr><td>INPUT: input (A)</td><td style="text-align: right;">OUTPUT: AND (or clock / 16)</td></tr>
    <tr><td>INPUT: input (B)</td><td style="text-align: right;">OUTPUT: OR (or clock / 8)</td></tr>
    <tr><td>INPUT: Inverse (INV)</td><td style="text-align: right;">OUTPUT: XOR (or clock / 4)</td></tr>
    <tr><td>INPUT: Clock (CLK)</td><td style="text-align: right;">OUTPUT: Q - Flip Flop (or clock / 2)</td></tr>
    <tr><td>OUTPUT: Inverse of TRIG BUTTON</td><td style="text-align: right;">Trigger on press & release of TRIG BUTTON</td></tr>
    <tr><td>BUTTON: Mode button</td><td style="text-align: right;">BUTTON: Reset button</td></tr>
    <tr><td colspan="2" style="text-align: center;">TRIG BUTTON</td></tr>
</tr>
</table>

## Functions

### Logic functions

- Basic logic funcions AND, OR, XOR (using the first inputs on the left side).
- Flip Flop (Q) (first input will SET, second input will RESET).
- NOT input, or MODE button, to inverse all logic outputs (NAND, NOR, NXOR, NQ)
- CLOCK input, when present, all Logic outputs only are HIGH when CLOCK is HIGH
- RESET button to reset Q and CLOCK

### Trig button

When the TRIG button is pressed (and released):

- A LOW gate is send to the first (left) output. It will be HIGH when the button is not pressed.
- A short pulse is send to the second output on press & release of the button.
- If the module is in Clock Divider mode, the press & release time will set the Clock Tempe if no signal is present on the CLOCK input.

## Modes:

Current mode is set in memory.

### Normal mode (mode button is unlit):

- Inputs A & B determines logic outputs (AND, OR, XOR, Q). Where A is set and B is reset for the flipflop (Q).
- If the NOT input is HIGH, all logic outputs are inverted (NAND, NOR, NXOR,NQ)

### Inverse mode (mode button is lit):

- Pressing the mode button will inverse all logic outputs (same as when NOT input is HIGH)

### Clockdivider mode (mode button is flashing):

- A long press on the mode button will put the module in Clockdivider mode. The mode button will flash.
- The logic outputs will divide the clock (where Q = /2, XOR = /4, OR = /8, AND =/16)
- The clock source is:
  - The CLK input
  - If no clock is present on the CLK input, a standard 120 BPM clock is used.
  - Or, if the trig buttons has been pressed and released, the timing between press and release is used to determine the clock time.

### External Trig Mode (trig button flashes):

- A long press of the RESET button toggles between external trig options mode on/off. When on, the trig button flashes.
- When external trig mode is on, the first input can function as an external trig for the trig button.


## Uses

- Use the logic function for all kinds of GATE manipluations, for interesting sequences for example.
- Same for the CLOCK DIVIDER mode.
- Make use of the CLOCK input to quantize the logic outputs with a clock signal.
- Use the TRIG BUTTON (or external trig mode) for changing a GATE into two short TRIGS.
- Use the TRIG BUTTON to fire a LOW gate (patch it into one of the logic function to have a HIGH gate on the OR output).

### Example with a Morphagene

One of my main modules in my eurorack case is the Make-Noise Morphagene. One thing that i'd like it to do is:

- Press and hold button to record - currently playing loop is stopped, and recording is started.
- Release button - recording is stopped and starts playing.

To make this work:

- Prepare the Morphagene by setting the `Record option (rsop)` in the `options.txt` file on the SD card to '1'.
- Patch the TRIGGER output of the TRIG BUTTON to the RECORD input of the Morphagene
- Patch the INVERSE output of the TRIG BUTTOON to the PLAY input of the Morphagene

Now:

- When pressing (and holding) the TRIG BUTTON the Morphagene start recording (and stops playing).
- When releasing the TRIG BUTTON the Morphagene stops recording and starts playing.
- If in Clock Divider mode and no Clock signal is present at the CLOCK input, the tempo is set to the time between pressing and releasing the button.

# Repo:

(github.com/JanBurp/MorphaLogic)[https://github.com/JanBurp/MorphaLogic]


# Buying a kit

If you'd like to build this module you can buy a kit with:

- PCB
- Frontpanel
- 3D printed buttons

Contact me if you are interested.

By Jan den Besten (www.jandenbesten.com)[www.jandenbesten.com]
