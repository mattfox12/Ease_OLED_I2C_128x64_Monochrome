#include "Ease_OLED_I2C_128x64_Monochrome.h"
#include "Ease_OLED_I2C_128x64_Monochrome_Font.h"
#include "Ease_OLED_I2C_128x64_Strings.h"
#include "Ease_OLED_I2C_128x64_Bitmaps.h"
#include <Wire.h>
#include <avr/pgmspace.h>

Display::Display(DrawingObj objs[], uint16_t count){
	drawingObjs = objs;
	drawingCount = count;
}

void Display::init(boolean regulator) {
	Wire.begin();
	// upgrade to 400KHz! (only use it when all other i2c devices support that speed)
	if (I2C_400KHZ) {
		byte twbrbackup = TWBR;
		TWBR = 12;
	}

	if (regulator) {
    	sendCommand(COMMAND_CHARGE_PUMP_SETTING);
		sendCommand(COMMAND_CHARGE_PUMP_ENABLE);
	}

	scroll = false;

	// mark all drawingObjs as deleted
	for (uint16_t i=0; i<drawingCount; i++) {
		bitSet(drawingObjs[i].data.register0, 7);
	}
	clearTileBuffer();
	clearBuffer();

	setDisplayOff();
    setBlackBackground();
	setPageMode();
	// setHorizontalMode();
	clear();
    setDisplayOn();
}

void Display::sendCommand(byte command) {
	Wire.beginTransmission(OLED_ADDRESS); // begin transmitting
	Wire.write(COMMAND_MODE); // data mode
	Wire.write(command); // send command
	Wire.endTransmission(); // stop transmitting
}

void Display::sendData(byte data) {
	Wire.beginTransmission(OLED_ADDRESS);
	Wire.write(OLED_DATA_MODE);
	Wire.write(data);
	Wire.endTransmission();
}

void Display::setCursor(byte posX, byte posY) {
	// X - 1 unit = 8 pixel columns
	// Y - 1 unit = 1 page (8 pixel rows)
    sendCommand(0x00 + (8 * posX & 0x0F)); // set column lower address
    sendCommand(0x10 + ((8 * posX >> 4) & 0x0F)); // set column higher address
	sendCommand(0xB0 + posY); // set page address
}

void Display::setWhiteBackground() {
	sendCommand(COMMAND_WHITE_BACKGROUND);
}

void Display::setBlackBackground() {
	sendCommand(COMMAND_BLACK_BACKGROUND);
}

void Display::setDisplayOff() {
	sendCommand(COMMAND_DISPLAY_OFF);
}

void Display::setDisplayOn() {
	sendCommand(COMMAND_DISPLAY_ON);
}

void Display::setBrightness(byte brightness) {
	sendCommand(COMMAND_SET_BRIGHTNESS);
	sendCommand(brightness);
}

void Display::setPageMode() {
	addressingMode = PAGE_ADDRESSING;
	sendCommand(0x20); //set addressing mode
	sendCommand(PAGE_ADDRESSING); //set page addressing mode
}

void Display::setHorizontalMode() {
	addressingMode = HORIZONTAL_ADDRESSING;
	sendCommand(0x20); // set addressing mode
	sendCommand(HORIZONTAL_ADDRESSING); // set page addressing mode
}

void Display::rotateDisplay180() {
	sendCommand(COMMAND_MIRROR_VERTICAL);
	sendCommand(COMMAND_MIRROR_HORIZONTAL);
}

uint16_t Display::addObj(DrawingObj givenObj) {
	int16_t returnValue = -1;
	for (uint16_t i=0; i<drawingCount; i++) {
		// find first obj that is ready to be reused
		if (bitRead(drawingObjs[i].data.register0, 7)) {
			for (uint16_t j=0; j<sizeof(DrawingObj); j++) {
				drawingObjs[i].bytes[j] = givenObj.bytes[j];
			}

			// set to not deleted
			bitClear(drawingObjs[i].data.register0, 7);

			// redraw within object bounds
			updateObjBuffer(i);

			returnValue = i;
			i = drawingCount; // exit loop
		}
	}

	return returnValue;
}

void Display::updateObjStyle(uint16_t index, uint8_t v) {
	// make invisible
	bitClear(drawingObjs[index].data.register0, 5);
	bitClear(drawingObjs[index].data.register0, 6);

	// update bits in register0
	drawingObjs[index].data.register0 = drawingObjs[index].data.register0 | v;

	updateObjBuffer(index);
}

void Display::updateObj(uint16_t index, uint8_t x, uint8_t y) {
	// redraw old spot
	updateObjBuffer(index);

	// set new values
	drawingObjs[index].data.x 	= x;
	drawingObjs[index].data.y 	= y;

	// redraw new spot
	updateObjBuffer(index);
}

