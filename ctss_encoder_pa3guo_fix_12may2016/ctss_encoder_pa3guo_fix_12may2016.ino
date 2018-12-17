// -------------------------------------------------------
// CTCSS Encoder by Henk Hamoen, PA3GUO, The Netherlands.
// March 2016
//
// Credits CtcssTone libraries: Jan van Lienden PE1CID.
// -------------------------------------------------------

#include "CtcssToneId.h"
#include "CtcssTone.h"

// variables for the (morse code timing) of the flash LED
// -------------------------------------------------------
const int flash_start   = 100;
const int flash_dash    = 300;
const int flash_dot     = 100;
const int flash_spacing = 100;

// variables that define the inputs & outputs of the Arduino
// -------------------------------------------------------
const int toggle_switch =  10;
const int ptt_out = 9;
const int switch_1      =   9; // LSB
const int switch_2      =   8; // MSB (4 states)
const int switch_3      =   7; // MSB (8 states)
const int switch_8st    =   6; // '0' 4 states, '1' 8 states
const int led_a         =   5; // LSB
const int led_b         =   4; // MSB (4 states)
const int led_c         =   3; // MSB (8 states)
const int led_m         =   2; // morse code LED
const int led_L         =    LED_BUILTIN; // default

// variables that define the 4 (or 8) states of the finite state machine (FSM)
// -------------------------------------------------------
const int state_1       =   1; //  71,9 Hz region B: South NL
const int state_2       =   2; //  77,0 Hz region D: Mid & East NL
const int state_3       =   3; //  82,5 Hz region F: North NL
const int state_4       =   4; //  88,5 Hz region H: West NL
const int state_5       =   5; //  79,9 Hz region E: West-Vlaanderen en Oost-Vlaanderen Belgium
const int state_6       =   6; // 131,8 Hz region T: Antwerpen, Limburg, Vlaams-Brabant, Waals-Brabant en het Brussels gewest Belgium
const int state_7       =   7; // 110,0 Hz region O: Germany
const int state_8       =   8; // 123,0 Hz region R: Germany

// variously temporal values
// -------------------------------------------------------
int state;
int val_1;
int val_2;
int val_3;
int val_8st;
int val_t = 1;

