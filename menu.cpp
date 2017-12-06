#include <Arduino.h>
#include <Adafruit_ILI9341.h>
#include <string.h>
//#include "menu.h"

#define TFT_DC 9
#define TFT_CS 10
#define SD_CS 6

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

#define DISPLAY_WIDTH  320
#define DISPLAY_HEIGHT 240

//Joystick for this arduino
#define JOY_VERT_1 A1
#define JOY_HORIZ_1 A0
#define JOY_SEL_1   2

#define JOY_CENTER   512
#define JOY_DEADZONE 32

#define BORDER 2

using namespace std;

uint8_t textSize = 2;
uint8_t space = 20;

void drawSplashScreen();

void setup(){
	init();

	pinMode(JOY_SEL_1, INPUT_PULLUP);

	Serial.begin(9600);
	tft.begin();

	tft.setRotation(3);
	drawSplashScreen();
}

char joystick() {
	int xVal = analogRead(JOY_HORIZ_1);
	int yVal = analogRead(JOY_VERT_1);
	char direction;

	//if the joystick has been moved left
	if(xVal > JOY_CENTER + JOY_DEADZONE){
		direction = 'l';
	}
	//if the joystick has been moved right
	else if( xVal < JOY_CENTER - JOY_DEADZONE){
		direction = 'r';
	}
	if(yVal < JOY_CENTER - JOY_DEADZONE){
		direction = 'u';
	}
	//if the joystick has been pushed down
	else if(yVal > JOY_CENTER + JOY_DEADZONE){
		direction = 'd';
	}
	if (digitalRead(JOY_SEL_1) == LOW){
		direction = 'p';
	}
	return direction;
}

void drawMenu();

void drawSplashScreen(){
	tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
	tft.setCursor(DISPLAY_WIDTH/2-5*textSize*sizeof("Arduino Air Hockey"), DISPLAY_HEIGHT/2 - space - 7*textSize);
	tft.print("Arduino Air Hockey");

	tft.setCursor(DISPLAY_WIDTH/2-5*(textSize-1)*sizeof("-- Press Right Joystick Buttonn to Start --"), DISPLAY_HEIGHT/2 + space);
	tft.setTextSize(textSize-1);
	tft.print("-- Press Right Joystick Button to Start --");

	while(true) {
		if (joystick() == 'p') {
			drawMenu();
		}
	}
}

void menuSelect(uint16_t colour[]);

int bestOfSelect() {
	int select = 1;
	while(true) {
		//char direction = Serial3.read();
		char direction = joystick();
		if (direction == 'r' && select<3) {
			tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
			tft.setCursor((DISPLAY_WIDTH - (5*textSize*sizeof("Player 1 Colour: ") + 2*space))*select/4, space*3+7*(textSize+1));
			tft.print(2*select+1);

			select ++;
			tft.setTextColor(ILI9341_BLACK, ILI9341_WHITE);
			tft.setCursor((DISPLAY_WIDTH - (5*textSize*sizeof("Player 1 Colour: ") + 2*space))*select/4, space*3+7*(textSize+1));

		}
		else if (direction == 'l' && select>1) {
			tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
			tft.setCursor((DISPLAY_WIDTH - (5*textSize*sizeof("Player 1 Colour: ") + 2*space))*select/4, space*3+7*(textSize+1));
			tft.print(2*select+1);

			select --;
			tft.setTextColor(ILI9341_BLACK, ILI9341_WHITE);
			tft.setCursor((DISPLAY_WIDTH - (5*textSize*sizeof("Player 1 Colour: ") + 2*space))*select/4, space*3+7*(textSize+1));
		}
		else if (direction =='l' && select==1) {
			tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
			tft.setCursor((DISPLAY_WIDTH - (5*textSize*sizeof("Player 1 Colour: ") + 2*space))*select/4, space*3+7*(textSize+1));
			tft.print(2*select+1);

			return 5;//therefore use default
		}
		if (direction == 'p') {
			tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
			tft.setCursor((DISPLAY_WIDTH - (5*textSize*sizeof("Player 1 Colour: ") + 2*space))*select/4, space*3+7*(textSize+1));

			return 2*select+1;
		}
	}
}