void Display::updateObj(uint16_t index, uint8_t x, uint8_t y, uint8_t v0, uint8_t v1) {
	// redraw old spot
	updateObjBuffer(index);

	// set new values
	drawingObjs[index].data.x 	= x;
	drawingObjs[index].data.y 	= y;
	drawingObjs[index].data.v0 	= v0;
	drawingObjs[index].data.v1 	= v1;

	// redraw new spot
	updateObjBuffer(index);
}

void Display::updateObjBuffer(uint16_t index) {
	if ((BITMAP_T & drawingObjs[index].data.register0) == BITMAP_T) updateBitmapBuffer(index);
	else if ((TEXT_T & drawingObjs[index].data.register0) == TEXT_T) updateTextBuffer(index);
	else if ((CHAR_T & drawingObjs[index].data.register0) == CHAR_T) updateCharBuffer(index);
	else if ((ELLIPSEFILL_T & drawingObjs[index].data.register0) == ELLIPSEFILL_T) updateRectBuffer(index, true); //updateEllipseBuffer(index, true);
	else if ((ELLIPSE_T & drawingObjs[index].data.register0) == ELLIPSE_T) updateRectBuffer(index, true); //updateEllipseBuffer(index, false);
	else if ((RECTFILL_T & drawingObjs[index].data.register0) == RECTFILL_T) updateRectBuffer(index, true);
	else if ((RECT_T & drawingObjs[index].data.register0) == RECT_T) updateRectBuffer(index, false);
	else if ((LINE_T & drawingObjs[index].data.register0) == LINE_T) updateLineBuffer(index);
	else setBuffer(drawingObjs[index].data.x, drawingObjs[index].data.y);
}

void Display::updateBitmapBuffer(uint16_t index) {
	int16_t startX = drawingObjs[index].data.x;
	if (startX > 192) startX -= 256;
	int16_t startY = drawingObjs[index].data.y;
	if (startY > 192) startY -= 256;

	// load buffer with string text
	int8_t bitmap_index = drawingObjs[index].data.v0;

	int8_t width = drawingObjs[index].data.v1;
	int16_t height = bufferBitmapHeight(index, width);

	// square for now
	for (byte x = max(0,floor(startX/8)*8); x < startX+width; x+=8) {
		for (byte y = max(0,floor(startY/8)*8); y < startY+height; y+=8) {
			setBuffer(x, y);
		}
	}
}

void Display::updateTextBuffer(uint16_t index) {
	int16_t startX = drawingObjs[index].data.x;
	if (startX > 192) startX -= 256;
	int16_t startY = drawingObjs[index].data.y;
	if (startY > 192) startY -= 256;

	// load buffer with string text
	int8_t string_index = drawingObjs[index].data.v0;
	strcpy_P(buffer, (char*)pgm_read_word(&(string_table[string_index]))); // Necessary casts and dereferencing, just copy.

	int16_t width = bufferTextWidth(); // increase with the chars given
	int16_t height = drawingObjs[index].data.v1;

	// square for now
	for (byte x = max(0,floor(startX/8)*8); x < startX+width; x+=8) {
		for (byte y = max(0,floor(startY/8)*8); y < startY+height; y+=8) {
			setBuffer(x, y);
		}
	}
}

void Display::updateCharBuffer(uint16_t index) {
	int16_t startX = drawingObjs[index].data.x;
	if (startX > 192) startX -= 256;
	int16_t startY = drawingObjs[index].data.y;
	if (startY > 192) startY -= 256;

	byte pChar = drawingObjs[index].data.v0;

	uint8_t width = charWidth(pChar);
	uint8_t height = drawingObjs[index].data.v1;

	// square for now
	for (byte x = max(0,floor(startX/8)*8); x < startX+width; x+=8) {
		for (byte y = max(0,floor(startY/8)*8); y < startY+height; y+=8) {
			setBuffer(x, y);
		}
	}
}

void Display::updateRectBuffer(uint16_t index, boolean filled) {
	int16_t startX = drawingObjs[index].data.x;
	if (startX > 192) startX -= 256;
	int16_t startY = drawingObjs[index].data.y;
	if (startY > 192) startY -= 256;
	int16_t width = drawingObjs[index].data.v0;
	if (width > 160) width -= 256;
	int16_t height = drawingObjs[index].data.v1;
	if (height > 160) height -= 256;

	// use int for comparison, or int8_t may wrap to negative
	if (filled) {
		for (byte x = max(0,floor(startX/8)*8); x < startX+width; x+=8) {
			for (byte y = max(0,floor(startY/8)*8); y < startY+height; y+=8) {
				setBuffer(x, y);
			}
		}
	} else {
		for (byte x = max(0,floor(startX/8)*8); x < startX+width; x+=8) {
			setBuffer(x, floor(startY/8)*8);
			setBuffer(x, startY+height-1);
		}
		for (byte y = max(0,floor(startY/8)*8); y < startY+height; y+=8) {
			setBuffer(floor(startX/8)*8, y);
			setBuffer(startX+width-1, y);
		}
	}
}