// function that flashes the associated letter/character of a state in morse code (B, D, ...)
// -------------------------------------------------------
int send_morse_code(int statee){
  if (statee == state_1) 
    {delay(flash_start); // B: -...
     digitalWrite(led_m, HIGH);delay(flash_dash);digitalWrite(led_m, LOW);delay(flash_spacing);
     digitalWrite(led_m, HIGH);delay(flash_dot); digitalWrite(led_m, LOW);delay(flash_spacing);
     digitalWrite(led_m, HIGH);delay(flash_dot); digitalWrite(led_m, LOW);delay(flash_spacing);
     digitalWrite(led_m, HIGH);delay(flash_dot); digitalWrite(led_m, LOW);delay(flash_spacing);
    }
  else if (statee == state_2) 
    {delay(flash_start); // D: -..
     digitalWrite(led_m, HIGH);delay(flash_dash);digitalWrite(led_m, LOW);delay(flash_spacing);
     digitalWrite(led_m, HIGH);delay(flash_dot); digitalWrite(led_m, LOW);delay(flash_spacing);
     digitalWrite(led_m, HIGH);delay(flash_dot); digitalWrite(led_m, LOW);delay(flash_spacing);
    }
  else if (statee == state_3)
    {delay(flash_start); // F: ..-.
     digitalWrite(led_m, HIGH);delay(flash_dot); digitalWrite(led_m, LOW);delay(flash_spacing);
     digitalWrite(led_m, HIGH);delay(flash_dot); digitalWrite(led_m, LOW);delay(flash_spacing);
     digitalWrite(led_m, HIGH);delay(flash_dash);digitalWrite(led_m, LOW);delay(flash_spacing);
     digitalWrite(led_m, HIGH);delay(flash_dot); digitalWrite(led_m, LOW);delay(flash_spacing);
    }
  else if (statee == state_4)
    {delay(flash_start); // H: ....
     digitalWrite(led_m, HIGH);delay(flash_dot); digitalWrite(led_m, LOW);delay(flash_spacing);
     digitalWrite(led_m, HIGH);delay(flash_dot); digitalWrite(led_m, LOW);delay(flash_spacing);
     digitalWrite(led_m, HIGH);delay(flash_dot); digitalWrite(led_m, LOW);delay(flash_spacing);
     digitalWrite(led_m, HIGH);delay(flash_dot); digitalWrite(led_m, LOW);delay(flash_spacing);
    }
  else if (statee == state_5)
    {delay(flash_start); // E: .
     digitalWrite(led_m, HIGH);delay(flash_dot); digitalWrite(led_m, LOW);delay(flash_spacing);
     }
  else if (statee == state_6)
    {delay(flash_start); // T: -
     digitalWrite(led_m, HIGH);delay(flash_dash);digitalWrite(led_m, LOW);delay(flash_spacing);
     }
  else if (statee == state_7)
    {delay(flash_start); // O: --- 
     digitalWrite(led_m, HIGH);delay(flash_dash);digitalWrite(led_m, LOW);delay(flash_spacing);
     digitalWrite(led_m, HIGH);delay(flash_dash);digitalWrite(led_m, LOW);delay(flash_spacing);
     digitalWrite(led_m, HIGH);delay(flash_dash);digitalWrite(led_m, LOW);delay(flash_spacing);
     }
  else if (statee == state_8)
    {delay(flash_start); // R: .-.
     digitalWrite(led_m, HIGH);delay(flash_dot); digitalWrite(led_m, LOW);delay(flash_spacing);
     digitalWrite(led_m, HIGH);delay(flash_dash);digitalWrite(led_m, LOW);delay(flash_spacing);
     digitalWrite(led_m, HIGH);delay(flash_dot); digitalWrite(led_m, LOW);delay(flash_spacing);
    }
}

// the setup routine that runs only once (at start up)
// -------------------------------------------------------

void setup() 
{                
  // configure the inputs and outputs
  // -------------------------------------
  pinMode(led_a, OUTPUT);  
  pinMode(led_b, OUTPUT);  
  pinMode(led_c, OUTPUT);  
  pinMode(led_m, OUTPUT); 
  pinMode(led_L, OUTPUT); 

  pinMode(toggle_switch, INPUT_PULLUP);
  pinMode(ptt_out, OUTPUT);
  //pinMode(switch_1,      INPUT_PULLUP);
  pinMode(switch_2,      INPUT_PULLUP);
  pinMode(switch_3,      INPUT_PULLUP);
  pinMode(switch_8st,    INPUT_PULLUP);

  // read start-up state from the switches
  // -------------------------------------
  val_1   = digitalRead(switch_1); 
  val_2   = digitalRead(switch_2);
  val_3   = digitalRead(switch_3); 
  val_8st = digitalRead(switch_8st); 
  
  if (val_8st==0)  // 4 states mode
  {
    if      (val_2 == 0 && val_1 == 0)              {state = state_1;}
    else if (val_2 == 0 && val_1 == 1)              {state = state_2;}
    else if (val_2 == 1 && val_1 == 0)              {state = state_3;}
    else                                            {state = state_4;}
  }
  else if (val_8st==1)  // 8 states mode
  {
    if      (val_3 == 0 && val_2 == 0 && val_1 == 0) {state = state_1;} 
    else if (val_3 == 0 && val_2 == 0 && val_1 == 1) {state = state_2;}
    else if (val_3 == 0 && val_2 == 1 && val_1 == 0) {state = state_3;} // fixed
    else if (val_3 == 0 && val_2 == 1 && val_1 == 1) {state = state_4;}
    else if (val_3 == 1 && val_2 == 0 && val_1 == 0) {state = state_5;}
    else if (val_3 == 1 && val_2 == 0 && val_1 == 1) {state = state_6;}
    else if (val_3 == 1 && val_2 == 1 && val_1 == 0) {state = state_7;} // fixed
    else                                             {state = state_8;}
  }
  
  send_morse_code(state);
  
  // initialise the CTCSS tone generation 
  // -------------------------------------
  CtcssTone.init();

}

