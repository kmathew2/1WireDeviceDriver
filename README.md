# 1WireDeviceDriver
Driving the LED strip by writing a device driver in Linux to illuminate in the required pattern which is input as hexadecimal values for each of the 16 LED lights. Each LED is driven by a WS2812 driver which works by the one wire communication. At first an SPI driver was used to test the strip and the 1-wire communication. Then taking into account the delays associated for the pulse to work as a 1-wire communication, some delays were added and the LED strip was driven using the GPIO pins.
Sample execution can be found at 
https://www.youtube.com/watch?v=JswCqg8SajA