void Display::updateLineBuffer(uint16_t index) {
	// get point info
	int16_t x0 = drawingObjs[index].data.x;
	if (x0 > 192) x0 -= 256;
	int16_t y0 = drawingObjs[index].data.y;
	if (y0 > 192) y0 -= 256;
	int16_t x1 = drawingObjs[index].data.v0;
	if (x1 > 160) x1 -= 256;
	x1 += x0;
	int16_t y1 = drawingObjs[index].data.v1;
	if (y1 > 160) y1 -= 256;
	y1 += y0;

	int16_t dy = y1 - y0;
	int16_t dx = x1 - x0;
	float t = 0.5f;                      // offset for rounding

	// first pixel
	setBuffer(x0, y0);

	// functions for the two slopes
	if (abs(dx) > abs(dy)) {          // slope < 1
		float m = (float)dy / (float)dx;      // compute slope
		t += y0;
		dx = (dx < 0) ? -1 : 1;
		m *= dx;
		while (x0 != x1) {
			x0 += dx;                           // step to next x value
			t += m;                             // add slope to y value

			uint8_t currentY = (int8_t)t;
			setBuffer(x0, currentY);
		}
	} else {                                    // slope >= 1
		float m = (float) dx / (float) dy;      // compute slope
		t += x0;
		dy = (dy < 0) ? -1 : 1;
		m *= dy;
		while (y0 != y1) {
			y0 += dy;                           // step to next y value
			t += m;                             // add slope to x value

			uint8_t currentX = (uint8_t)t;
			setBuffer(currentX, y0);
		}
	}
}

void Display::setBuffer(uint8_t bufX, uint8_t bufY) { // limits: x is between 0-127, y 0-63
	// assign x/y bit to true
	if (bufY >= 0 && bufY < OLED_Max_Y && bufX >= 0 && bufX < OLED_Max_X) {
		bitSet(render_buffer[bufX / 8], bufY / 8);
	}
}

void Display::showBuffer() {
	// skip setCursor if we are continuing in same row
	int8_t lastIndex = -2;
	int8_t lastJ = -1;

	// make solid square
	for(uint8_t i = 0; i < sizeof(tile_buffer); i++) {
		tile_buffer[i] = 0xFF;
	}

	// loop through render_buffer bytes
	for (uint8_t j = 0; j < 8; j++) {
		for(uint8_t i = 0; i < sizeof(render_buffer); i++) {
			if (bitRead(render_buffer[i], j)) { // this frame needs rendering
				if (lastIndex != i - 1 || lastJ != j) setCursor(i, j);

				drawTileBuffer();	// draw to screen

				// remember last frame
				lastIndex = j*8 + i;
			}
		}
		lastJ = j;
	}

	clearTileBuffer();
}

void Display::draw() {
	// skip setCursor if we are continuing in same row
	int8_t lastIndex = -2;
	int8_t lastJ = -1;

	// loop through render_buffer bytes
	for (uint8_t j = 0; j < 8; j++) {
		for(uint8_t i = 0; i < sizeof(render_buffer); i++) {
			if (bitRead(render_buffer[i], j)) { // this frame needs rendering
				if (lastIndex != i - 1 || lastJ != j) setCursor(i, j);

				renderTile(i, j); 	// render to buffer
				drawTileBuffer();	// draw to screen
				clearTileBuffer();

				// remember last frame
				lastIndex = j*8 + i;
			}
		}
		lastJ = j;
	}

	clearBuffer();
}

void Display::drawText(const char *givenString, int16_t X, int16_t Y, boolean negative, boolean inverted) {
	uint8_t startX = max(0,X/8);
	uint8_t startY = max(0,Y/8);
	uint8_t width = textWidth(givenString);
	uint8_t height = 8; // assumes 8px height

	// skip setCursor if we are continuing in same row
	int8_t lastIndex = -2;
	int8_t lastJ = -1;

	for (uint8_t j = startY; j <= min(7,(Y+height)/8); j++) {
		for(uint8_t i = startX; i <= min(15,(X+width)/8); i++) {
			if (lastIndex != i - 1 || lastJ != j) setCursor(i, j);

			renderTile(i, j); 	// render to buffer
			renderText(givenString, X, Y, height, negative, inverted, i, j); // adds text on top of buffer

			drawTileBuffer();	// draw to screen
			clearTileBuffer();

			// remember last frame
			lastIndex = j*8 + i;
		}
		lastJ = j;
	}
}

