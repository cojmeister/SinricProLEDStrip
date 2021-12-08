# Code For SinricPro LED LightStrip
This is a platform io code in C++ for using my LED W2812B Strip with SinricPro and thus connecting it to my google home.

It has some minor changes from the example found here, as in adding the ability to set the white color temperature.

This is meant to compile to an ESP32 using platformIO, and your personal settings are taken from the sinricPro website: 
    https://portal.sinric.pro/dashboard

The code also uses the SinricPro, and FastLED dependencies from platformIO.

My strip works in GRB for some reason, so if you see different colors than what you are meant to, then switch it back to RGB.

Original Examples on:
    https://github.com/sinricpro/esp8266-esp32-sdk/tree/master/examples