#include <Ease_OLED_I2C_128x64_Monochrome.h>

DrawingObj objs[16]; // up to 16 objects can be on screen
Display lcd(objs, 16);

int hello_index = -1;
int8_t h_dir_x = 1;
int8_t h_dir_y = 1;
int world_index = -1;
int8_t w_dir_x = 1;
int8_t w_dir_y = 1;

void setup() {
	Serial.begin(9600);
	while(!Serial);

	lcd.init(true);
	lcd.rotateDisplay180(); // optional

	hello_index = lcd.addObj(TEXT_T | INVERTED, 4, 20, 0, 0); // register, x, y, w, string_table index: see Ease_OLED_I2C_128x64_Strings.h
	world_index = lcd.addObj(TEXT_T | INVERTED, 5, 28, 0, 1); // register, x, y, w, string_table index

	lcd.draw();
}

void loop() {
	// update object position
	if (hello_index >= 0) {
		int8_t newX = objs[hello_index].data.x;
    	if (newX == 0 || newX == 128 - lcd.bufferTextWidth(hello_index)) h_dir_x *= -1;
		newX += h_dir_x;
		int8_t newY = objs[hello_index].data.y;
    	if (newY == 17 || newY == 64 - 8) h_dir_y *= -1;
		newY += h_dir_y;

		// use updateObj to be sure buffers are correctly redrawn
		lcd.updateObj(hello_index, newX, newY);
	}
	if (world_index >= 0) {
		int8_t newX = objs[world_index].data.x;
    	if (newX == 0 || newX == 128 - lcd.bufferTextWidth(world_index)) w_dir_x *= -1;
		newX += w_dir_x;
		int8_t newY = objs[world_index].data.y;
    	if (newY == 17 || newY == 64 - 8) w_dir_y *= -1;
		newY += w_dir_y;

		// use updateObj to be sure buffers are correctly redrawn
		lcd.updateObj(world_index, newX, newY);
	}

	// uncomment next two lines to see where buffer causes redraw
	//  lcd.showBuffer();
	//  delay(1000);

	lcd.draw();
	delay(500);
}
