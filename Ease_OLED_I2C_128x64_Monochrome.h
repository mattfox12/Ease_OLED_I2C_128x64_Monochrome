/*
	DESCRIPTION

		This library is written for an 4-pin I2C monochrome display with a resolution of 128x64 pixels.
		It's tested with the following display:

			- 0,96" I2C OLED, Manufacturer: Heltec-Taobao

		It is not tested with other displays, therefor I can't guarantee that it will work with another device.

	LICENSE

		This library is licensed under the GNU GPLv3 (https://www.gnu.org/licenses/gpl.html) open source license.
		Thus anybody is allowed to copy and modify the source code, provided all changes are open source too and the author is in knowledge of all changes.
		This can happen either via eMail or directly on GitHub, in other words, on this repository.

	MICROCONTROLLER
		- Arduino Nano/UNO/Pro Mini
			- SDA -> A4
			- SCL -> A5
			- VCC -> 3.3-5V (depending on your display)
			- GND -> Ground
*/

#ifndef OLED_data_H
#define OLED_data_H

#include <Arduino.h>
#include <Wire.h>

#define OLED_Max_X 128
#define OLED_Max_Y 64

#define OLED_ADDRESS 0x3C
#define I2C_400KHZ 1 // 0 to use default 100Khz, 1 for 400Khz

#define COMMAND_MODE 0x80
#define OLED_DATA_MODE 0x40

#define COMMAND_CHARGE_PUMP_SETTING	0x8d
#define COMMAND_CHARGE_PUMP_ENABLE 0x14

#define COMMAND_DISPLAY_OFF 0xAE
#define COMMAND_DISPLAY_ON 0xAF
#define COMMAND_BLACK_BACKGROUND 0xA6
#define COMMAND_WHITE_BACKGROUND 0xA7
#define COMMAND_SET_BRIGHTNESS 0x81

#define COMMAND_MIRROR_VERTICAL	0xA0 | 0x1
#define COMMAND_MIRROR_HORIZONTAL 0xC8

#define HORIZONTAL_ADDRESSING 0x00
#define PAGE_ADDRESSING	0x02

#define LINE_T 0x01
#define RECT_T 0x02
#define RECTFILL_T 0x03
#define ELLIPSE_T 0x04
#define ELLIPSEFILL_T 0x05
#define CHAR_T 0x06
#define TEXT_T 0x07
#define BITMAP_T 0x08
#define VISIBLE 0x20 // bit 5
#define NEGATIVE 0x40 // bit 6
#define INVERTED 0x60 // bit 5 & 6
#define DELETED 0x80 // bit 7

// DrawingObj: 5 bytes for drawing objects to screen, dimensions in pixels
typedef struct drawData_t {
	byte register0; // bits: 0-4 object type, 5-6 Invisible|Visible|Negative|Inverted, 7 Deleted
	uint8_t x; // will be adjusted to screen coordinates -64 < 192
	uint8_t y; // will be adjusted to screen coordinates -96 < 160
	uint8_t v0; // depending on object: width, (char/text/bitmap) index
	uint8_t v1; // depending on object: height, (bitmap) width
};
typedef union DrawingObj{
  drawData_t data;
  byte bytes[sizeof(drawData_t)];
};


class Display {
	char buffer[16];    	// make sure large enough for largest string in Strings.h
	byte render_buffer[16]; // is rendering needed
	byte tile_buffer[8]; 	// for drawing an 8x8 square
	DrawingObj *drawingObjs;
	uint16_t drawingCount;

	public:
		byte addressingMode;
		boolean scroll;

		Display(DrawingObj objs[], uint16_t count);

		// screen commands/settings
		void init(boolean regulator);
		void sendCommand(byte command);
		void sendData(byte data);
		void setCursor(byte posX, byte posY);
		void setBlackBackground();
		void setWhiteBackground();
		void setDisplayOff();
		void setDisplayOn();
		void setPageMode();
		void setHorizontalMode();
		void setBrightness(byte brightness);
		void rotateDisplay180();

		// objects to draw
		uint16_t addObj(DrawingObj givenObj);
		void updateObjStyle(uint16_t index, uint8_t v);
		void updateObj(uint16_t index, uint8_t x, uint8_t y);
		void updateObj(uint16_t index, uint8_t x, uint8_t y, uint8_t v0, uint8_t v1);

		// force buffer location to redraw
		void setBuffer(uint8_t bufX = 255, uint8_t bufY = 255); // set render buffer to true

		// drawing to screen
		void showBuffer();
		void draw();
		void drawTileBuffer();
		void drawText(const char *givenString, int16_t X, int16_t Y, boolean negative = false, boolean inverted = false);
		void drawBitmap(const byte *bitmapArray, int16_t X, int16_t Y, int16_t width = 8, int16_t height = 8, boolean negative = false, boolean inverted = false, uint8_t scale = 1);

		// text width
		uint8_t charWidth(uint8_t givenChar);
		uint16_t bufferTextWidth(uint16_t string_index);
		uint16_t bufferTextWidth();
		uint16_t textWidth(const char *givenString);

		// bitmap height
		uint16_t bufferBitmapHeight(uint16_t bitmap_index, uint8_t width);

		// clear buffers/screen
		void clearTileBuffer();
		void clearBuffer();
		void clear();

	private:
		// buffer
		void updateObjBuffer(uint16_t index);
		// void updateEllipseBuffer(uint16_t index, boolean filled = true);
		void updateRectBuffer(uint16_t index, boolean filled = true);
		void updateLineBuffer(uint16_t index);
		void updateCharBuffer(uint16_t index);
		void updateTextBuffer(uint16_t index);
		void updateBitmapBuffer(uint16_t index);

		// render objects/strings/bitmaps to tile_buffer
		void renderTile(uint8_t bufX = 255, uint8_t bufY = 255);
		void renderLine(uint16_t index, uint8_t bufX = 255, uint8_t bufY = 255);
		void renderRect(uint16_t index, uint8_t bufX = 255, uint8_t bufY = 255, boolean filled = true);
		void renderEllipse(uint16_t index, uint8_t bufX = 255, uint8_t bufY = 255, boolean filled = true);
		void plotFilledEllipsePoints(uint16_t index, int16_t X, int16_t Y, int16_t startX, int16_t startY, int8_t offsetX, int8_t offsetY, uint8_t bufX, uint8_t bufY);
		void plot4EllipsePoints(uint16_t index, int16_t X, int16_t Y, int16_t startX, int16_t startY, int8_t offsetX, int8_t offsetY, uint8_t bufX, uint8_t bufY);
		void putPixel(uint16_t index, int16_t X, int16_t Y, uint8_t bufX, uint8_t bufY);
		void renderChar(uint16_t index, uint8_t bufX = 255, uint8_t bufY = 255);
		void renderChar(uint8_t givenChar, int16_t x, int16_t y, uint8_t height, boolean negative = false, boolean inverted = false, uint8_t bufX = 255, uint8_t bufY = 255);
		void renderBufferText(uint16_t index, uint8_t bufX = 255, uint8_t bufY = 255);
		void renderBufferBitmap(uint16_t index, uint8_t bufX = 255, uint8_t bufY = 255);

		// render non-indexed strings, bitmaps
		void renderText(const char *givenString, int16_t x, int16_t y, uint8_t height, boolean negative = false, boolean inverted = false, uint8_t bufX = 255, uint8_t bufY = 255);
		void renderBitmap(const byte *bitmapArray, int16_t x, int16_t y, uint16_t width, uint16_t height, boolean negative = false, boolean inverted = false, uint8_t scale = 1, uint8_t bufX = 255, uint8_t bufY = 255);
};
#endif
