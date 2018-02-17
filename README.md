# Ease OLED Library

This is a library for an 0,96" monochrome organic liquid crystal display (128x64 pixels)
It is modified from https://github.com/deloarts/OLED_I2C_128x64_Monochrome_Library and was created
to allow pixel perfect drawing at low memory cost, as well as tighter font usage.
It has been tested with the following device:
	- 0,96" I2C OLED, Manufacturer: Heltec-Taobao on ATmega 328P

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
		- uint8_t: v0
		- uint8_t: v1
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

## License

This library is licensed under the GNU GPLv3 (https://www.gnu.org/licenses/gpl.html) open source license.
Thus anybody is allowed to copy and modify the source code, provided all changes are open source too and the author is in knowledge of all changes.
This can happen either via eMail or directly on GitHub, in other words, on this repository.
