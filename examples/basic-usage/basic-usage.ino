#include <Ease_OLED_I2C_128x64_Monochrome.h>

DrawingObj objs[16]; // up to 16 objects can be on screen
Display lcd(objs, 16);

int item_index = -1;
int8_t dirX = 1;
int8_t dirY = 1;

void setup() {
	Serial.begin(9600);
	while(!Serial);

	lcd.init(true);
	lcd.rotateDisplay180(); // optional

	DrawingObj item;
	// choose a register to draw that type of shape
	item.data.register0 = RECTFILL_T | VISIBLE;
	// item.data.register0 = RECT_T | VISIBLE;
	// item.data.register0 = ELLIPSEFILL_T | VISIBLE;
	// item.data.register0 = ELLIPSE_T | VISIBLE;
	// item.data.register0 = LINE_T | VISIBLE;
	item.data.x = 4;
	item.data.y = 20;
	item.data.w = 18; // width
	item.data.v = 18; // height
	item_index = lcd.addObj(item); // pass in DrawingObj, values copied to 1st available, returns index

	lcd.draw();
}

void loop() {
	// update object position
	if (item_index >= 0) {
		int8_t newX = objs[item_index].data.x;
    	if (newX == 0 || newX == 128 - objs[item_index].data.w) dirX *= -1;
		newX += dirX;
		int8_t newY = objs[item_index].data.y;
    	if (newY == 17 || newY == 64 - objs[item_index].data.v) dirY *= -1;
		newY += dirY;

		// use updateObj to be sure buffers are correctly redrawn
		lcd.updateObj(item_index, newX, newY); // optional: add width, height values as well
	}

	// uncomment next two lines to see where buffer causes redraw
	//  lcd.showBuffer();
	//  delay(1000);

	lcd.draw();
	delay(500);
}