void Display::drawBitmap(const byte *bitmapArray, int16_t X, int16_t Y, int16_t width = 8, int16_t height = 8, boolean negative = false, boolean inverted = false, uint8_t scale = 1) {
	uint8_t startX = max(0,X/8);
	uint8_t startY = max(0,Y/8);

	// skip setCursor if we are continuing in same row
	int8_t lastIndex = -2;
	int8_t lastJ = -1;

	for (uint8_t j = startY; j <= min(7,(Y+height*scale)/8); j++) {
		for(uint8_t i = startX; i <= min(15,(X+width*scale)/8); i++) {
			if (lastIndex != i - 1 || lastJ != j) setCursor(i, j);

			renderTile(i, j); 	// render to buffer
			renderBitmap(bitmapArray, X, Y, width, height, negative, inverted, scale, i, j); // adds text on top of buffer

			drawTileBuffer();	// draw to screen
			clearTileBuffer();

			// remember last frame
			lastIndex = j*8 + i;
		}
		lastJ = j;
	}
}

void Display::renderTile(uint8_t bufX, uint8_t bufY) {
	for (uint16_t index = 0; index < drawingCount; index++) {
		if ((bitRead(drawingObjs[index].data.register0, 5) || bitRead(drawingObjs[index].data.register0, 6)) && !bitRead(drawingObjs[index].data.register0, 7)) { // visible, not deleted
			// update tile_buffer
			if ((BITMAP_T & drawingObjs[index].data.register0) == BITMAP_T) renderBufferBitmap(index, bufX, bufY);
			else if ((TEXT_T & drawingObjs[index].data.register0) == TEXT_T) renderBufferText(index, bufX, bufY);
			else if ((CHAR_T & drawingObjs[index].data.register0) == CHAR_T) renderChar(index, bufX, bufY);
			else if ((ELLIPSEFILL_T & drawingObjs[index].data.register0) == ELLIPSEFILL_T) renderEllipse(index, bufX, bufY, true);
			else if ((ELLIPSE_T & drawingObjs[index].data.register0) == ELLIPSE_T) renderEllipse(index, bufX, bufY, false);
			else if ((RECTFILL_T & drawingObjs[index].data.register0) == RECTFILL_T) renderRect(index, bufX, bufY, true);
			else if ((RECT_T & drawingObjs[index].data.register0) == RECT_T) renderRect(index, bufX, bufY, false);
			else if ((LINE_T & drawingObjs[index].data.register0) == LINE_T) renderLine(index, bufX, bufY);
		}
	}
}

void Display::renderLine(uint16_t index, uint8_t bufX, uint8_t bufY) {
	bool negative = false;
	bool inverted = false;
	if ((INVERTED & drawingObjs[index].data.register0) == INVERTED) inverted = true;
	else if ((NEGATIVE & drawingObjs[index].data.register0) == NEGATIVE) negative = true;

	int8_t pixelXOffset = bufX*8;
	int8_t pixelYOffset = bufY*8;

	// get point info
	int16_t x0 = drawingObjs[index].data.x;
	if (x0 > 192) x0 -= 256;
	int16_t y0 = drawingObjs[index].data.y;
	if (y0 > 192) y0 -= 256;
	int16_t x1 = drawingObjs[index].data.v0;
	if (x1 > 160) x1 -= 256;
	x1 += x0;
	int16_t y1 = drawingObjs[index].data.v1;
	if (y1 > 160) y1 -= 256;
	y1 += y0;

	int16_t dy = y1 - y0;
	int16_t dx = x1 - x0;
	float t = 0.5f;	// offset for rounding

	if (x0 >= pixelXOffset && x0 < pixelXOffset+8 && y0 >= pixelYOffset && y0 < pixelYOffset+8) {
		if (inverted) {
			if (bitRead(tile_buffer[x0-pixelXOffset], y0-pixelYOffset)) bitClear(tile_buffer[x0-pixelXOffset], y0-pixelYOffset);
			else bitSet(tile_buffer[x0-pixelXOffset], y0-pixelYOffset);
		} else if (negative) bitClear(tile_buffer[x0-pixelXOffset], y0-pixelYOffset);
		else bitSet(tile_buffer[x0-pixelXOffset], y0-pixelYOffset);
	}

	// functions for the two slopes
	if (abs(dx) > abs(dy)) {          // slope < 1
		float m = (float)dy / (float)dx;      // compute slope
		t += y0;
		dx = (dx < 0) ? -1 : 1;
		m *= dx;
		while (x0 != x1) {
			x0 += dx;                           // step to next x value
			t += m;                             // add slope to y value

			int8_t currentY = (int8_t)t;

			// make sure to stay within current frame
			if (currentY >= pixelYOffset && currentY < pixelYOffset + 8 && x0 >= pixelXOffset && x0 < pixelXOffset + 8) {
				if (inverted) {
					if (bitRead(tile_buffer[x0-pixelXOffset], currentY-pixelYOffset)) bitClear(tile_buffer[x0-pixelXOffset], currentY-pixelYOffset);
					else bitSet(tile_buffer[x0-pixelXOffset], currentY-pixelYOffset);
				} else if (negative) bitClear(tile_buffer[x0-pixelXOffset], currentY-pixelYOffset);
				else bitSet(tile_buffer[x0-pixelXOffset], currentY-pixelYOffset);
			}
		}
	} else {                                    // slope >= 1
		float m = (float) dx / (float) dy;      // compute slope
		t += x0;
		dy = (dy < 0) ? -1 : 1;
		m *= dy;
		while (y0 != y1) {
			y0 += dy;                           // step to next y value
			t += m;                             // add slope to x value

			int8_t currentX = (int8_t)t;

			// make sure to stay within current frame
			if (y0 >= pixelYOffset && y0 < pixelYOffset + 8 && currentX >= pixelXOffset && currentX < pixelXOffset + 8) {
				if (inverted) {
					if (bitRead(tile_buffer[currentX-pixelXOffset], y0-pixelYOffset)) bitClear(tile_buffer[currentX-pixelXOffset], y0-pixelYOffset);
					else bitSet(tile_buffer[currentX-pixelXOffset], y0-pixelYOffset);
				} else if (negative) bitClear(tile_buffer[currentX-pixelXOffset], y0-pixelYOffset);
				else bitSet(tile_buffer[currentX-pixelXOffset], y0-pixelYOffset);
			}
		}
	}
}

