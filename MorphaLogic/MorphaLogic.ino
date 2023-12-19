/**
 * 
 * MorphaLogic
 * 
 * An Eurorack module based around a Arduino Nano with:
 * 
 *  - Basic logic funcions AND, OR, XOR
 *  - Flip Flop (Q)
 *  - NOT button and NOT input to inverse all logic outputs (NAND, NOR, NXOR, NQ)
 *  - CLOCK input, when present, all Logic outputs only are HIGH when CLOCK is HIGH
 *  - RESET button to reset Q and CLOCK
 *  - Trig button and two outputs to control a MakeNoise Morphagene (PLAY and REC outputs)
 *  
 *  - Each in and outputs has its corresponding LED to show its status.
 *  - Also for each button.
 *
 * How does it work:
 *
 * - Normal mode:
 *     - Inputs A & B determines logic outputs (AND, OR, XOR, Q). Where A is set and B is reset for the flipflop (Q).
 *     - If the NOT input is HIGH, all logic outputs are inverted (NAND, NOR, NXOR,NQ)
 *     - PLAY output is HIGH. Except during a press on the trig button.
 *     - Every change of the trig button (on->off, or off->on) makes the REC output flash shortly.
 *
 * - Inverse mode:
 *     - Pressing the mode button will inverse all logic outputs (same as when NOT input is HIGH)
 *
 * - Clockdivider mode:
 *     - A long press on the mode button will put the module in Clockdivider mode. The mode button will flash.
 *     - The logic outputs will divide the clock (where Q = /2, XOR = /4, OR = /8, AND =/16)
 *     - The clock source is:
 *         - The CLK input
 *         - If no clock is present on the CLK input, a standard 120 BPM clock is used.
 *         - Or, if the trig buttons has been pressed and released, the timing between press and release is used to determine the clock time.
 *
 * - External trig mode:
 *     - A long press of the RESET button switches the external trig options on/off. When on, the trih button flashes.
 *     - The Trig button functions the same in both modes.
 *     - But when external trig mode is on, input A functions as an external trig button
 *
 *
 * Both mode settings are stored in memory.
 *
 *
 *  (c) Jan den Besten
 * 
 */

#define DEBUG false

#include <EEPROM.h>

#include "lib/Input.cpp"
#include "lib/Button.cpp"
#include "lib/Output.cpp"


/**
 * PINS
 */

#define INPUT_A             19  // A5
#define INPUT_B             18  // A4
#define INPUT_NOT           17  // A3
#define INPUT_CLK           16  // A2

#define BUTTON_MODE         11  // LEFT BUTTON
#define BUTTON_RESET        12  // RIGHT BUTTON
#define BUTTON_TRIG         10  // MAIN BUTTON

#define OUTPUT_AND          2
#define OUTPUT_OR           3
#define OUTPUT_XOR          4
#define OUTPUT_Q            5
#define OUTPUT_PLAY         6
#define OUTPUT_REC          7

#define LED_BUTTON_TRIG     13
#define LED_BUTTON_MODE     8
#define LED_BUTTON_RESET    9

/**
 * MODES
 */
#define LOGIC               0
#define INVERSE             1
#define CLOCKDIVIDER        2

#define NORMAL_TRIG         1
#define EXTERNAL_TRIG       2

/**
 * TIMINGS
 */
#define LONG_BUTTON_PRESS   1000        // Time to press a button to trigger the long press function
#define RESET_CLOCK_DELAY   2000        // Time to reset clock input


byte ClockDivisions[4] = { 1,2,4,8 };


struct STATE {

    // inputs
    Input inputA;
    Input inputB;
    Input inputNOT;
    Input inputCLK;

    // buttons
    Button buttonMODE;
    Button buttonRESET;
    Button buttonTRIG;

    // states
    bool A;
    bool prevA;
    bool B;
    bool NOT;
    bool CLK;
    bool MODE_BUTTON;
    bool RESET_BUTTON;
    bool TRIG_BUTTON;
    bool Q;

