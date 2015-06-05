LEDWall
=======

A giant wall fo WS2811 LEDs, made to display video.  Once this code is installed
the lights can be controlled by playing a video, much easier than "real" coding.

Teensy
======

modified version of OctoWS2811 scripts.

These scripts sit on the Teensy.  Most of the scripts test that the lights work as expected.

videoDisplay controls the LEDs based on instructions arriving through the serial port.

Processing
==========

Modified version of movie2serial (http://www.pjrc.com/teensy/td_libs_OctoWS2811.html).
Reads a movie file and streams it to a serial device.