void Display::renderRect(uint16_t index, uint8_t bufX, uint8_t bufY, boolean filled) {
	bool negative = false;
	bool inverted = false;
	if ((INVERTED & drawingObjs[index].data.register0) == INVERTED) inverted = true;
	else if ((NEGATIVE & drawingObjs[index].data.register0) == NEGATIVE) negative = true;

	int16_t startX = drawingObjs[index].data.x;
	if (startX > 192) startX -= 256;
	int16_t startY = drawingObjs[index].data.y;
	if (startY > 192) startY -= 256;
	int16_t width = drawingObjs[index].data.v0;
	if (width > 160) width -= 256;
	int16_t height = drawingObjs[index].data.v1;
	if (height > 160) height -= 256;

	for (byte i=0; i<8; i++) {
		// check within x
		if (startX <= bufX*8 + i && startX + width > bufX*8 + i) {
			for (byte j=0; j<8; j++) {
				// check within y
				if (startY <= bufY*8 + j && startY + height > bufY*8 + j) {
					if (filled) { // fill
						if (inverted) {
							if (bitRead(tile_buffer[i], j)) bitClear(tile_buffer[i], j);
							else bitSet(tile_buffer[i], j);
						} else if (negative) bitClear(tile_buffer[i], j);
						else bitSet(tile_buffer[i], j);
					} else { // outline
						if (startX == bufX*8 + i || startX + width - 1 == bufX*8 + i || startY == bufY*8 + j || startY + height - 1 == bufY*8 + j) {
							if (inverted) {
								if (bitRead(tile_buffer[i], j)) bitClear(tile_buffer[i], j);
								else bitSet(tile_buffer[i], j);
							} else if (negative) bitClear(tile_buffer[i], j);
							else bitSet(tile_buffer[i], j);
						}
					}
				}
			}
		}
	}
}

void Display::renderEllipse(uint16_t index, uint8_t bufX, uint8_t bufY, boolean filled) {
	int16_t startX = drawingObjs[index].data.x;
	if (startX > 192) startX -= 256;
	int16_t startY = drawingObjs[index].data.y;
	if (startY > 192) startY -= 256;
	int16_t rx = drawingObjs[index].data.v0/2;
	int16_t ry = drawingObjs[index].data.v1/2;
	int8_t offsetX = 0;
	if (drawingObjs[index].data.v0 % 2 == 0) offsetX = 1;
	int8_t offsetY = 0;
	if (drawingObjs[index].data.v1 % 2 == 0) offsetY = 1;
	startX += rx;
	startY += ry;

    int16_t EllipseError 	= 0;
    int16_t TwoASquare 		= 2*rx*rx;
    int16_t TwoBSquare 		= 2*ry*ry;
    int16_t X 				= rx;
    int16_t Y 				= 0;
    int16_t XChange 		= ry*ry*(1-2*rx);
    int16_t YChange 		= rx*rx;
    int16_t StoppingX 		= TwoBSquare*rx;
    int16_t StoppingY 		= 0;

	boolean doPlot = (offsetY == 0);

    while (StoppingX >= StoppingY) {
        if (doPlot) {
			if (filled) {
				plotFilledEllipsePoints(index, X, Y, startX, startY, offsetX, offsetY, bufX, bufY);
	        } else {
				plot4EllipsePoints(index, X, Y, startX, startY, offsetX, offsetY, bufX, bufY);
	        }
		}
		doPlot = true;

        Y++;
        StoppingY += TwoASquare;
        EllipseError += YChange;
        YChange += TwoASquare;

        if (2*EllipseError + XChange > 0) {
            X--;
            StoppingX -= TwoBSquare;
            EllipseError += XChange;
            XChange += TwoBSquare;
        }
    }

    int16_t memX = X;
    int16_t endY = Y-1;

    // second set of points
    X = 0;
    Y = ry;
    XChange = ry * ry;
    YChange = rx * rx * (1-2*ry);
    EllipseError = 0;
    StoppingX = 0;
    StoppingY = TwoASquare * ry;

	doPlot = (offsetX == 0);

    while (StoppingX <= StoppingY) {
		if (doPlot && !filled) {
	        if (memX+1 != X || endY != Y) {
				plot4EllipsePoints(index, X, Y, startX, startY, offsetX, offsetY, bufX, bufY);
	        }
		}
		doPlot = true;

        X++;
        StoppingX += TwoBSquare;
        EllipseError += XChange;
        XChange += TwoBSquare;

        if (2*EllipseError + YChange > 0) {
			if (filled) {
				if (memX+1 != X - 1 || endY != Y) {
					plotFilledEllipsePoints(index, X-1, Y, startX, startY, offsetX, offsetY, bufX, bufY);
	            }
			}

            Y--;
            StoppingY -= TwoASquare;
            EllipseError += YChange;
            YChange += TwoASquare;
        }
    }
}