uint8_t playerColourSelect(uint8_t pNum, uint16_t colour[]) {
	uint8_t select = 0;
	while (true) {
		//char direction = Serial3.read();
		char direction = joystick();
		if (direction=='r' && select<6) {
			tft.fillRect(((DISPLAY_WIDTH - (5*textSize*sizeof("Player 1 Colour: ") + 2*space))*(select+1)/7), //top right x coordingate
										space*(3+2*pNum)+7*(textSize+1)+pNum*7*(textSize), //top left y coordinate
										7*textSize, 7*textSize, ILI9341_BLACK);
			tft.fillRect(((DISPLAY_WIDTH - (5*textSize*sizeof("Player 1 Colour: ") + 2*space))*(select+1)/7), //top right x coordingate
										space*(3+2*pNum)+7*(textSize+1)+pNum*7*(textSize), //top left y coordinate
										5*textSize, 5*textSize, colour[select]);

			select ++;
			tft.fillRect(((DISPLAY_WIDTH - (5*textSize*sizeof("Player 1 Colour: ") + 2*space))*(select+1)/7), //top right x coordingate
										space*(3+2*pNum)+7*(textSize+1)+pNum*7*(textSize), //top left y coordinate
										7*textSize, 7*textSize, ILI9341_WHITE);
			tft.fillRect(((DISPLAY_WIDTH - (5*textSize*sizeof("Player 1 Colour: ") + 2*space))*(select+1)/7), //top right x coordingate
										space*(3+2*pNum)+7*(textSize+1)+pNum*7*(textSize), //top left y coordinate
										5*textSize, 5*textSize, colour[select]);
		}
		if (direction=='l' && select>0) {
			tft.fillRect(((DISPLAY_WIDTH - (5*textSize*sizeof("Player 1 Colour: ") + 2*space))*(select+1)/7), //top right x coordingate
										space*(3+2*pNum)+7*(textSize+1)+pNum*7*(textSize), //top left y coordinate
										7*textSize, 7*textSize, ILI9341_BLACK);
			tft.fillRect(((DISPLAY_WIDTH - (5*textSize*sizeof("Player 1 Colour: ") + 2*space))*(select+1)/7), //top right x coordingate
										space*(3+2*pNum)+7*(textSize+1)+pNum*7*(textSize), //top left y coordinate
										5*textSize, 5*textSize, colour[select]);

			select--;
			tft.fillRect(((DISPLAY_WIDTH - (5*textSize*sizeof("Player 1 Colour: ") + 2*space))*(select+1)/7), //top right x coordingate
										space*(3+2*pNum)+7*(textSize+1)+pNum*7*(textSize), //top left y coordinate
										7*textSize, 7*textSize, ILI9341_WHITE);
			tft.fillRect(((DISPLAY_WIDTH - (5*textSize*sizeof("Player 1 Colour: ") + 2*space))*(select+1)/7), //top right x coordingate
										space*(3+2*pNum)+7*(textSize+1)+pNum*7*(textSize), //top left y coordinate
										5*textSize, 5*textSize, colour[select]);
		}
		if (direction=='l' && select==0) {
			tft.fillRect(((DISPLAY_WIDTH - (5*textSize*sizeof("Player 1 Colour: ") + 2*space))*(select+1)/7), //top right x coordingate
										space*(3+2*pNum)+7*(textSize+1)+pNum*7*(textSize), //top left y coordinate
										7*textSize, 7*textSize, ILI9341_BLACK);
			tft.fillRect(((DISPLAY_WIDTH - (5*textSize*sizeof("Player 1 Colour: ") + 2*space))*(select+1)/7), //top right x coordingate
										space*(3+2*pNum)+7*(textSize+1)+pNum*7*(textSize), //top left y coordinate
										5*textSize, 5*textSize, colour[select]);
			if (pNum == 1) {
				return 4; //player 1 is blue by default
			}
			else if (pNum == 2) {
				return 1; //player 2 is red by default
			}
		}
		if (direction == 'p') {
			tft.fillRect(((DISPLAY_WIDTH - (5*textSize*sizeof("Player 1 Colour: ") + 2*space))*(select+1)/7), //top right x coordingate
										space*(3+2*pNum)+7*(textSize+1)+pNum*7*(textSize), //top left y coordinate
										7*textSize, 7*textSize, ILI9341_BLACK);
			tft.fillRect(((DISPLAY_WIDTH - (5*textSize*sizeof("Player 1 Colour: ") + 2*space))*(select+1)/7), //top right x coordingate
										space*(3+2*pNum)+7*(textSize+1)+pNum*7*(textSize), //top left y coordinate
										5*textSize, 5*textSize, colour[select]);

			return select; //return the character representing the selected colour
		}
	}
}

