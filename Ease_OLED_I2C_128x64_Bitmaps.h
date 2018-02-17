// #############################################################################
// ##### OLED BITMAPS FILE #####################################################
// #############################################################################

// Create constant strings here
const char bitmap_0[] PROGMEM = {
  0xF0,0x09,0x37,0x0D,0x01,0x01,0x01,0x01,0x01,0x17,0x09,0xF0,
  0xF7,0xF8,0xF8,0xF8,0xF8,0xF8,0xF8,0xF8,0xF8,0xF8,0xF8,0xF7
};   // change to suit. Should look like a large jar.

// include each of the strings created above, referenced in DrawingObj by index within this table
const char* const bitmap_table[] PROGMEM = {bitmap_0};