void Display::plotFilledEllipsePoints(uint16_t index, int16_t X, int16_t Y, int16_t startX, int16_t startY, int8_t offsetX, int8_t offsetY, uint8_t bufX, uint8_t bufY) {
	bool negative = false;
	bool inverted = false;
	if ((INVERTED & drawingObjs[index].data.register0) == INVERTED) inverted = true;
	else if ((NEGATIVE & drawingObjs[index].data.register0) == NEGATIVE) negative = true;

	int16_t x1 = startX + X - offsetX - bufX * 8;
    int16_t y1 = startY + Y - offsetY - bufY * 8;
    int16_t x2 = startX - X - bufX * 8;
    int16_t y2 = startY - Y - bufY * 8;

	// quadrant 1/2
	for (x1; x1 >= x2; x1--) {
		if (x1 >= 0 && x1 < 8) {
			if (y1 >= 0 && y1 < 8) {
				if (inverted) {
					if (bitRead(tile_buffer[x1], y1)) bitClear(tile_buffer[x1], y1);
					else bitSet(tile_buffer[x1], y1);
				} else if (negative) bitClear(tile_buffer[x1], y1);
				else bitSet(tile_buffer[x1], y1);
			}
			if (y1 != y2 && y2 >= 0 && y2 < 8) {
				if (inverted) {
					if (bitRead(tile_buffer[x1], y2)) bitClear(tile_buffer[x1], y2);
					else bitSet(tile_buffer[x1], y2);
				} else if (negative) bitClear(tile_buffer[x1], y2);
				else bitSet(tile_buffer[x1], y2);
			}
		}
	}
}

void Display::plot4EllipsePoints(uint16_t index, int16_t X, int16_t Y, int16_t startX, int16_t startY, int8_t offsetX, int8_t offsetY, uint8_t bufX, uint8_t bufY) {
	int16_t x1 = startX + X - offsetX;
    int16_t y1 = startY + Y - offsetY;
    int16_t x2 = startX - X;
    int16_t y2 = startY - Y;

    putPixel(index, x1, y1, bufX, bufY); // quadrant 1

	if (x2 != x1) putPixel(index, x2, y1, bufX, bufY); // quadrant 2

	if (y1 != y2) {
		if (x2 != x1) putPixel(index, x2, y2, bufX, bufY); // quadrant 3

	    putPixel(index, x1, y2, bufX, bufY); // quadrant 4
	}
}

void Display::putPixel(uint16_t index, int16_t X, int16_t Y, uint8_t bufX, uint8_t bufY) {
	bool negative = false;
	bool inverted = false;
	if ((INVERTED & drawingObjs[index].data.register0) == INVERTED) inverted = true;
	else if ((NEGATIVE & drawingObjs[index].data.register0) == NEGATIVE) negative = true;

	int8_t x0 = X - bufX * 8;
	int8_t y0 = Y - bufY * 8;
	if (x0 >= 0 && y0 >= 0 && x0 < 8 && y0 < 8) {
		if (inverted) {
			if (bitRead(tile_buffer[x0], y0)) bitClear(tile_buffer[x0], y0);
			else bitSet(tile_buffer[x0], y0);
		} else if (negative) bitClear(tile_buffer[x0], y0);
		else bitSet(tile_buffer[x0], y0);
    }
}

void Display::renderChar(uint16_t index, uint8_t bufX, uint8_t bufY) {
	bool negative = false;
	bool inverted = false;
	if ((INVERTED & drawingObjs[index].data.register0) == INVERTED) inverted = true;
	else if ((NEGATIVE & drawingObjs[index].data.register0) == NEGATIVE) negative = true;

	int16_t startX = drawingObjs[index].data.x;
	if (startX > 192) startX -= 256;
	int16_t startY = drawingObjs[index].data.y;
	if (startY > 192) startY -= 256;

	byte pChar = drawingObjs[index].data.v0;

	// byte width = charWidth(pChar);
	byte height = drawingObjs[index].data.v1;

	renderChar(pChar, startX, startY, height, negative, inverted, bufX, bufY);
}