// main program loop
// -------------------------------------

void loop() {
  val_t   = digitalRead(toggle_switch); 
  if (val_t == 0 ) {
    digitalWrite(led_L, HIGH);
    digitalWrite(ptt_out, LOW);
    CtcssTone.tone_on(tone_ctcss_R);
  } else {
    digitalWrite(led_L, LOW);
    digitalWrite(ptt_out, HIGH);
    CtcssTone.tone_off();
  }

}

void ORIGINALloop() 
{

  //configure outputs depending on current state
  // -------------------------------------------
  if      (state == state_1) 
    {
    digitalWrite(led_c, LOW);  digitalWrite(led_b, LOW); digitalWrite(led_a, LOW); // (a = LSB)
    CtcssTone.tone_on(tone_ctcss_B);
    }
  else if (state == state_2) 
    {
    digitalWrite(led_c, LOW); digitalWrite(led_b, LOW); digitalWrite(led_a, HIGH); // (a = LSB)
    CtcssTone.tone_on(tone_ctcss_D);
    }
  else if (state == state_3) 
    {
    digitalWrite(led_c, LOW);  digitalWrite(led_b, HIGH); digitalWrite(led_a, LOW); // (a = LSB)
    CtcssTone.tone_on(tone_ctcss_F);
    }
  else if (state == state_4) 
    {
    digitalWrite(led_c, LOW); digitalWrite(led_b, HIGH); digitalWrite(led_a, HIGH); // (a = LSB)
    CtcssTone.tone_on( tone_ctcss_H );
    }
  else if (state == state_5) 
    {
    digitalWrite(led_c, HIGH); digitalWrite(led_b, LOW); digitalWrite(led_a, LOW); // (a = LSB)
    CtcssTone.tone_on(tone_ctcss_E);
    }
  else if (state == state_6) 
    {
    digitalWrite(led_c, HIGH);  digitalWrite(led_b, LOW); digitalWrite(led_a, HIGH); // (a = LSB)
    CtcssTone.tone_on(tone_ctcss_T);
    }
  else if (state == state_7) 
    {
    digitalWrite(led_c, HIGH); digitalWrite(led_b, HIGH); digitalWrite(led_a, LOW); // (a = LSB)
    CtcssTone.tone_on( tone_ctcss_O );
    }
  else if (state == state_8) 
    {
    digitalWrite(led_c, HIGH); digitalWrite(led_b, HIGH); digitalWrite(led_a, HIGH); // (a = LSB)
    CtcssTone.tone_on( tone_ctcss_R );
    }
  
   // delay inbetween toggling from one state to the next
   // --------------------------------------
   if (val_t == 0) {delay(300);send_morse_code(state);val_t = 1;}               
   
   // check if the toggle switch is pushed to go to the next state
   // ---------------------------------------------
   val_t   = digitalRead(toggle_switch); 
   val_8st = digitalRead(switch_8st);
       
       if (val_t == 0) // toggle switch pushed
         {
         if (val_8st == 0) // 4 state mode
           {
                 if (state == state_1) {state = state_2;}
            else if (state == state_2) {state = state_3;}
            else if (state == state_3) {state = state_4;}
            else                       {state = state_1;}
           }
         else if (val_8st == 1) // 8 state mode
           {
                 if (state == state_1) {state = state_2;}
            else if (state == state_2) {state = state_3;}
            else if (state == state_3) {state = state_4;}
            else if (state == state_4) {state = state_5;}
            else if (state == state_5) {state = state_6;}
            else if (state == state_6) {state = state_7;}
            else if (state == state_7) {state = state_8;}
            else                       {state = state_1;}
           }
           
        }
   }

