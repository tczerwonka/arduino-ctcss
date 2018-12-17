/*
 * Driver to control the generation of CTCSS tones using Timer 2.
 * The Timer 2 is used in PWM mode and the output is at pin D11.
 *
 * Copyright: 2010 PE1CID
 * Version:
 * 1.0 06-01-2016 Initial version based on T813Tone.cpp
 */

/* Include files */
#include "arduino.h"
#include "CtcssToneId.h"
#include "CtcssTone.h"

/* Defintions */

#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))

// table of 256 sine values / one sine period / stored in flash memory
PROGMEM const unsigned char sine256[] =
{
  127, 130, 133, 136, 139, 143, 146, 149, 152, 155, 158, 161, 164, 167, 170, 173,
  176, 178, 181, 184, 187, 190, 192, 195, 198, 200, 203, 205, 208, 210, 212, 215,
  217, 219, 221, 223, 225, 227, 229, 231, 233, 234, 236, 238, 239, 240, 242, 243,
  244, 245, 247, 248, 249, 249, 250, 251, 252, 252, 253, 253, 253, 254, 254, 254,
  254, 254, 254, 254, 253, 253, 253, 252, 252, 251, 250, 249, 249, 248, 247, 245,
  244, 243, 242, 240, 239, 238, 236, 234, 233, 231, 229, 227, 225, 223, 221, 219,
  217, 215, 212, 210, 208, 205, 203, 200, 198, 195, 192, 190, 187, 184, 181, 178,
  176, 173, 170, 167, 164, 161, 158, 155, 152, 149, 146, 143, 139, 136, 133, 130,
  127, 124, 121, 118, 115, 111, 108, 105, 102,  99,  96,  93,  90,  87,  84,  81,
   78,  76,  73,  70,  67,  64,  62,  59,  56,  54,  51,  49,  46,  44,  42,  39,
   37,  35,  33,  31,  29,  27,  25,  23,  21,  20,  18,  16,  15,  14,  12,  11,
   10,   9,   7,   6,   5,   5,   4,   3,   2,   2,   1,   1,   1,   0,   0,   0,
    0,   0,   0,   0,   1,   1,   1,   2,   2,   3,   4,   5,   5,   6,   7,   9,
   10,  11,  12,  14,  15,  16,  18,  20,  21,  23,  25,  27,  29,  31,  33,  35,
   37,  39,  42,  44,  46,  49,  51,  54,  56,  59,  62,  64,  67,  70,  73,  76,
   78,  81,  84,  87,  90,  93,  96,  99, 102, 105, 108, 111, 115, 118, 121, 124
};

const uint32_t CtcssToneClass::tone_defs [TONE_MAX_TONES] =
{
         0, /* no_tone */
   9172440, /*  A */
   9843260, /*  B */
  10185515, /*  C */
  10541460, /*  D */
  10911096, /*  E */
  11294422, /*  F */
  11691438, /*  G */
  12115834, /*  H */
  12526541, /*  I */
  12978317, /*  J */
  13334263, /*  K */
  13690208, /*  L */
  14169366, /*  M */
  14675903, /*  N */
  15182441, /*  O */
  15716359, /*  P */
  16263967, /*  Q */
  16838956, /*  R */
  17427635, /*  S */
  18043694, /*  T */
  18687134, /*  U */
  19344264, /*  V */
  20015084, /*  W */
  20726975, /*  X */
  21452556, /*  Y */
  22205518, /*  Z */
  22985860, /* AA */
  23793582, /* AB */
  24628685, /* AC */
  25491168, /* AD */
  26394722, /* AE */
  27859574, /* AF */
  28845269, /* AG */
  29858344, /* AH */
  30898800, /* AI */
  31980326, /* AJ */
  33102924, /* AK */
  34266591, /* AL */
};

/* Static variables */

Tone_Id    CtcssToneClass::current_tone    = tone_no_tone;
Tone_State CtcssToneClass::tone_state      = tone_state_off;

// variables used inside interrupt service declared as voilatile
volatile uint8_t  icnt;            // ISR variable
volatile uint32_t phaccu;          // phase accumulator
volatile uint32_t tword_m;         // dds tuning word m

// Constructors ////////////////////////////////////////////////////////////////

CtcssToneClass::CtcssToneClass()
{
}

// Private Methods //////////////////////////////////////////////////////////////

/*
 * Setup_timer2
 * Function to setup timer2 for tone generation
 *    set prescaler to 1,
 *    PWM mode to phase correct PWM,
 *    16000000/510 = 31372.55 Hz clock.
 * in: none
 * out: none
 * return: none
 */
