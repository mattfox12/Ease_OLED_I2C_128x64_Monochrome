#include <Ease_OLED_I2C_128x64_Monochrome.h>
#include <Wire.h>
#include <avr/pgmspace.h>

// Buttons, connected to digital inputs as listed below
#define UP 4
#define LEFT 3
#define RIGHT 5
#define DOWN 6
#define BUTTON 2

#define BLINK_TIME 1000 // cursor blink time (milliseconds)

DrawingObj objs[74]; // up to 74 objects can be on screen (increase if you add more)
Display lcd(objs, 74);

int cursor_index = -1;

long timer = 0;
int blink_timer = BLINK_TIME;

byte button_prev_register = 0x00;
byte button_down_register = 0x00;

uint8_t cursor_x = 0;
uint8_t cursor_y = 0;
boolean cursor_moved = false;
int8_t cursor_value = -1;

uint16_t bitmap_start = 0;
byte bitmap[8] = {0x70,0x89,0xB7,0x8D,0x81,0x97,0x89,0x70}; // initial image
char buf[4];

void setup() {
	Serial.begin(9600);
	while(!Serial);

	pinMode(UP, INPUT_PULLUP);
	pinMode(LEFT, INPUT_PULLUP);
	pinMode(RIGHT, INPUT_PULLUP);
	pinMode(DOWN, INPUT_PULLUP);
	pinMode(BUTTON, INPUT_PULLUP);

	lcd.init(true);
	lcd.rotateDisplay180(); // optional

	// TOP HEADER
	DrawingObj header;
	header.data.register0 = RECTFILL_T | VISIBLE;
	header.data.x = 8;
	header.data.y = 0;
	header.data.v0 = 112; // width
	header.data.v1 = 16; // height
	lcd.addObj(header);

	DrawingObj headerL;
	headerL.data.register0 = ELLIPSEFILL_T | VISIBLE;
	headerL.data.x = 0;
	headerL.data.y = 0;
	headerL.data.v0 = 16; // width
	headerL.data.v1 = 16; // height
	lcd.addObj(headerL);

	DrawingObj headerR;
	headerR.data.register0 = ELLIPSEFILL_T | VISIBLE;
	headerR.data.x = 112;
	headerR.data.y = 0;
	headerR.data.v0 = 16; // width
	headerR.data.v1 = 16; // height
	lcd.addObj(headerR);

	// DRAWING AREA
	DrawingObj border;
	border.data.register0 = RECTFILL_T | VISIBLE;
	border.data.x = 18;
	border.data.y = 19;
	border.data.v0 = 45; // width
	border.data.v1 = 45; // height
	lcd.addObj(border);

	DrawingObj stage;
	stage.data.register0 = RECTFILL_T | NEGATIVE;
	stage.data.x = 20;
	stage.data.y = 21;
	stage.data.v0 = 41; // width
	stage.data.v1 = 41; // height
	lcd.addObj(stage);

	// NEGATIVE PREVIEW
	DrawingObj double_neg_bg;
	double_neg_bg.data.register0 = RECTFILL_T | VISIBLE;
	double_neg_bg.data.x = 78;
	double_neg_bg.data.y = 42;
	double_neg_bg.data.v0 = 20; // width
	double_neg_bg.data.v1 = 20; // height
	lcd.addObj(double_neg_bg);

	DrawingObj neg_bg;
	neg_bg.data.register0 = RECTFILL_T | VISIBLE;
	neg_bg.data.x = 102;
	neg_bg.data.y = 46;
	neg_bg.data.v0 = 12; // width
	neg_bg.data.v1 = 12; // height
	lcd.addObj(neg_bg);

	DrawingObj line0;
	line0.data.register0 = LINE_T | VISIBLE;
	line0.data.x = double_neg_bg.data.x + double_neg_bg.data.v0 - 1;
	line0.data.y = double_neg_bg.data.y;
	line0.data.v0 = neg_bg.data.x + neg_bg.data.v0 - line0.data.x - 1; // width
	line0.data.v1 = neg_bg.data.y - line0.data.y; // height
	lcd.addObj(line0);

	DrawingObj line1;
	line1.data.register0 = LINE_T | VISIBLE;
	line1.data.x = line0.data.x;
	line1.data.y = double_neg_bg.data.y + double_neg_bg.data.v1 - 1;
	line1.data.v0 = line0.data.v0; // width
	line1.data.v1 = neg_bg.data.y + neg_bg.data.v1 - line1.data.y; // height
	lcd.addObj(line1);

	// draw squares based on bitmap bytes
	for (uint8_t y = 0; y < 8; y++) {
		for (uint8_t x = 0; x < 8; x++) {
			DrawingObj pixel;
			if (bitRead(bitmap[x], y)) pixel.data.register0 = RECTFILL_T | VISIBLE;
			else pixel.data.register0 = RECTFILL_T;
			pixel.data.x = 21 + x * 5;
			pixel.data.y = 22 + y * 5;
			pixel.data.v0 = 4; // width
			pixel.data.v1 = 4; // height
			uint16_t index = lcd.addObj(pixel);

			// save start index for bitmap squares
			if (bitmap_start == 0) bitmap_start = index;
		}
	}

	// cursor on top of all
	DrawingObj _cursor;
	_cursor.data.register0 = RECT_T | INVERTED;
	_cursor.data.x = 20 + cursor_x * 5;
	_cursor.data.y = 21 + cursor_y * 5;
	_cursor.data.v0 = 6; // width
	_cursor.data.v1 = 6; // height
	cursor_index = lcd.addObj(_cursor);

	lcd.draw();

	// show byte text in header
	for (uint8_t x = 0; x < 8; x++) {
		drawByteText(x);
	}

	// draw previews from bitmap
	lcd.drawBitmap(bitmap, 104, 24);
	lcd.drawBitmap(bitmap, 104, 48, 8, 8, false, true);
	lcd.drawBitmap(bitmap, 80, 21, 8, 8, false, false, 2);
	lcd.drawBitmap(bitmap, 80, 44, 8, 8, false, true, 2); // test errors for 81, 39

	timer = millis();
}

