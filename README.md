# arduino-ctcss
Arduino CTCSS generator -- my mods using PA3GUO's code

See:
http://www.pa3guo.com/

http://www.pa3guo.com/PA3GUO_Arduino_CTCSS_v1.2.pdf

for more information

## description

I used this code with an Arduino Nano to generate a continuous low-frequency
(123Hz), low-level tone.  These tones are often called PL ("Private Line" -- a
Motorola-ism and probably even trademarked for all I know) or CTCSS (Continuous
Tone-Coded Squelch System) tones.  These are injected at a low level in addition
to audio in a radio system to open squelch or provide access to a system.

With my DMR project with the W9YT repeater ( http://pages.cs.wisc.edu/~timc/e/w9yt-dmr/ )
I needed to add a 123Hz CTCSS tone to an audio stream.  I did not have parts sufficient
to do the same with a 555 timer chip or similar.  So -- inexpensive microcontroller to
the rescue.

This project by PA3GUO has additional features but I modified the code to generate only the 123Hz tone.

Additionally minor logic has been added to only generate the tone when transmit is
enabled and to further duplicate this signal on another pin.