void Display::renderChar(uint8_t givenChar, int16_t x, int16_t y, uint8_t height, boolean negative, boolean inverted, uint8_t bufX = 255, uint8_t bufY = 255) {
	uint8_t width = charWidth(givenChar);

	// Ignore unused ASCCII characters
	if (givenChar < 32 || givenChar > 127) {
		givenChar = '?'; // ?: characters that can't be displayed
	}
	givenChar -= 32;

	for(uint8_t i = 0; i < 8; i++) {
		// check within x
		if (bufX * 8 + i - x >= 0 && bufX * 8 + i - x < width) {
			byte fontByte = pgm_read_byte(&char_table[givenChar][bufX * 8 + i - x]);
			for (uint8_t j=0; j<8; j++) {
				// check within y
				if (bufY * 8 + j - y >= 0 && bufY * 8 + j - y < height) {
					if (bitRead(fontByte, bufY * 8 + j - y)) { // pixel
						if (inverted) {
							if (bitRead(tile_buffer[i], j)) bitClear(tile_buffer[i], j);
							else bitSet(tile_buffer[i], j);
						} else if (negative) bitClear(tile_buffer[i], j);
						else bitSet(tile_buffer[i], j);
					}
				}
			}
		}
	}
}

void Display::renderBufferText(uint16_t index, uint8_t bufX, uint8_t bufY) {
	// load buffer with string text
	int8_t string_index = drawingObjs[index].data.v0;
	strcpy_P(buffer, (char*)pgm_read_word(&(string_table[string_index]))); // Necessary casts and dereferencing, just copy.

	bool negative = false;
	bool inverted = false;
	if ((INVERTED & drawingObjs[index].data.register0) == INVERTED) inverted = true;
	else if ((NEGATIVE & drawingObjs[index].data.register0) == NEGATIVE) negative = true;

	int16_t startX = drawingObjs[index].data.x;
	if (startX > 192) startX -= 256;
	int16_t startY = drawingObjs[index].data.y;
	if (startY > 192) startY -= 256;

	int16_t width = 0; // increase with the chars given
	int16_t height = drawingObjs[index].data.v1;

	byte i = 0;
    while(buffer[i] && i < sizeof(buffer)) {
		renderChar(buffer[i], startX, startY, height, negative, inverted, bufX, bufY);
		startX += charWidth(buffer[i]) + 1;
		width += charWidth(buffer[i]) + 1;

		i++;
	}
}

void Display::renderBufferBitmap(uint16_t index, uint8_t bufX, uint8_t bufY) {
	int8_t bitmap_index = drawingObjs[index].data.v0;

	bool negative = false;
	bool inverted = false;
	if ((INVERTED & drawingObjs[index].data.register0) == INVERTED) inverted = true;
	else if ((NEGATIVE & drawingObjs[index].data.register0) == NEGATIVE) negative = true;

	int16_t startX = drawingObjs[index].data.x;
	if (startX > 192) startX -= 256;
	int16_t startY = drawingObjs[index].data.y;
	if (startY > 192) startY -= 256;

	int16_t width = drawingObjs[index].data.v1; // increase with the chars given
	int16_t height = bufferBitmapHeight(index, drawingObjs[index].data.v1);

	int16_t x = bufX * 8 - startX;
	int16_t y = bufY * 8 - startY;
	int16_t offset = 0;

	for(uint8_t i = 0; i < 8; i++) {
		// reset offset
		offset = width * (y / 8) + x;

		// check within x
		if (offset + i >= 0 && (uint8_t)(x + i) >= 0 && (uint8_t)(x + i) < width) {

			byte currentByte = pgm_read_byte(&bitmap_table[bitmap_index][offset + i]);
			for (uint8_t j=0; j<8; j++) {
				// check within y
				if ((uint8_t)(y + j) >= 0 && (uint8_t)(y + j) < height) {
					// check if we need a new byte
					if (y % 8 > 0 && (y + j) % 8 == 0) {
						offset += width;
						currentByte = pgm_read_byte(&bitmap_table[bitmap_index][offset + i]);
					}

					// draw
					if (bitRead(currentByte, (uint8_t)(y + j)%8)) { // pixel
						if (inverted) {
							if (bitRead(tile_buffer[i], j)) bitClear(tile_buffer[i], j);
							else bitSet(tile_buffer[i], j);
						} else if (negative) bitClear(tile_buffer[i], j);
						else bitSet(tile_buffer[i], j);
					}
				}
			}
		}
	}
}