void loop() {
	// clear current buttons down
	button_down_register = 0x00;

	cursor_moved = false;
	// move cursor
	if (justPressed(UP)) {
		if (cursor_y <= 0) cursor_y = 7; // wrap to same line
		else cursor_y--;
		cursor_moved = true;
	}
	if (justPressed(DOWN)) {
		if (cursor_y >= 7) cursor_y = 0; // wrap to same line
		else cursor_y++;
		cursor_moved = true;
	}
	if (justPressed(LEFT)) {
		if (cursor_x <= 0) cursor_x = 7; // wrap to same line
		else cursor_x--;
		cursor_moved = true;
	}
	if (justPressed(RIGHT)) {
		if (cursor_x >= 7) cursor_x =0; // wrap to same line
		else cursor_x++;
		cursor_moved = true;
	}

	if (cursor_index >= 0) {
		if (cursor_moved) {
			// also change the bitmap
			if (isDown(BUTTON)) change_bitmap(cursor_x, cursor_y, cursor_value);

			// update x,y
			lcd.updateObj(cursor_index, 20 + cursor_x * 5, 21 + cursor_y * 5);

			// set to show immediately
			lcd.updateObjStyle(cursor_index, INVERTED);
			blink_timer = BLINK_TIME;
		} else if (blink_timer <= 0) {
			byte register0 = objs[cursor_index].data.register0;
			if ((INVERTED & objs[cursor_index].data.register0) == INVERTED) lcd.updateObjStyle(cursor_index, 0);
			else lcd.updateObjStyle(cursor_index, INVERTED);

			lcd.updateObj(cursor_index, objs[cursor_index].data.x, objs[cursor_index].data.y, objs[cursor_index].data.v0, objs[cursor_index].data.v1);
			blink_timer = BLINK_TIME;
		}
	}

	if (justPressed(BUTTON)) change_bitmap(cursor_x, cursor_y, -1);

	// uncomment next three lines to see rendering optimization
	//  lcd.clear();
	//  lcd.showBuffer();
	//  delay(1000);

	// draw to screan
	lcd.draw();

	// remember button values for next frame
	button_prev_register = button_down_register;

	// increase time
	long current_millis = millis();
	blink_timer -= current_millis-timer;
	timer = current_millis;
}

bool isDown(byte button) {
	if (digitalRead(button) == LOW) return true;
	return false;
}

bool justPressed(byte button) {
	if (isDown(button)) {
		bitSet(button_down_register, button);
		if (!bitRead(button_prev_register, button)) return true;
	}
	return false;
}

void drawByteText(uint8_t index) {
	itoa(bitmap[index], buf, 16);

	if (strlen(buf) < 2) {
		buf[1] = buf[0];
		buf[0] = '0';
	}

	buf[0] = toupper(buf[0]);
	buf[1] = toupper(buf[1]);

	lcd.drawText(buf, 3 + 16 * index, 5, true);
}

void change_bitmap(uint8_t x, uint8_t y, int8_t change) {
	// change bitmap
	boolean cur_bit = false;
	if (change == -1) {
		cur_bit = bitRead(bitmap[x], y); // toggle
		cursor_value = cur_bit; // remember if we move the cursor
	} else cur_bit = (boolean)change;

	if (cur_bit) {
		bitClear(bitmap[x], y);

		// update drawings
		lcd.updateObjStyle(bitmap_start + x + y * 8, NEGATIVE);
	} else {
		bitSet(bitmap[x], y);

		// update drawings
		lcd.updateObjStyle(bitmap_start + x + y * 8, VISIBLE);
	}

	// draw from bitmap
	lcd.drawBitmap(bitmap, 104, 24);
	lcd.drawBitmap(bitmap, 104, 48, 8, 8, false, true);
	lcd.drawBitmap(bitmap, 80, 21, 8, 8, false, false, 2);
	lcd.drawBitmap(bitmap, 80, 44, 8, 8, false, true, 2);

	// draw text
	drawByteText(x);

	// show cursor immediately
	lcd.updateObjStyle(cursor_index, INVERTED);
	blink_timer = BLINK_TIME;
}