    // mode
    byte prevMode;
    byte mode;

    // clock
    bool hasClock;
    bool clockIsRunning;
    bool prevClock;
    bool risingClock;
    bool fallingClock;
    int ticks[4];

    // timings
    unsigned long startPeriod;
    unsigned long clockPeriod;
    unsigned long startLoopPeriod;
    unsigned long loopPeriod;

    // trigger modes
    bool prevTrig;
    byte trigMode;
    byte prevTrigMode;

} State;


struct OUTPUTS {

    // outputs
    Output AND;
    Output OR;
    Output XOR;
    Output Q;
    Output PLAY;
    Output REC;

    // LEDS
    Output mode;
    Output reset;
    Output trig;

} Outputs;



/**
 * 
 * ==== SETUP ====
 * 
 */

void setup() {
    if (DEBUG) Serial.begin(9600);

    // state input pins
    State.inputA.init( INPUT_A );
    State.inputB.init( INPUT_B );
    State.inputNOT.init( INPUT_NOT );
    State.inputCLK.init( INPUT_CLK );
    State.buttonMODE.init( BUTTON_MODE );
    State.buttonRESET.init( BUTTON_RESET );
    State.buttonTRIG.init( BUTTON_TRIG );

    // state defaults
    State.A = false;
    State.prevA = false;
    State.B = false;
    State.NOT = false;
    State.CLK = false;
    State.MODE_BUTTON = false;
    State.RESET_BUTTON = false;
    State.TRIG_BUTTON = false;
    State.Q = false;

    State.prevMode = LOGIC;
    // State.mode = LOGIC;
    EEPROM.get(0, State.mode);

    State.clockIsRunning = false;
    State.prevClock = false;
    State.risingClock = false;
    State.fallingClock = false;
    State.startPeriod = 0;
    State.clockPeriod = 500; // 120bpm

    State.prevTrig = false;
    State.prevTrigMode = NORMAL_TRIG;
    // State.trigMode = NORMAL_TRIG;
    EEPROM.get(1, State.trigMode);
    State.startLoopPeriod = 0;
    State.loopPeriod = 0;

    // output pins
    Outputs.AND.init( OUTPUT_AND );
    Outputs.OR.init( OUTPUT_OR );
    Outputs.XOR.init( OUTPUT_XOR );
    Outputs.Q.init( OUTPUT_Q );
    Outputs.PLAY.init( OUTPUT_PLAY );
    Outputs.REC.init( OUTPUT_REC );
    // led pins
    Outputs.mode.init( LED_BUTTON_MODE );
    Outputs.reset.init( LED_BUTTON_RESET );
    Outputs.trig.init( LED_BUTTON_TRIG );
}


/**
 * Read's inputs & buttons
 * And prepare states
 */
