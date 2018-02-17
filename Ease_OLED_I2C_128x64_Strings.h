// #############################################################################
// ##### OLED STRINGS FILE #####################################################
// #############################################################################

// Create constant strings here
const char string_0[] PROGMEM = "Hello";   // "Hello" etc are strings to store - change to suit.
const char string_1[] PROGMEM = "World!";

// include each of the strings created above, referenced in DrawingObj by index within this table
const char* const string_table[] PROGMEM = {string_0, string_1};
