// #############################################################################
// ##### OLED BITMAPS FILE #####################################################
// #############################################################################

// Create constant bitmaps here, just a list of bytes
const PROGMEM byte bitmap_table[]  = {
	0x3C,0x42,0x81,0x9D,0x91,0x91,0x42,0x3C,	// clock
	0x81,0xC3,0xB5,0xCD,0xCD,0xB5,0xC3,0x81,	// hourglass
	0x60,0x9E,0x81,0x9E,0x60,					// thermometer
	0x3C,0xC4,0x8F,0x01,0x07,0x01,0xC7,0x3E,	// hold
	0x3C,0x3C,0xFF,0x7E,0x3C,0x18,				// arrow
	0xFF,0x7E,0x3C,0x18, // play
	0xFF,0xFF,0x00,0xFF,0xFF, // pause
	0x24,0x18,0xFF,0x5A,0x24, // Bluetooth
	0x20,0x64,0xF2,0x42,0x42,0x4F,0x26,0x04 // Repeat
};

// specify the [index, length] of each of the bitmaps listed in bitmap_table
const PROGMEM uint16_t bitmap_table_index[][2] = {
	{ 0, 8 },	// clock, index 0, length 8
	{ 8, 8 },	// hourglass, index 8, length 8
	{ 16, 5 },	// thermometer, index 8, length 8
	{ 21, 8 },	// hold, index 21, length 8
	{ 29, 6 },	// arrow, index 29, length 6
	{ 35, 4 },	// play, index 35, length 4
	{ 39, 5 },	// pause, index 39, length 5
	{ 44, 5 },	// Bluetooth, index 44, length 5
	{ 49, 8 },	// Repeat, index 48, length 8
};
