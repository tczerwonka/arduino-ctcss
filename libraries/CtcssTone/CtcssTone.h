/*
 * Include file to control the generation of CTCSS tones using timer 2.
 * The output of the timer 2 is at pin D11.
 *
 * Copyright: 2010 PE1CID
 * Version:
 * 1.0 06-01-2016 Initial version based on T813Tone.h
 */

#ifndef CTCSS_TONE_H
#define CTCSS_TONE_H

/* Include files */

#include "CtcssToneId.h"

/* Definitions */

typedef enum
{
  tone_state_off,
  tone_state_on
} Tone_State;

class CtcssToneClass
{
  private:
    // Private variables
    static const uint32_t tone_defs [TONE_MAX_TONES];
    static Tone_Id        current_tone;
    static Tone_State     tone_state;
    // Private methods
    void Setup_timer2( void );
  public:
    CtcssToneClass();
    // Public methods
    void init ( void );
    void tone_on ( Tone_Id tone_id );
    void set_tone ( Tone_Id tone_id );
    void prev_tone_on ( void );
    void tone_off ( void );
    void toggle_tone ( void );
    Tone_Id get_tone ( void );
    bool get_tone_on ( void );
};

extern CtcssToneClass CtcssTone;

#endif /* CTCSS_TONE_H */

