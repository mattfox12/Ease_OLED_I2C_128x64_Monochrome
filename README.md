# Ease OLED Library

This is a library for an 0,96" monochrome organic liquid crystal display (128x64 pixels)
It is modified from https://github.com/deloarts/OLED_I2C_128x64_Monochrome_Library and was created
to allow pixel perfect drawing at low memory cost, as well as tighter font usage.
It has been tested with the following devices:
	- 0,96" I2C OLED, Manufacturer: Heltec-Taobao on ATmega 328P, ESP32

## Commands

- **init**
	- Takes
		- boolean: Regulator on/off
	- Returns void
- **sendCommand**
	- Takes
		- byte: Command
	- Returns void
- **sendData**
	- Takes
		- byte: Data
	- Returns void
- **setCursor**
	- Takes
		- byte: Position X
		- byte: Position Y
	- Returns void
- **setBlackBackground**
	- Takes none
	- Returns void
- **setWhiteBackground**
	- Takes none
	- Returns void
- **setDisplayOff**
	- Takes none
	- Returns void
- **setDisplayOn**  
	- Takes none
	- Returns void
- **setPageMode**
	- Takes none
	- Returns void
- **setHorizontalMode**
	- Takes none
	- Returns void
- **setBrightness**
	- Takes
		- byte: Brightness
	- Returns void
- **rotateDisplay180**
	- Takes none
	- Returns void

- **addObj**
	- Takes
		- DrawingObj: object to render
	- Returns uint16_t: index of object
- **addObj**
	- Takes
		- byte: register
		- uint8_t: x
		- uint8_t: y
		- uint8_t: w
		- uint8_t: v
	- Returns uint16_t: index of object
- **updateObjStyle**
	- Takes
		- uint16_t: index
		- uint8_t: value
	- Returns void
- **updateObj**
	- Takes
		- uint16_t: index
		- uint8_t: x
		- uint8_t: y
	- Returns void
- **updateObj**
	- Takes
		- uint16_t: index
		- uint8_t: x
		- uint8_t: y
		- uint8_t: w
		- uint8_t: v
	- Returns void

- **setBuffer**
	- Takes
		- uint8_t: bufX
		- uint8_t: bufY
	- Returns void

- **showBuffer**
	- Takes none
	- Returns void
- **draw**
	- Takes none
	- Returns void
- **drawTileBuffer**
	- Takes none
	- Returns void
- **drawText**
	- Takes
		- const char: string
		- int16_t: x
		- int16_t: y
		- boolean: negative [false]
		- boolean inverted [false]
	- Returns void
- **drawBitmap**
	- Takes
		- const byte: data
		- int16_t: x
		- int16_t: y
		- int16_t: width [8]
		- int16_t: height [8]
		- boolean: negative [false]
		- boolean inverted [false]
		- uint8_t: scale [1]
	- Returns void

- **charWidth**
	-Takes
		- uint8_t: char
	- Returns uint8_t
- **bufferTextWidth**
	-Takes
		- int16_t: string_index
	- Returns uint16_t
- **bufferTextWidth**
	-Takes none
	- Returns uint16_t
- **textWidth**
	-Takes
		- const char: string
	- Returns uint16_t

- **bufferBitmapHeight**
	-Takes
		- int16_t: bitmap_index
		- uint8_t: width
	- Returns uint16_t

- **clearTileBuffer**
	- Takes none
	- Returns void
- **clearBuffer**
	- Takes none
	- Returns void
- **clear**
	- Takes none
	- Returns void

## Data

- **DrawingObj**
	- byte register0
	 	- bits: 0-4 object type, 5-6 Invisible|Visible|Negative|Inverted, 7 Deleted
	- uint8_t x
		- will be adjusted to screen coordinates -64 < 192
	- uint8_t y
		- will be adjusted to screen coordinates -96 < 160
	- uint8_t w
	 	- width
	- uint8_t v
		- depending on object: height, (char/text/bitmap) index
- **DrawingObj Register Values**
	- object type
		- LINE_T: single pixel line from point to point
		- RECT_T: rectangle outline of 1px
		- RECTFILL_T: filled rectangle
		- ELLIPSE_T: ellipse outline of 1px
		- ELLIPSEFILL_T: filled ellipse
		- CHAR_T: single character (v is the value)
		- TEXT_T: string of text from list in Ease_OLED_I2C_128x64_Strings.h
		- BITMAP_T: bitmap from list in Ease_OLED_I2C_128x64_Bitmaps.h
	- drawing type
		- VISIBLE: draw object as positive, adding pixels to what is already in buffer
		- NEGATIVE: draw object as negative, deleting pixels from buffer
		- INVERTED: draw object as opposite of what is existing in buffer
	- reuse
		- DELETED: allow reuse of object in buffer
	- EXAMPLES
		- LINE_T|VISIBLE: line object, drawn positive
		- ELLIPSEFILL_T|INVERTED: filled ellipse, drawn inverted

## License

This library is licensed under the GNU GPLv3 (https://www.gnu.org/licenses/gpl.html) open source license.
Thus anybody is allowed to copy and modify the source code, provided all changes are open source too and the author is in knowledge of all changes.
This can happen either via eMail or directly on GitHub, in other words, on this repository.