//draws the Player's new score when they are updated
void drawScores(uint8_t score, uint8_t player) {
	if (player == 1) {
		tft.setCursor(DISPLAY_WIDTH/4-5*textSize/2, space*3+8*textSize*2);
	}
	else if (player == 2) {
		tft.setCursor(DISPLAY_WIDTH/4-5*textSize/2, space*3+8*textSize*2);
	}
	tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
	tft.print(score);
}

//draws the players chosen names and starting scores, as well the scoring mode
void drawScoreScreen(uint16_t p1Colour, uint16_t p2Colour, uint8_t bestOf) {

	tft.fillScreen(ILI9341_BLACK);

	tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
	tft.setTextSize(textSize);
	tft.setTextWrap(0);

	//print the scoring mode
	tft.setCursor(DISPLAY_WIDTH/2-5*textSize*(sizeof("Best of: ")+1)/2, space);
	tft.print("Best of: ");
	tft.print(bestOf);

	//print the chosen name of Player 1 to the left
	tft.setCursor(DISPLAY_WIDTH/4-5*textSize*sizeof("Player 1")/2, space*2 +8*textSize);
	tft.setTextColor(p1Colour);
	tft.print("Player 1");

	//print the chosen name of Player 2 to the right
	tft.setCursor(DISPLAY_WIDTH/4-5*textSize*sizeof("Player 2")/2, space*2 +8*textSize);
	tft.setTextColor(p2Colour);
	tft.print("Player 2");

	//draw the starting scores of each player
	drawScores(0, 1);
	drawScores(0, 2);
}

//draw the congragulatory sentence when a player wins and display a reset button
void Win(char pName, uint8_t player, bool pressed){
	uint16_t GREY = tft.color565(211,211,211);

	tft.setCursor(DISPLAY_WIDTH/2-5*textSize*sizeof(pName)/2, DISPLAY_HEIGHT-space*4-8*textSize*2);
	tft.setTextColor(ILI9341_WHITE);
	tft.print(pName);
	tft.print(" Won!");

	tft.fillRect(DISPLAY_WIDTH/2-5*textSize*sizeof("Reset")/2-space/2, DISPLAY_HEIGHT-space*3-8*textSize*2, space*2+5*textSize*sizeof("Reset"), space*2+7*textSize, GREY);
	tft.drawRect(DISPLAY_WIDTH/2-5*textSize*sizeof("Reset")/2-space/2, DISPLAY_HEIGHT-space*3-8*textSize*2, space*2+5*textSize*sizeof("Reset"), space*2+7*textSize, ILI9341_WHITE);
	tft.setCursor(DISPLAY_WIDTH/2-5*textSize*sizeof("Reset")/2, DISPLAY_HEIGHT-space*3-8*textSize*2);

	if (pressed) { //If right joystick button is pressed
		tft.fillRect(DISPLAY_WIDTH/2-5*textSize*sizeof("Reset")/2-space/2, DISPLAY_HEIGHT-space*3-8*textSize*2, space*2+5*textSize*sizeof("Reset"), space*2+7*textSize, ILI9341_WHITE);
		tft.drawRect(DISPLAY_WIDTH/2-5*textSize*sizeof("Reset")/2-space/2, DISPLAY_HEIGHT-space*3-8*textSize*2, space*2+5*textSize*sizeof("Reset"), space*2+7*textSize, GREY);
		tft.setTextColor(GREY);
		tft.setCursor(DISPLAY_WIDTH/2-5*textSize*sizeof("Reset")/2, DISPLAY_HEIGHT-space*3-8*textSize*2);

		delay(250);

		tft.fillScreen(ILI9341_BLACK);
	}
}

