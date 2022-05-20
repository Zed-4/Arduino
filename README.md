# Arduino

Objective:

To be able to use both Millis and interrupts to make a working digital clock using no delays and other functions that will cause input misses. Also, to add tilt wake up to the LCD with the blinking light intervolving every second. The clock is in 24-hour format of HH:MM:SS with green background and black font size of 6. Using two push buttons WIO_KEY_A, WIO_KEY_B, and accelerometer sensor to detect when screen should be turned on and off. The clock should properly roll over from seconds to minutes  and minutes to hours at 59 and for hours roll back to zero after 23. The clock should not roll when changing the time using the two buttons. Screens should time out after 5 seconds of no activity.

Procedure:

Running digital clock and blinking of the blue LED at each changing second.
Changing the time using push buttons A for minutes and B for hours.
Keeping the board idle to turn off the display’s backlight and making a tilt of 45 degrees x-axis to show the clock display again

Observation:

If the value of x-axis for the accelerometer is more than 0.39 (about 40 degrees) and less than 0.48 (about 50 degrees) the screen will turn on for 5 seconds.
This is an illustration of a Lab from CSCE 3612 Embedded system design to show things and features such as interrupts, not using delay function,
all the knowledge of LCD screen, and all the other sensors.

Test case #1:
      Showing the clock working properly on the given format and roll overs correctly.
https://drive.google.com/file/d/113wCRpT30gK7026BjxTucw6K-yTyun8X/view?usp=sharing

Test case #2:
      Showing the clock blue LED blinks at each second change and screen times out after 5 seconds.
https://drive.google.com/file/d/1LaT4DO0TwA1YOTQROcOLXJ0xTBUSdJLy/view?usp=sharing

Test case #3:
      Showing that the screen will wake up after screen turned only by 45 degrees within positive x-xis.
https://drive.google.com/file/d/1dnrTXSo65bJwNwLb2V_HRLg4538XyRIU/view?usp=sharing
