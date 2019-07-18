# LCDSim

LCDSim is a library written in C that allow you to emulate a HD44780 LCD 16x2 display on your computer. The display function is possible with the use of the SDL (with some extra lib that allow loading of image).

## Disclaimer

This library is using SDL1.2.

## Example

```c
// Sample from example.c, available in this repository
LCD_State(lcd, 1, 1, 1);    // Display ON, Cursor ON, Blink ON
LCD_SetCursor(lcd, 0, 3);   // Set the cursor position to 1st line, 4th column
LCD_PutS(lcd, "Hello,");    // Display the text "Hello,"
LCD_SetCursor(lcd, 1, 5);   // Set the cursor position to 2nd line, 6th column
LCD_PutS(lcd, "GitHub!");   // Display the text "GitHub!"
```
And there is what we get :

![result_win](https://image.noelshack.com/fichiers/2017/36/4/1504796313-output-yqaqe7.gif)
![result_ubu](https://image.noelshack.com/fichiers/2017/36/4/1504810251-output-u7utir.gif)

## Features

The library is fully emulating the HD44780, except for the reading-in and function set instruction, features that felt useless in our case.

## How to use it?

You will have to download the SDL 1.2 library (and LCDSim) :
- [SDL version 1.2.15](https://www.libsdl.org/download-1.2.php)

Once it's done, refer this library to your compiler and add lcdsim.h and lcdsim.c into your project. Don't forget to include LCDSim to your program by this way :

```c
#include "lcdsim.h"
```

## Wiki

- [CGROM organization](https://github.com/dylangageot/LCDSim/wiki/CGROM-organization)
- [Functions](https://github.com/dylangageot/LCDSim/wiki/Functions)

## Bibliography

- [LCD 16x2 image](http://paulvollmer.net/FritzingParts/parts/lcd-GDM1602K.html)
- [Instruction Set](https://mil.ufl.edu/3744/docs/lcdmanual/commands.html)