void drawMenu(){
	tft.fillScreen(ILI9341_BLACK);

	tft.setTextColor(ILI9341_WHITE);
	tft.setTextSize(textSize+1);
	tft.setTextWrap(0);

	tft.setCursor(space, space);
	tft.print("Menu");

	tft.setCursor(space, space*3+7*(textSize+1));
	tft.setTextSize(textSize);
	tft.print("Best of: ");

	tft.setCursor(space, space*5+7*(textSize+1)+7*(textSize));
	tft.print("Player 1 Colour: ");

	tft.setCursor(space, space*7+7*(textSize+1)+2*7*(textSize));
	tft.print("Player 2 Colour: ");

	//Vertical line between setting names and options
	tft.fillRect(5*textSize*sizeof("Player 1 Colour: ") + 2*space, 0, BORDER, DISPLAY_HEIGHT, ILI9341_WHITE);
	//Horizontal lines between names
	tft.fillRect(0, 7*(textSize+1) + 2*space, DISPLAY_WIDTH, BORDER, ILI9341_WHITE);
	tft.fillRect(0, 7*(textSize+1) +7*textSize + 4*space, DISPLAY_WIDTH, BORDER, ILI9341_WHITE);
	tft.fillRect(0, 7*(textSize+1) +2*7*textSize + 6*space, DISPLAY_WIDTH, BORDER, ILI9341_WHITE);
	tft.fillRect(0, 7*(textSize+1) +3*7*textSize + 8*space, DISPLAY_WIDTH, BORDER, ILI9341_WHITE);

	//print the "best of" options
	tft.setCursor((DISPLAY_WIDTH - (5*textSize*sizeof("Player 1 Colour: ") + 2*space))/4, space*3+7*(textSize+1));
	tft.print("3");
	tft.setCursor((DISPLAY_WIDTH - (5*textSize*sizeof("Player 1 Colour: ") + 2*space))/2, space*3+7*(textSize+1));
	tft.print("5");
	tft.setCursor((DISPLAY_WIDTH - (5*textSize*sizeof("Player 1 Colour: ") + 2*space))*3/4, space*3+7*(textSize+1));
	tft.print("7");

	uint16_t colour[6] = {ILI9341_MAGENTA, ILI9341_RED, ILI9341_YELLOW, ILI9341_GREEN, ILI9341_BLUE, ILI9341_CYAN};
	//Print the player 1 and Player 2 colour options
	for (int i=1; i<=2; i++) {
		for (int j=0; j<6; j++) {
			tft.fillRect(((DISPLAY_WIDTH - (5*textSize*sizeof("Player 1 Colour: ") + 2*space))*(j+1)/7), //top right x coordingate
										space*(3+2*i)+7*(textSize+1)+i*7*(textSize), //top left y coordinate
										7*textSize, 7*textSize, ILI9341_BLACK);
			tft.fillRect(((DISPLAY_WIDTH - (5*textSize*sizeof("Player 1 Colour: ") + 2*space))*(j+1)/7), //top right x coordingate
										space*(3+2*i)+7*(textSize+1)+i*7*(textSize), //top left y coordinate
										5*textSize, 5*textSize, colour[j]);
		}
	}
			/*
			tft.setCursor((DISPLAY_WIDTH - (5*textSize*sizeof("Player 1 Colour: ") + 2*space))/6, space*(3+2*i)+7*(textSize+1)+i*7*(textSize));
			tft.print("Magenta");
			tft.setCursor((DISPLAY_WIDTH - (5*textSize*sizeof("Player 1 Colour: ") + 2*space))/3, space*(3+2*i)+7*(textSize+1)+i*7*(textSize));
			tft.print("Red");
			tft.setCursor(space, space*5+7*(textSize+1)+7*(textSize));
			tft.print("Yellow");
			tft.setCursor((DISPLAY_WIDTH - (5*textSize*sizeof("Player 1 Colour: ") + 2*space))/2, space*(3+2*i)+7*(textSize+1)+i*7*(textSize));
			tft.print("Green");
			tft.setCursor((DISPLAY_WIDTH - (5*textSize*sizeof("Player 1 Colour: ") + 2*space))*2/3, space*(3+2*i)+7*(textSize+1)+i*7*(textSize));
			tft.print("Blue");
			tft.setCursor((DISPLAY_WIDTH - (5*textSize*sizeof("Player 1 Colour: ") + 2*space))*5/6, space*(3+2*i)+7*(textSize+1)+i*7*(textSize));
			tft.print("Cyan");
			*/

	//print the play button and highlight it by default
	tft.setCursor(space, DISPLAY_HEIGHT-space-7*textSize);
	tft.setTextColor(ILI9341_BLACK, ILI9341_WHITE);
	tft.print("Play");

	menuSelect(colour);
}


