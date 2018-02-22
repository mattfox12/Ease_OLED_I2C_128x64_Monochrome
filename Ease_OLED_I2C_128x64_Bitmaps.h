// #############################################################################
// ##### OLED BITMAPS FILE #####################################################
// #############################################################################

// Create constant bitmaps here, just a list of bytes
const char bitmap_table[] PROGMEM = {
  0xF0,0x09,0x37,0x0D,0x01,0x01,0x01,0x01,0x01,0x17,0x09,0xF0,
  0x07,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x07
};   // change to suit. Should look like a large jar.

// specify the index and length of each of the bitmaps listed in bitmap_table
const uint8_t bitmap_table_index[][2] PROGMEM = {
	{ 0, 24 },	// jar, index 0, length 24
};
