#include <Ease_OLED_I2C_128x64_Monochrome.h>
#include <Wire.h>
#include <avr/pgmspace.h>

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

	DrawingObj hello;
	hello.data.register0 = TEXT_T | INVERTED;
	hello.data.x = 4;
	hello.data.y = 20;
	hello.data.v0 = 0; // string_table index, see: Ease_OLED_I2C_128x64_Strings.h
	hello.data.v1 = 8; // height, TODO: maybe change to wrap width?
	hello_index = lcd.addObj(hello); // pass in DrawingObj, values copied to 1st available, returns index

	DrawingObj world;
	world.data.register0 = TEXT_T | INVERTED;
	world.data.x = 5;
	world.data.y = 28;
	world.data.v0 = 1; // string_table index, see: Ease_OLED_I2C_128x64_Strings.h
	world.data.v1 = 8; // height, TODO: maybe change to wrap width?
	world_index = lcd.addObj(world); // pass in DrawingObj, values copied to 1st available, returns index

	lcd.draw();
}

void loop() {
	// update object position
	if (hello_index >= 0) {
		int8_t newX = objs[hello_index].data.x;
    	if (newX == 0 || newX == 128 - lcd.bufferTextWidth(hello_index)) h_dir_x *= -1;
		newX += h_dir_x;
		int8_t newY = objs[hello_index].data.y;
    	if (newY == 17 || newY == 64 - objs[hello_index].data.v1) h_dir_y *= -1;
		newY += h_dir_y;
		lcd.updateObj(hello_index, newX, newY);
	}
	if (world_index >= 0) {
		int8_t newX = objs[world_index].data.x;
    	if (newX == 0 || newX == 128 - lcd.bufferTextWidth(world_index)) w_dir_x *= -1;
		newX += w_dir_x;
		int8_t newY = objs[world_index].data.y;
    	if (newY == 17 || newY == 64 - objs[world_index].data.v1) w_dir_y *= -1;
		newY += w_dir_y;
		lcd.updateObj(world_index, newX, newY);
	}

	// uncomment next two lines to see where buffer causes redraw
	//  lcd.showBuffer();
	//  delay(1000);

	lcd.draw();
	delay(500);
}
