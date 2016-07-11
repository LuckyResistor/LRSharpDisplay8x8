//
// Lucky Resistor's Sharp Display Driver 8x8 Pixel
// ---------------------------------------------------------------------------
// (c)2016 by Lucky Resistor. See LICENSE for details.
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


#include "LRSharpDisplay.h"


namespace lr {


// An anonymous namespace to prevent name conflicts.
namespace {

    
// Use a number of global variables for time critical parts
// The following variables are used for the communication.
static uint8_t gChipSelectPin;
static uint8_t gClockPin;
static volatile uint8_t *gClockPort;
static uint8_t gClockMask;
static uint8_t gDataPin;
static volatile uint8_t *gDataPort;
static uint8_t gDataMask;
   
    
// This constants are bitmasks for the commands
static const uint8_t CMD_WRITE = 0x80;
static const uint8_t CMD_CLEAR = 0x20;
static const uint8_t CMD_VCOM = 0x40;
    
    
// This flag is used to toggle the required VCOM bit.
static uint8_t gVComBit;

    
// The array with the 15x25 screen.
static const uint8_t gScreenHeight = 15;
static const uint8_t gScreenWidth = 25;
static uint8_t gScreenCharacters[gScreenHeight*gScreenWidth];
static const uint8_t gScreenRowRequiresUpdateSize = gScreenHeight/8+1;
static uint8_t gScreenRowRequiresUpdate[gScreenRowRequiresUpdateSize];
    
// The height of a single character.
static const uint8_t gCharacterHeight = 8;

// A empty font, to prevent any crashes if no font is set.
static const uint8_t gNoFont[8] PROGMEM = {0, 0, 0, 0, 0, 0, 0, 0};

// The current font for the display
static const uint8_t *gTextFont;
    
// The current text flags
static const uint8_t gTextFlagInverse = 0x80;
static const uint8_t gTextCharacterMask = 0x7f;
static uint8_t gTextFlags;
    
// The cursor position
static uint8_t gCursorX;
static uint8_t gCursorY;
    
    
// Set the clock low
inline static void setClockLow()
{
    *gClockPort &= ~gClockMask;
}

    
// Set the clock high
inline static void setClockHigh()
{
    *gClockPort |= gClockMask;
}

    
// Set the data low
inline static void setDataLow()
{
    *gDataPort &= ~gDataMask;
}
    
    
// Set the data high
inline static void setDataHigh()
{
    *gDataPort |= gDataMask;
}

    
// This method sends a single byte MSB first to the display
static void sendByteMSB(uint8_t byte)
{
    for (uint8_t i = 0; i < 8; ++i) {
        setClockLow();
        if (byte & 0x80) {
            setDataHigh();
        } else {
            setDataLow();
        }
        setClockHigh();
        byte <<= 1;
    }
    setClockLow();
}


// This method sends a single byte MSB first to the display
static void sendByteMSBx2(uint8_t byte)
{
    for (uint8_t i = 0; i < 8; ++i) {
        setClockLow();
        if (byte & 0x80) {
            setDataHigh();
        } else {
            setDataLow();
        }
        setClockHigh();
        setClockLow();
        if (byte & 0x80) {
            setDataHigh();
        } else {
            setDataLow();
        }
        setClockHigh();
        byte <<= 1;
    }
    setClockLow();
}

    
// This method sends a single byte LSB first to the display
static void sendByteLSB(uint8_t byte)
{
    for (uint8_t i = 0; i < 8; ++i) {
        setClockLow();
        if (byte & 0x01) {
            setDataHigh();
        } else {
            setDataLow();
        }
        setClockHigh();
        byte >>= 1;
    }
    setClockLow();
}

    
// This method toggles the VCOM bit
inline static void toggleVComBit()
{
    gVComBit ^= CMD_VCOM;
}

    
// This method calculates a pointer to a character on the screen
inline uint8_t* getCharacterPosition(uint8_t row, uint8_t column)
{
    return &gScreenCharacters[(row*gScreenWidth)+column];
}
    

// Clear the whole screen.
inline void clearScreen()
{
    memset(gScreenCharacters, 0x00, gScreenWidth*gScreenHeight);
}
    

// Check if a given row is maked for updates
inline bool isRowMarkedForUpdate(uint8_t row)
{
    return ((gScreenRowRequiresUpdate[row>>3] & (1<<(row&7))) != 0);
}
    
    
// Mark a given row for an update
inline void markRowForUpdate(uint8_t row)
{
    gScreenRowRequiresUpdate[row>>3] |= (1<<(row&7));
}

    
// Mark the whole screen for an update.
inline void markScreenForUpdate()
{
    memset(gScreenRowRequiresUpdate, 0xff, gScreenRowRequiresUpdateSize);
}

 
// Remove any row update requests from the screen.
inline void clearScreenUpdate()
{
    memset(gScreenRowRequiresUpdate, 0x00, gScreenRowRequiresUpdateSize);
}

    
}


SharpDisplay::SharpDisplay(uint8_t chipSelectPin, uint8_t clockPin, uint8_t dataPin)
{
    // Prepare the values for the SPI communication
    gChipSelectPin = chipSelectPin;
    gClockPin = clockPin;
    gClockPort = portOutputRegister(digitalPinToPort(clockPin));
    gClockMask = digitalPinToBitMask(clockPin);
    gDataPin = dataPin;
    gDataPort = portOutputRegister(digitalPinToPort(dataPin));
    gDataMask = digitalPinToBitMask(dataPin);
    
    // Initialize the screen memory
    clearScreen();
    clearScreenUpdate();
    gTextFlags = 0;
    gTextFont = gNoFont;
    gCursorX = 0;
    gCursorY = 0;
}

    
SharpDisplay::~SharpDisplay()
{
}

    
void SharpDisplay::begin()
{
    // Prepare all communication ports
    digitalWrite(gChipSelectPin, HIGH);
    digitalWrite(gClockPin, LOW);
    digitalWrite(gDataPin, HIGH);
    pinMode(gChipSelectPin, OUTPUT);
    pinMode(gClockPin, OUTPUT);
    pinMode(gDataPin, OUTPUT);
    // Set the VCOM bit
    gVComBit = CMD_VCOM;
    // Clear the display.
    clear();
}

    
void SharpDisplay::setFont(const uint8_t *fontData)
{
    gTextFont = fontData;
    markScreenForUpdate();
}

    
void SharpDisplay::clear()
{
    // Send the clear command.
    digitalWrite(gChipSelectPin, HIGH);
    sendByteMSB(CMD_CLEAR|gVComBit);
    sendByteMSB(0);
    toggleVComBit();
    digitalWrite(gChipSelectPin, LOW);

    // Clear the screen.
    clearScreen();
    clearScreenUpdate();
    gCursorX = 0;
    gCursorY = 0;
}
    

void SharpDisplay::refresh()
{
    // Send the write command.
    digitalWrite(gChipSelectPin, HIGH);
    sendByteMSB(CMD_WRITE|gVComBit);
    toggleVComBit();
    // Update all rows which need a refresh.
    for (uint8_t row = 0; row < gScreenHeight; ++row) {
        if (isRowMarkedForUpdate(row)) {
            // Draw the row pixel row by pixel row
            for (uint8_t pixelRow = 0; pixelRow < gCharacterHeight; ++pixelRow) {
                for (uint8_t repeat = 0; repeat < 2; ++repeat) {
                    sendByteLSB(row*gCharacterHeight*2+pixelRow*2+repeat+1);
                    for (uint8_t column = 0; column < gScreenWidth; ++column) {
                        const uint8_t screenData = *(getCharacterPosition(row, column));
                        const uint8_t characterIndex = (screenData & gTextCharacterMask);
                        uint16_t characterStart = characterIndex;
                        characterStart *= gCharacterHeight;
                        characterStart += pixelRow;
                        uint8_t pixelMask = pgm_read_byte(gTextFont+characterStart);
                        if ((screenData & gTextFlagInverse) != 0) { // Inverse character?
                            pixelMask = ~pixelMask;
                        }
                        sendByteMSBx2(pixelMask);
                    }
                    sendByteLSB(0x00);
                }
            }
        }
    }
    sendByteLSB(0x00);
    digitalWrite(gChipSelectPin, LOW);
    clearScreenUpdate();
}

    
void SharpDisplay::setTextInverse(bool enable)
{
    if (enable) {
        gTextFlags |= gTextFlagInverse;
    } else {
        gTextFlags &= ~gTextFlagInverse;
    }
}

    
void SharpDisplay::setCharacter(uint8_t row, uint8_t column, uint8_t character)
{
    if (row < gScreenWidth && column < gScreenHeight) {
        uint8_t* const cp = getCharacterPosition(row, column);
        *cp = ((static_cast<uint8_t>(character)-0x20) & gTextCharacterMask) | gTextFlags;
        markRowForUpdate(row);
    }
}
   
    
char SharpDisplay::getCharacter(uint8_t row, uint8_t column)
{
    if (row < gScreenWidth && column < gScreenHeight) {
        const uint8_t* const cp = getCharacterPosition(row, column);
        return (*cp & gTextCharacterMask) + 0x20;
    } else {
        return 0;
    }
}

    
void SharpDisplay::setLineText(uint8_t row, const String &text)
{
    if (row < gScreenHeight) {
        const uint8_t length = text.length();
        for (uint8_t column = 0; column < gScreenWidth; ++column) {
            if (column < length) {
                setCharacter(row, column, text.charAt(column));
            } else {
                setCharacter(row, column, ' ');
            }
        }
        markRowForUpdate(row);
    }
}
    
    
void SharpDisplay::setLineInverted(uint8_t row, bool inverted)
{
    if (row < gScreenHeight) {
        for (uint8_t column = 0; column < gScreenWidth; ++column) {
            uint8_t *c = getCharacterPosition(row, column);
            if (inverted) {
                *c |= gTextFlagInverse;
            } else {
                *c &= ~gTextFlagInverse;
            }
        }
        markRowForUpdate(row);
    }
}
    
    
void SharpDisplay::setCursorPosition(uint8_t row, uint8_t column)
{
    // Check the bounds.
    if (row > gScreenHeight) {
        row = gScreenHeight;
    }
    if (column > gScreenWidth) {
        column = gScreenWidth;
    }
    gCursorY = row;
    gCursorX = column;
    // If the cursor is below the last row, only X position 0 is valid.
    if (gCursorY == gScreenHeight) {
        gCursorX = 0;
    }
}
    
    
void SharpDisplay::getCursorPosition(uint8_t &row, uint8_t &column)
{
    row = gCursorY;
    column = gCursorX;
}
    
    
void SharpDisplay::writeCharacter(uint8_t c)
{
    if (c == '\n') { // Add a line break
        gCursorX = 0;
        if (gCursorY < gScreenHeight) {
            ++gCursorY;
        } else {
            // Cursor is at the bottom. Scroll is required.
            scrollScreen(ScrollUp);
        }
    } else if (c >= 0x20) { // Ignore any other control characters.
        if (gCursorX == gScreenWidth) {
            gCursorX = 0;
            if (gCursorY < gScreenHeight) {
                ++gCursorY;
            }
        }
        if (gCursorY == gScreenHeight) {
            scrollScreen(ScrollUp);
            --gCursorY;
        }
        setCharacter(gCursorY, gCursorX, c);
        ++gCursorX;
    }
}

    
void SharpDisplay::writeText(const String &text)
{
    const unsigned int length = text.length();
    for (unsigned int i = 0; i < length; ++i) {
        writeCharacter(text.charAt(i));
    }
}

    
void SharpDisplay::scrollScreen(ScrollDirection direction)
{
    switch (direction) {
        case ScrollUp:
            memmove(getCharacterPosition(0, 0), getCharacterPosition(1, 0), gScreenWidth*(gScreenHeight-1));
            memset(getCharacterPosition(gScreenHeight-1, 0), 0, gScreenWidth);
            break;
            
        case ScrollDown:
            memmove(getCharacterPosition(1, 0), getCharacterPosition(0, 0), gScreenWidth*(gScreenHeight-1));
            memset(getCharacterPosition(0, 0), 0, gScreenWidth);
            break;

        case ScrollLeft:
            memmove(getCharacterPosition(0, 0), getCharacterPosition(0, 1), (gScreenWidth*gScreenHeight)-1);
            for (uint8_t row = 0; row < gScreenHeight; ++row) {
                uint8_t *c = getCharacterPosition(row, gScreenWidth-1);
                *c = 0;
            }
            break;

        case ScrollRight:
            memmove(getCharacterPosition(0, 1), getCharacterPosition(0, 0), (gScreenWidth*gScreenHeight)-1);
            for (uint8_t row = 0; row < gScreenHeight; ++row) {
                uint8_t *c = getCharacterPosition(row, 0);
                *c = 0;
            }
            break;
    }
    markScreenForUpdate();
}
    

}





