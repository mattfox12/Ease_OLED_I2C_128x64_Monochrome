#include <Ease_OLED_I2C_128x64_Monochrome.h>
#include <Wire.h>
#include <avr/pgmspace.h>

DrawingObj objs[16]; // up to 16 objects can be on screen
Display lcd(objs, 16);

int bitmap_index = -1;
int8_t dirX = 1;
int8_t dirY = 1;

void setup() {
	Serial.begin(9600);
	while(!Serial);

	lcd.init(true);
	lcd.rotateDisplay180(); // optional

	DrawingObj bitmap;
	bitmap.data.register0 = BITMAP_T | VISIBLE;
	bitmap.data.x = 4;
	bitmap.data.y = 20;
	bitmap.data.v0 = 0; // bitmap_table index, see: Ease_OLED_I2C_128x64_Bitmaps.h
	bitmap.data.v1 = 12; // wrap width
	bitmap_index = lcd.addObj(bitmap); // pass in DrawingObj, values copied to 1st available, returns index

	lcd.draw();
}

void loop() {
	// update object position
	if (bitmap_index >= 0) {
		int8_t newX = objs[bitmap_index].data.x;
    	if (newX == 0 || newX == 128 - objs[bitmap_index].data.v1) dirX *= -1;
		newX += dirX;
		int8_t newY = objs[bitmap_index].data.y;
    	if (newY == 17 || newY == 64 - lcd.bufferBitmapHeight(bitmap_index,objs[bitmap_index].data.v1)) dirY *= -1;
		newY += dirY;
		lcd.updateObj(bitmap_index, newX, newY);
	}

	// uncomment next two lines to see where buffer causes redraw
	//  lcd.showBuffer();
	//  delay(1000);

	lcd.draw();
	delay(500);
}