void setState() {

    State.A = State.inputA.read();
    State.B = State.inputB.read();
    State.NOT = State.inputNOT.read();
    State.CLK = State.inputCLK.read();
    State.MODE_BUTTON = State.buttonMODE.read();
    State.RESET_BUTTON = State.buttonRESET.read();
    State.TRIG_BUTTON = State.buttonTRIG.read();

    // Mode
    byte currentMode = State.mode;
    if ( State.buttonMODE.readOnce() ) {
        if ( State.mode == LOGIC ) {
            State.mode = INVERSE;
        }
        else {
            State.mode = LOGIC;
        }
    }
    if ( State.buttonMODE.readLongPressOnce(LONG_BUTTON_PRESS) ) {
        State.mode = CLOCKDIVIDER;
    }
    // Keep in memory if changed
    if (currentMode!=State.mode) {
        EEPROM.write(0, State.mode);
    }

    // Reset
    if ( State.RESET_BUTTON == HIGH ) {
        State.hasClock = false;
        State.clockIsRunning = false;
    }

    // Clock
    State.risingClock = ( State.CLK==HIGH && State.prevClock==LOW );
    State.fallingClock = ( State.CLK==LOW && State.prevClock==HIGH );
    if ( State.risingClock ) {
        if (State.startPeriod > 0) {
            State.clockPeriod = millis() - State.startPeriod;
            State.hasClock = true;
        }
        State.startPeriod = millis();
    }
    if ( State.CLK==LOW && millis()-State.startPeriod > RESET_CLOCK_DELAY ) {
        State.hasClock = false;
    }

    State.prevClock = State.CLK;


    // Trig & Loop timing
    if ( State.prevTrig==false && State.TRIG_BUTTON==true) {
        State.startLoopPeriod = millis();
    }
    if ( State.prevTrig==true && State.TRIG_BUTTON==false) {
        State.loopPeriod = millis() - State.startLoopPeriod;
        State.startLoopPeriod = 0;
    }

    // Trigmode
    if ( State.buttonRESET.readLongPressOnce(LONG_BUTTON_PRESS) ) {
        if ( State.trigMode == NORMAL_TRIG ) {
            State.trigMode = EXTERNAL_TRIG;
        }
        else {
            State.trigMode = NORMAL_TRIG;
        }
    }
    // Keep in memory if changed
    if ( State.trigMode != State.prevTrigMode ) {
        EEPROM.write(1, State.trigMode );
    }


}

/**
 * Set button LED's according to state
 */
void buttonLEDS() {
    if (DEBUG) {
        Serial.print("Mode:");
        switch ( State.mode ) {
            case LOGIC:
                Serial.print("LOGIC           CLK: "); Serial.print( State.hasClock ); Serial.print(" RUNNING: "); Serial.println(State.clockIsRunning);
                break;
            case INVERSE:
                Serial.print("INVERSE         CLK: "); Serial.print( State.hasClock ); Serial.print(" RUNNING: "); Serial.println(State.clockIsRunning);
                break;
            case CLOCKDIVIDER:
                Serial.print("CLOCKDIVIDER    CLK: "); Serial.print( State.hasClock ); Serial.print(" RUNNING: "); Serial.println(State.clockIsRunning);
                break;
        }
    }

    // Mode
    if ( State.mode != State.prevMode ) {
        switch ( State.mode ) {
            case LOGIC:
                Outputs.mode.off();
                break;
            case INVERSE:
                Outputs.mode.on();
                break;
            case CLOCKDIVIDER:
                Outputs.mode.blink(500);
                break;
        }
    }
    State.prevMode = State.mode;

    // Reset
    Outputs.reset.set( State.RESET_BUTTON );

    // Trig
    if ( State.trigMode == EXTERNAL_TRIG ) {
        if ( State.trigMode != State.prevTrigMode ) {
            Outputs.trig.blink(500);
        }
    }
    else {
        Outputs.trig.set( State.TRIG_BUTTON );
    }
    State.prevTrigMode = State.trigMode;
}


/**
 * Determine clockdividers outputs
 */
void ClockDivider() {

    // Reset
    if ( !State.clockIsRunning || State.RESET_BUTTON==HIGH ) {
        Outputs.AND.off();
        Outputs.OR.off();
        Outputs.XOR.off();
        Outputs.Q.off();
        State.clockIsRunning = false;
    }

    // No clock & loop - just start with default clock
    if ( !State.hasClock && State.loopPeriod==0 && State.TRIG_BUTTON==LOW ) {
        if ( !State.clockIsRunning ) {
            State.clockIsRunning = true;
            SetClockTicks( State.clockPeriod );
        }
    }

    // Use Loop timing
    if ( !State.hasClock && State.loopPeriod!=0 ) {
        if ( !State.clockIsRunning ) {
            State.clockIsRunning = true;
            SetClockTicks( State.loopPeriod );
        }
    }

    // Normal clock divider with CLK input
    if ( State.hasClock ) {
        if ( State.risingClock ) {
            // Start
            if ( !State.clockIsRunning ) {
                State.clockIsRunning = true;
                for (int i = 0; i < 4; ++i)
                {
                    State.ticks[i] = 0;
                }
            }
            else {
                for (int i = 0; i < 4; ++i)
                {
                    State.ticks[i] += 1;
                }
            }
            // Show
            if ( State.ticks[0] / ClockDivisions[0] == 1 ) { Outputs.Q.toggle(); State.ticks[0] = 0; }
            if ( State.ticks[1] / ClockDivisions[1] == 1 ) { Outputs.XOR.toggle(); State.ticks[1] = 0; }
            if ( State.ticks[2] / ClockDivisions[2] == 1 ) { Outputs.OR.toggle(); State.ticks[2] = 0; }
            if ( State.ticks[3] / ClockDivisions[3] == 1 ) { Outputs.AND.toggle(); State.ticks[3] = 0; }
        }
    }
}