void menuSelect(uint16_t colour[]) {
	uint8_t select = 3; //select the play button by default
	int bestOf = 5;			//game is best of 5 by default
	uint8_t p1Colour = 4; //player 1 is blue by default
	uint8_t p2Colour = 1;	//player 2 is red by default
	char colourCharacter[] = {'m', 'r', 'y', 'g', 'b', 'c'};

	str setting[] = {"Best Of: ", "Player 1 Colour", "Player 2 Colour", "Play"};

	while(true) {
		//char direction = Serial3.read();
		char direction = joystick();
		if (direction == 'd' && select <3) {
			tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
			tft.setCursor(space, space*(3+2*select)+7*(textSize+1) + select*7*textSize);
			tft.print(setting[select]);

			setting++;
			tft.setTextColor(ILI9341_BLACK, ILI9341_WHITE);
			if (select == 3) {
				tft.setCursor(space, DISPLAY_HEIGHT-space-7*textSize);
			}
			else {
				tft.setCursor(space, space*(3+2*select)+7*(textSize+1) + select*7*textSize);
			}
			tft.print(setting[select]);
		}
		else if (direction == 'u' && select>0) {
			if (select == 3) {
				tft.setCursor(space, DISPLAY_HEIGHT-space-7*textSize);
			}
			else {
				tft.setCursor(space, space*(3+2*select)+7*(textSize+1) + select*7*textSize);
			}
			tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
			tft.print(setting[select]);

			setting--;
			tft.setTextColor(ILI9341_BLACK, ILI9341_WHITE);
			tft.setCursor(space, space*(3+2*select)+7*(textSize+1) + select*7*textSize);
			tft.print(setting[select]);
		}
		else if (direction == 'r') {
			if (select == 0) {
				bestOf = bestOfSelect();
			}
			else if (select == 1) {
				p1Colour = playerColourSelect(1, colour);
			}
			else if (select == 2) {
				p2Colour = playerColourSelect(2, colour);
			}
			else if (select == 3) {
				Serial3.write(bestOf);
				Serial3.write(colourCharacter[p1Colour]);
				Serial3.write(colourCharacter[p2Colour]);
				//play the game
				drawScoreScreen(colour[p1Colour], colour[p2Colour], bestOf);
			}
		}
	}
}

int main(){
	setup();

	drawSplashScreen();

	return 0;
}
