const bool DEBUG = true;

// OUTPUTS - LEDS
const int LED_BUTTON_INV      = 7;
const int LED_BUTTON_TRIG     = 6;
const int LED_CLK_DIV         = 5;
const int LED_LOGIC_AND       = 4;
const int LED_LOGIC_OR        = 3;
const int LED_LOGIC_XOR       = 2;

// OUTPUTS - JACKS
const int OUT_BUTTON_INV      = 9;
const int OUT_BUTTON_TRIG     = 8;
const int OUT_CLK_DIV         = 14;
const int OUT_LOGIC_AND       = 12;
const int OUT_LOGIC_OR        = 11;
const int OUT_LOGIC_XOR       = 10;

// INPUTS - UI
const int IN_BUTTON           = 15;
const int IN_CLK_DIV_POT      = A6;
const int IN_LOGIC_SWITCH     = 13;

// INPUTS - JACKS
const int IN_BUTTON_GATE      = 16;
const int IN_CLK_DIV          = 19;
const int IN_LOGIC_A          = 18;  // A4
const int IN_LOGIC_B          = 17;  // A3

const int DIVISIONS[] { 2, 3, 4, 5, 6, 7, 8, 16, 32 };

// const unsigned long MODE_SWITCH_LONG_PRESS_DURATION_MS = 3000; // Reset button long-press duration for trig/gate mode switch
const unsigned long BUTTON_DEBOUNCE_DELAY = 50; // Debounce delay for all buttons
const unsigned long LED_MIN_DURATION_MS = 50;   // Minimum "on" duration for all LEDs visibility


// ================================

#include "lib/Led.cpp"
#include "lib/Button.cpp"
#include "lib/Knob.cpp"

// OUTPUTS - LEDS
Led LedButtonInv;
Led LedButtonTrig;
Led LedClkDiv;
Led LedLogicAND;
Led LedLogicOR;
Led LedLogicXOR;

// OUTPUTS - JACKS
Led GateButtonInv;
Led GateButtonTrig;
Led GateClkDiv;
Led GateLogicAND;
Led GateLogicOR;
Led GateLogicXOR;

// INPUTS - UI
Button RecButton;
Button LogicSwitch;
Knob ClkDivision;

// INPUT - JACKS
Button InputRec;
Button InputClk;
Button LogicInA;
Button LogicInB;

bool clockInputState = false;
int ClockDivision = 0;
int clockCounter = 0;

bool recButtonState = false;

void setup() {

    if (DEBUG) {
        Serial.begin(9600);
    }

    // OUTPUTS - LEDS
    LedButtonInv.init( LED_BUTTON_INV, LED_MIN_DURATION_MS );
    LedButtonTrig.init( LED_BUTTON_TRIG, LED_MIN_DURATION_MS );
    LedClkDiv.init( LED_CLK_DIV, LED_MIN_DURATION_MS );
    LedLogicAND.init( LED_LOGIC_AND, LED_MIN_DURATION_MS );
    LedLogicOR.init( LED_LOGIC_OR, LED_MIN_DURATION_MS );
    LedLogicXOR.init( LED_LOGIC_XOR, LED_MIN_DURATION_MS );

    // OUTPUTS - Jacks
    GateButtonInv.init( OUT_BUTTON_INV, LED_MIN_DURATION_MS );
    GateButtonTrig.init( OUT_BUTTON_TRIG, LED_MIN_DURATION_MS );
    GateClkDiv.init( OUT_CLK_DIV, LED_MIN_DURATION_MS );
    GateLogicAND.init( OUT_LOGIC_AND, LED_MIN_DURATION_MS );
    GateLogicOR.init( OUT_LOGIC_OR, LED_MIN_DURATION_MS );
    GateLogicXOR.init( OUT_LOGIC_XOR, LED_MIN_DURATION_MS );

    // INPUTS - UI
    RecButton.init( IN_BUTTON, BUTTON_DEBOUNCE_DELAY, true );
    LogicSwitch.init( IN_LOGIC_SWITCH,BUTTON_DEBOUNCE_DELAY );
    ClkDivision.init( IN_CLK_DIV_POT );

    // INPUTS - JACKS
    InputRec.init( IN_BUTTON_GATE, BUTTON_DEBOUNCE_DELAY );
    InputClk.init( IN_CLK_DIV, BUTTON_DEBOUNCE_DELAY );
    LogicInA.init( IN_LOGIC_A, BUTTON_DEBOUNCE_DELAY );
    LogicInB.init( IN_LOGIC_B, BUTTON_DEBOUNCE_DELAY );

    // Clock divider
//    attachInterrupt( digitalPinToInterrupt(IN_CLK_DIV), clockInputChanged, CHANGE );
}


void loop() {

    // LEDS
    LedButtonInv.loop();
    LedButtonTrig.loop();
    LedClkDiv.loop();
    LedLogicAND.loop();
    LedLogicOR.loop();
    LedLogicXOR.loop();
    // GATES
    GateClkDiv.loop();

    // Morphagene - INV GATE
    bool buttonRead = RecButton.read() || InputRec.read();
//    if (DEBUG) {
//        Serial.print( "REC: "); Serial.print( InputRec.read() ); 
//    }
    if ( buttonRead ) {
        LedButtonInv.set(false);
        GateButtonInv.set(false);
    }
    else {
        LedButtonInv.set(true);
        GateButtonInv.set(true);
    }
    // Morphagene - TRIG on change
    if ( buttonRead!=recButtonState ) {
        LedButtonTrig.flash();
        GateButtonTrig.flash();
        recButtonState = buttonRead;
    }
    else {
        LedButtonTrig.off();
        GateButtonTrig.off();
    }

    // ClockDivider
    int clockPot = ClkDivision.readSwitch(-8);
    ClockDivision = DIVISIONS[clockPot];

    bool clockInput = InputClk.read();
    if (clockInput != clockInputState) {
      clockInputState = clockInput;
      if (clockInput==true) {
        Serial.println( " CLK Input changed ");
        clockCounter ++;
        if (clockCounter > (ClockDivision/2) ) {
          LedClkDiv.on();
          GateClkDiv.on();
        }
        if (clockCounter > ClockDivision) {
          clockCounter = 0;
          LedClkDiv.off();
          GateClkDiv.off();
        }
      }
    }
        
    if (DEBUG) {
        Serial.print( " CLK In: "); Serial.print( clockInput );
        Serial.print( " CLK Div: "); Serial.print( ClockDivision );
        Serial.print( " CLK Out: "); Serial.println( GateClkDiv.getState() );
    }


    // LOGIC
    bool LogicA = LogicInA.read();
    bool LogicB = LogicInB.read();
    bool LogicInv = LogicSwitch.read();
    
    bool AND  = (LogicA && LogicB);
    bool OR   = (LogicA || LogicB);
    bool XOR  = (LogicA == LogicB);

    if (LogicInv) {
      AND =  ! AND;
      OR  =  ! OR;
      XOR =  ! XOR;
    }
    
    LedLogicAND.set( AND );
    GateLogicAND.set( AND );
    LedLogicOR.set( OR );
    GateLogicOR.set( OR );
    LedLogicXOR.set( XOR );
    GateLogicXOR.set( XOR );

//    if (DEBUG) {
//        Serial.print(" Inv: "); Serial.println( LogicInv );
//        Serial.print(" A: "); Serial.print( LogicA );
//        Serial.print(" B: "); Serial.print( LogicB );
//        Serial.print(" AND "); Serial.print( AND );
//        Serial.print(" OR "); Serial.print( OR );
//        Serial.print(" XOR "); Serial.print( XOR );
//    }

}
