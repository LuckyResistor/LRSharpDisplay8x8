//
// Lucky Resistor's Sharp Display Driver 8x8 Pixel
// ---------------------------------------------------------------------------
// (c)2015 by Lucky Resistor. See LICENSE for details.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//


// Include the header file of the driver.
#include <LRSharpDisplay.h>

// Include the header file of the font.
#include "Fonts.h"


// Create a global instance of the display driver.
//
// Add the chip select, clock and data pins which are connected to the
// display as arguments to the constructor in this order. In this 
// example, we use pin 11 for chip select, pin 9 for the clock and
// pin 10 for the data in.
//
lr::SharpDisplay sharpDisplay(11, 9, 10);


void setup() {
  // In the setup method, you have to call begin() to initialize the
  // library and setup communication.
  sharpDisplay.begin();

  // A font is required, you can use the example font for your own
  // project. Just add the "Fonts.h" and "Fonts.cpp" to your project.
  sharpDisplay.setFont(Fonts::a);

  // The display is automatically cleared at the start, so you can 
  // start writing text to the display:
  sharpDisplay.writeText(F("Hello World!"));

  // The text is _not_ visible at this point, you have to call
  // refresh() to make the current text buffer visible on the display.

  // Make the text visible on the display.
  sharpDisplay.refresh();
}


void loop() {
}