void Display::renderText(const char *givenString, int16_t x, int16_t y, uint8_t height, boolean negative, boolean inverted, uint8_t bufX, uint8_t bufY) {
	int16_t startX = x;
	int16_t width = 0; // increase with the chars given

	byte i = 0;
    while(givenString[i] && i < strlen(givenString)) {
		renderChar(givenString[i], startX, y, height, negative, inverted, bufX, bufY);
		startX += charWidth(givenString[i]) + 1;
		width += charWidth(givenString[i]) + 1;

		i++;
	}
}

void Display::renderBitmap(const byte *bitmapArray, int16_t startX, int16_t startY, uint16_t width, uint16_t height, boolean negative, boolean inverted, uint8_t scale, uint8_t bufX, uint8_t bufY) {
	int16_t x = bufX * 8 - startX;
	int8_t xOff = ((bufX * 8) - startX) % scale; // fix rounding issue
	int16_t y = bufY * 8 - startY;
	int8_t yOff = startY % 8; // fix rounding issue
	int16_t offset = 0;
	int16_t row = ((y/scale) / 8);
	if (row < 0) row = 0; // don't allow to move outside our bytes
	uint8_t byte_offset = 0;

	for(uint8_t i = 0; i < 8; i++) {
		// reset offset
		offset = width * row + ((x-xOff)/scale);

		// check within x
		if (x + i >= 0 && (x+i)/scale < width) {

			byte_offset = (uint8_t)(offset + (i + xOff) / scale);
			for (uint8_t j=0; j<8; j++) {
				// check within y
				if (y + j >= 0 && (uint8_t)(y + j)/scale < height) {
					// check if we need a new byte
					if (bufY*8 + j >= startY + 8 && ((uint8_t)(y + j)/scale) % 8 == 0 && (row+1) * width < width * ((height + height % 8) / 8)) {
						byte_offset = (uint8_t)(offset + width + (i + xOff) / scale);
					}

					// draw
					if (bitRead(bitmapArray[byte_offset], ((uint8_t)(y + j)/scale)%8)) { // pixel
						if (inverted) {
							if (bitRead(tile_buffer[i], j)) bitClear(tile_buffer[i], j);
							else bitSet(tile_buffer[i], j);
						} else if (negative) bitClear(tile_buffer[i], j);
						else bitSet(tile_buffer[i], j);
					}
				}
			}
		}
	}
}

uint8_t Display::charWidth(uint8_t givenChar) {
	uint8_t width = 5; // known font width

	if (givenChar == (byte)(' ')) return 4; // forced width for space

	// Ignore unused ASCCII characters
	if (givenChar < 32 || givenChar > 127) {
		givenChar = '?'; // ?: characters that can't be displayed
	}
	givenChar -= 32;

	// loop through byte array of character, backwards
	for (int8_t i=4; i>=0; i--) {
		byte fontByte = pgm_read_byte(&char_table[givenChar][i]);
		if (fontByte == 0x00) width--;
		else i = -1; // exit loop
	}

	return width;
}

uint16_t Display::bufferTextWidth(uint16_t string_index) {
	strcpy_P(buffer, (char*)pgm_read_word(&(string_table[string_index]))); // Necessary casts and dereferencing, just copy.

	return bufferTextWidth();
}

uint16_t Display::bufferTextWidth() {
	uint16_t width = 0;

	byte i = 0;
    while(buffer[i] && i < sizeof(buffer)) {
		width += charWidth(buffer[i]) + 1;
		i++;
	}
	width -= 1;

	return width;
}

uint16_t Display::textWidth(const char *givenString) {
	uint16_t width = 0;

	byte i = 0;
	while(givenString[i] && i < strlen(givenString)) {
		width += charWidth(givenString[i]) + 1;
		i++;
	}
	width -= 1;

	return width;
}

uint16_t Display::bufferBitmapHeight(uint16_t bitmap_index, uint8_t width) {
	uint16_t h = 0;
	uint16_t i = 0;
	byte buf;
    while (buf = pgm_read_byte(&bitmap_table[bitmap_index][i])) {
      	if (i % width == 0) h++;
      	i++;
    }
	return h*8;
}

void Display::drawTileBuffer() {
	for(uint8_t i = 0; i < 8; i++) {
		sendData(tile_buffer[i]);
	}
}

void Display::clearTileBuffer() {
	for(uint8_t i = 0; i < sizeof(tile_buffer); i++) {
		tile_buffer[i] = 0x00;
	}
}

void Display::clearBuffer() {
	for(uint8_t i = 0; i < sizeof(render_buffer); i++) {
		render_buffer[i] = 0x00;
	}
}

void Display::clear() {
	for(byte i = 0; i < 8; i++) {
		setCursor(0, i);
		for(byte j = 0; j < OLED_Max_X; j++) { // clear all columns
			sendData(0);
		}
	}
	setCursor(0, 0);
}