void CtcssToneClass::Setup_timer2( void )
{
  // Timer2 Clock Prescaler to : 1
  sbi (TCCR2B, CS20);
  cbi (TCCR2B, CS21);
  cbi (TCCR2B, CS22);

  // Timer2 ouput A PWM Mode set to Phase Correct PWM
  cbi (TCCR2A, COM2A0);  // clear Compare Match
  sbi (TCCR2A, COM2A1);

  sbi (TCCR2A, WGM20);  // Mode 1  / Phase Correct PWM
  cbi (TCCR2A, WGM21);
  cbi (TCCR2B, WGM22);
}

// Interrupt Service Routine //////////////////////////////////////////////////////////////

//******************************************************************
/*
 * Interrupt Service Routine for Timer2
 * Timer2 Interrupt Service at 31372,550 kHz = 32uSec
 *   This is the timebase REFCLOCK for the DDS generator
 *   F_OUT = (M (REFCLK)) / (2 exp 32)
 *   runtime: 8 microseconds ( inclusive push and pop)
 */
ISR( TIMER2_OVF_vect )
{
  sbi( PORTB, 4 );           // Test / set PORTB,4 (Pin D12) high to observe timing with a oscope

  phaccu = phaccu + tword_m; // soft DDS, phase accu with 32 bits
  icnt = phaccu >> 24;       // use upper 8 bits for phase accu as frequency information
                             // read value from ROM sine table and send to PWM DAC
  OCR2A = pgm_read_byte_near( sine256 + icnt );

  cbi( PORTB, 4 );            // reset PORTB,4 (Pin D12)
}

// Public Methods //////////////////////////////////////////////////////////////

/*
 * init
 * Function to initialise the generation of a tones
 * in: none
 * out: none
 * return: none
 */
void CtcssToneClass::init ( void )
{
  current_tone    = tone_no_tone;
  tone_state      = tone_state_off;

  pinMode( 11, OUTPUT);      // pin11 = PWM output / frequency output (PORTB 3), CTCSS
  pinMode( 12, OUTPUT);      // pin12 = test output in ISR (PORTB 4)

  // Initialise timer 2
  CtcssTone.Setup_timer2();
  tword_m = tone_defs[0];
}

/*
 * tone_on
 * Function to select and start the generation of a tone
 * in: tone_id
 * out: none
 * return: none
 */
void CtcssToneClass::tone_on ( Tone_Id tone_id )
{
  // Switch off current tone
  cbi ( TIMSK2, TOIE2 );              // disable Timer2 Interrupt

  if ( tone_id <= tone_ctcss_AL )
  {
    current_tone = tone_id;
    tone_state   = tone_state_on;

    // Enter the tone values
    tword_m = tone_defs[tone_id];

    // Start the tone by enabling interrupt timer3
    sbi ( TIMSK2, TOIE2 );              // enable Timer3 Interrupt
  }
  else
  {
    // No tone
    current_tone = tone_no_tone;
    tone_state   = tone_state_off;
    tword_m      = tone_defs[0];
  }
}

/*
 * set_tone
 * Function to select the generation of a tone.
 *   If the tone is on or off is not changed.
 * in: tone_id
 * out: none
 * return: none
 */
void CtcssToneClass::set_tone ( Tone_Id tone_id )
{
  if ( tone_state_on == tone_state )
  {
    tone_on ( tone_id );
  }
  else
  {
    current_tone = tone_id;
  }
}

/*
 * prev_tone_on
 * Function to start the generation of the tone last used.
 * in: none
 * out: none
 * return: none
 */
void CtcssToneClass::prev_tone_on ( void )
{
  tone_on ( current_tone );
}

/*
 * tone_off
 * Function to stop the generation of a tone
 * in: none
 * out: none
 * return: none
 */
void CtcssToneClass::tone_off ( void )
{
  // Stop the tone 
  cbi ( TIMSK2, TOIE2 );              // disable Timer2 Interrupt
  tone_state = tone_state_off;
}

/*
 * toggle_tone
 * Function to toggle a tone
 * in: none
 * out: none
 * return: none
 */
void CtcssToneClass::toggle_tone ( void )
{
  switch ( tone_state )
  {
    case tone_state_off:
      prev_tone_on ();
      break;
    case tone_state_on:
      tone_off ();
      break;
    default:
      tone_off ();
      break;
  }
}

/*
 * get_tone
 * Function to return the current generated tone
 * in: none
 * out: none
 * return: tone_id
 */
Tone_Id CtcssToneClass::get_tone ( void )
{
  Tone_Id tone = tone_no_tone;

  switch ( tone_state )
  {
    case tone_state_off:
      tone = tone_no_tone;
      break;
    case tone_state_on:
      tone = current_tone;
      break;
    default:
      tone = tone_no_tone;
      break;
  }
  return ( tone );
}

/*
 * get_tone_on
 * Function to return if a tone is being generated
 * in: none
 * out: none
 * return: tone_on
 */
bool CtcssToneClass::get_tone_on ( void )
{
  return ( tone_state == tone_state_on );
}

// Preinstantiate Objects //////////////////////////////////////////////////////

CtcssToneClass CtcssTone;

/* End of file CtcssTone.cpp */