void SetClockTicks(unsigned long period) {
    Outputs.Q.blink( period * ClockDivisions[0] );
    Outputs.XOR.blink( period * ClockDivisions[1] );
    Outputs.OR.blink( period * ClockDivisions[2] );
    Outputs.AND.blink( period * ClockDivisions[3] );
}


/**
 * Determine logic outputs
 */
void Logic() {
    bool AND = (State.A && State.B);
    bool OR = (State.A || State.B);
    bool XOR = (State.A != State.B);

    if ( State.mode == INVERSE || State.NOT == true ) {
        AND =  ! AND;
        OR  =  ! OR;
        XOR =  ! XOR;
    }

    if ( State.hasClock ) {
        AND = State.CLK && AND;
        OR  = State.CLK && OR;
        XOR = State.CLK && XOR;
    }

    Outputs.AND.set( AND );
    Outputs.OR.set( OR );
    Outputs.XOR.set( XOR );
}


/**
 * Determine FlipFlop output
 */
void FlipFlop() {
    if ( State.hasClock ) {
        if ( State.CLK == HIGH ) {
            if ( State.TRIG_BUTTON || State.A) {
                State.Q = true;
            }
            if ( State.RESET_BUTTON || State.B ) {
                State.Q = false;
            }
        }
    }
    else {
        if ( State.TRIG_BUTTON || State.A) {
            State.Q = true;
        }
        if ( State.RESET_BUTTON || State.B ) {
            State.Q = false;
        }
    }

    if ( State.mode == INVERSE || State.NOT == true ) {
        Outputs.Q.set( !State.Q );
    }
    else {
        Outputs.Q.set( State.Q );
    }
}

/**
 * Deterime Morphagene control
 */
void Morphagene() {
    if ( State.TRIG_BUTTON || (State.trigMode == EXTERNAL_TRIG && State.A) ) {
        Outputs.PLAY.off();
        if ( !State.hasClock ) {
            State.clockIsRunning = false;
        }
    }
    else {
        Outputs.PLAY.on();
    }

    if ( State.TRIG_BUTTON != State.prevTrig || (State.trigMode == EXTERNAL_TRIG && State.A != State.prevA )) {
        Outputs.REC.flash();
        State.prevTrig = State.TRIG_BUTTON;
    }
    State.prevA = State.A;
}



/**
 * Loop all outputs (for blinking etc)
 */
void loopOutputs() {
    // Outputs
    Outputs.AND.loop();
    Outputs.OR.loop();
    Outputs.XOR.loop();
    Outputs.Q.loop();
    Outputs.PLAY.loop();
    Outputs.REC.loop();
    // LEDS
    Outputs.mode.loop();
    Outputs.reset.loop();
    Outputs.trig.loop();
}

/**
 *
 * ==== MAIN LOOP ====
 *
 */

void loop() {

    setState();
    buttonLEDS();
    Morphagene();

    if (State.mode==CLOCKDIVIDER) {
        ClockDivider();
    }
    else {
        Logic();
        FlipFlop();
    }

    loopOutputs();
}
