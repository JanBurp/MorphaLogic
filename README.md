# MorphaLogic

An eurorack logic module build with an arduino nano with several logic functions.

# Goal: use Make-Noise Morphagene as easy to use looper

One of my main modules in my eurorack case is the Make-Noise Morphagene. One thing that i'd like it to do is:

- Press and hold button to record - currently playing loop is stopped, and recording is started.
- Release button - recording is stopped and starts playing.

The Morphagene has a trigger input for record and for play. So with some external logic the above scenario is possible to achieve.
So i created a module with these functions to achieve this goal:

- Button
- Gate output that is the inverted state of the Button (goes into the Play input of the Morphagene).
- Trigger output that sends a trigger with a state change of the button (goes into the Record input of the Morphagene)
- A Gate input which has the same functionality as the Button

# Extra's

While the above functions are small and i could make some more use of the Arduino i added some extra's:

- Clock Divider with pot to set the division
- Logic AND, OR, XOR port with switch to invert the outputs resulting in a NAND, NOR, NXOR port

# In this repo

In this repo you'll find:

  - a .stl file to 3D print a frontpanel/closure for the module
  - arduino program (with some inspiration from joeSeggiola / arduino-eurorack-projects lib)
  - a basic example schematic for the inputs
  - some pictures

Outputs (gate or LED's) are just simple 220ohm resistors from a D2..D13 ouput, to the LED or GATE output, and then connected to the ground.





