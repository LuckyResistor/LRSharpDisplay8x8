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


// This is a demonstration of the driver features.


#include <LRSharpDisplay.h>

#include "Fonts.h"


// Create a global instance of the display driver.
lr::SharpDisplay sharpDisplay(11, 9, 10);


void setup() {
  sharpDisplay.begin();
  sharpDisplay.setFont(Fonts::a);
}


void loop() {
  // Write the welcome message.
  sharpDisplay.clear();
  sharpDisplay.setCharacter(0, 0, 'a');
  sharpDisplay.writeText(F("Welcome!\nThis is the text oriented Sharp display driver.\n\n"));
  sharpDisplay.refresh();
  delay(1000);
  
  // Display some values.
  for (uint8_t i = 1; i < 30; ++i) {
    const uint8_t value = analogRead(7);
    sharpDisplay.writeText(F("V "));
    sharpDisplay.writeText(String(i));
    sharpDisplay.writeText(F(": "));
    sharpDisplay.writeText(String(value));
    sharpDisplay.writeCharacter('\n');
    sharpDisplay.refresh();
    delay(200);
  }
  delay(500);
  
  // Scroll a portion of the text around.
  sharpDisplay.clear();
  sharpDisplay.writeText(F("\x88\x89\x89\x8a\n\x8b\x8f \x8b\n\x8b \x8f\x8b\n\x98\x89\x89\x99"));
  sharpDisplay.refresh();
  delay(500);
  lr::SharpDisplay::ScrollDirection sd[] = {lr::SharpDisplay::ScrollRight, lr::SharpDisplay::ScrollDown, lr::SharpDisplay::ScrollLeft, lr::SharpDisplay::ScrollUp};
  for (uint8_t i = 0; i < 4; ++i) {
    for (uint8_t j = 0; j < 4; ++j) {
      for (uint8_t k = 0; k < 8; ++k) {
        sharpDisplay.scrollScreen(sd[j]);
        sharpDisplay.refresh();
        delay(30);
      }
    }
  }
  for (uint8_t k = 0; k < 12; ++k) {
    sharpDisplay.scrollScreen(lr::SharpDisplay::ScrollDown);
    sharpDisplay.refresh();
    delay(30);
  }
  
  // Inversion of characters.
  sharpDisplay.clear();
  for (uint8_t i = 0; i < 4; ++i) {
    for (uint8_t j = 0; j < i; ++j) {
      sharpDisplay.writeCharacter(' ');
    }
    sharpDisplay.writeText(F("Normal\n"));
    for (uint8_t j = 0; j < i; ++j) {
      sharpDisplay.writeCharacter(' ');
    }
    sharpDisplay.setTextInverse(true);
    sharpDisplay.writeText(F("Inverse\n"));
    sharpDisplay.setTextInverse(false);
    sharpDisplay.refresh();
  }
  delay(2000);
  
  // Demonstrate the fast refresh speed.
  sharpDisplay.clear();
  sharpDisplay.writeText(F("Minimal\n  Display\n    Refresh\n= Less Power\n= Faster\n"));
  unsigned long t1 = millis();
  for (int i = 0; i < 1024; ++i) {
    String s = String(i);
    s += F(" > ");
    s += analogRead(7);
    sharpDisplay.setLineText(8, s);
    sharpDisplay.refresh();
  }
  unsigned long t2 = millis();
  sharpDisplay.setCursorPosition(9, 0);
  sharpDisplay.writeText(F("1024 updates\n= "));
  sharpDisplay.writeText(String(t2-t1));
  sharpDisplay.writeText(F("ms"));
  sharpDisplay.refresh();
  delay(2000);
  
  
  // Display all characters and rotate them.
  uint8_t offset = 0;
  for (int l = 0; l < 50; l++) {
    uint8_t character = offset;
    for (uint8_t row = 0; row < sharpDisplay.getScreenHeight(); ++row) {
      for (uint8_t column = 0; column < sharpDisplay.getScreenWidth(); ++column) {
        sharpDisplay.setCharacter(row, column, character);
        ++character;
      }
    }
    sharpDisplay.refresh();
    ++offset;
    delay(100);
  }
  
  
  // Character scroll, scroll large characters upwards.
  sharpDisplay.clear();
  for (uint16_t i = 0; i < 768; ++i) {
    for (uint8_t y = 0; y < sharpDisplay.getScreenHeight(); ++y) {
      uint8_t byte = pgm_read_byte(Fonts::a+i+y);
      for (uint8_t x = 0; x < 8; ++x) {
        if (byte&0x80) {
          sharpDisplay.setCharacter(y, x+2, 0x20);
        } else {
          sharpDisplay.setCharacter(y, x+2, 0x8e);
        }
        byte <<= 1;
      }
    }
    sharpDisplay.refresh();
    delay(10);
  }
}



