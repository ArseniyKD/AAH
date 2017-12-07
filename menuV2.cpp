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

#define BORDER 2

using namespace std;

uint8_t textSize = 1;
uint8_t space = 10;
int del = 250;

void drawSplashScreen();

/*
void setup(){
	init();

	pinMode(JOY_SEL_1, INPUT_PULLUP);

	Serial.begin(9600);
	tft.begin();

	tft.setRotation(3);
	drawSplashScreen();
}
*/

/*
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
	//if the joystick has been pushed down
	else if(yVal > JOY_CENTER + JOY_DEADZONE){
		direction = 'd';
	}
	if(yVal < JOY_CENTER - JOY_DEADZONE){
		direction = 'u';
	}
	if (digitalRead(JOY_SEL_1) == LOW){
		direction = 'p';
	}
	return direction;
}
*/

void drawMenu();

void drawSplashScreen(){
	tft.fillScreen(ILI9341_BLACK);
	tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
	tft.setTextSize(textSize+2);
	tft.setTextWrap(false);
	tft.setCursor((DISPLAY_WIDTH-6*(textSize+2)*sizeof("Arduino"))/2, DISPLAY_HEIGHT/2 - space - 2*8*(textSize+2));
	tft.print("Arduino");
	tft.setCursor((DISPLAY_WIDTH-6*(textSize+2)*sizeof("Air Hockey"))/2, DISPLAY_HEIGHT/2 - space - 8*(textSize+2));
	tft.print("Air Hockey");

	tft.setCursor(DISPLAY_WIDTH/2-(6*(textSize)*sizeof("-- Press Right Joystick Buttonn to Start --"))/2, DISPLAY_HEIGHT/2 + space);
	tft.setTextSize(textSize);
	tft.print("-- Press Right Joystick Button to Start --");

	while(true) {
		if (Serial3.read() == 'p') {
			drawMenu();
		}
	}
}

void menuSelect(uint16_t colour[]);

void drawHighlightBestOf(uint8_t select) {
	tft.fillRect(DISPLAY_WIDTH - (DISPLAY_WIDTH-(6*textSize*sizeof("Player 1 Colour: ") + 2*space))*(4-select)/4-2*textSize, space*3+7*(textSize+1)-2*textSize,
							9*textSize, 11*textSize, ILI9341_WHITE);
	tft.setCursor(DISPLAY_WIDTH - (DISPLAY_WIDTH-(6*textSize*sizeof("Player 1 Colour: ") + 2*space))*(4-select)/4, space*3+7*(textSize+1));
	tft.setTextColor(ILI9341_BLACK);

	tft.print(2*select+1);
}

void redrawBestOf(uint8_t select) {
	tft.fillRect(DISPLAY_WIDTH - (DISPLAY_WIDTH-(6*textSize*sizeof("Player 1 Colour: ") + 2*space))*(4-select)/4-2*textSize, space*3+7*(textSize+1)-2*textSize,
							9*textSize, 11*textSize, ILI9341_BLACK);
	tft.setCursor(DISPLAY_WIDTH - (DISPLAY_WIDTH-(6*textSize*sizeof("Player 1 Colour: ") + 2*space))*(4-select)/4, space*3+7*(textSize+1));
	tft.setTextColor(ILI9341_WHITE);

	tft.print(2*select+1);
}

uint8_t bestOfSelect(uint8_t prevSelect) {
	uint8_t select = (prevSelect-1)/2; //select = 1, 2, or 3, for 3, 5, and 7 accordingly

	delay(del);

	while(true) {
		char direction = Serial3.read();
		//char direction = joystick();
		if (direction == 'r' && select<3) {
			redrawBestOf(select);

			select ++;
			drawHighlightBestOf(select);
			delay(del);
		}
		else if (direction == 'l' && select>1) {
			redrawBestOf(select);

			select --;
			drawHighlightBestOf(select);
			delay(del);
		}
		else if (direction =='l' && select==1) {
			redrawBestOf(select);

			select = (prevSelect-1)/2;
			drawHighlightBestOf(select);

			return prevSelect;//therefore use default
		}
		if (direction == 'p') {
			return (2*select+1);
		}
	}
}

void drawHighlightColour(uint8_t pNum, uint16_t colour[], uint8_t select) {
	tft.fillRect(DISPLAY_WIDTH - (DISPLAY_WIDTH-(6*textSize*sizeof("Player 1 Colour: ") + 2*space))*(6-select)/7-1*textSize, //top right x coordingate
								space*(3+2*pNum)+8*(textSize+1)+pNum*8*(textSize)-1*textSize, //top left y coordinate
								8*textSize, 8*textSize, ILI9341_WHITE);
	tft.fillRect(DISPLAY_WIDTH - (DISPLAY_WIDTH-(6*textSize*sizeof("Player 1 Colour: ") + 2*space))*(6-select)/7, //top right x coordingate
								space*(3+2*pNum)+8*(textSize+1)+pNum*8*(textSize), //top left y coordinate
								6*textSize, 6*textSize, colour[select]);
}

void redrawColour(uint8_t pNum, uint16_t colour[], uint8_t select) {
	tft.fillRect(DISPLAY_WIDTH - (DISPLAY_WIDTH-(6*textSize*sizeof("Player 1 Colour: ") + 2*space))*(6-select)/7-1*textSize, //top right x coordingate
								space*(3+2*pNum)+8*(textSize+1)+pNum*8*(textSize)-1*textSize, //top left y coordinate
								8*textSize, 8*textSize, ILI9341_BLACK);
	tft.fillRect(DISPLAY_WIDTH - (DISPLAY_WIDTH-(6*textSize*sizeof("Player 1 Colour: ") + 2*space))*(6-select)/7, //top right x coordingate
								space*(3+2*pNum)+8*(textSize+1)+pNum*8*(textSize), //top left y coordinate
								6*textSize, 6*textSize, colour[select]);
}

uint8_t playerColourSelect(uint8_t pNum, uint16_t colour[], uint8_t prevSelect) {
	uint8_t select = prevSelect;

	delay(del);

	while (true) {
		if (Serial3.available() != 0) {
			char direction = Serial3.read();
			//char direction = joystick();
			if (direction=='r' && select<5) {
				redrawColour(pNum, colour, select);

				select ++;
				drawHighlightColour(pNum, colour, select);

				delay(del);
			}
			else if (direction=='l' && select>0) {
				redrawColour(pNum, colour, select);

				select--;
				drawHighlightColour(pNum, colour, select);

				delay(del);
			}
			else if (direction=='l' && select==0) {
				redrawColour(pNum, colour, select);

				select = prevSelect;
				drawHighlightColour(pNum, colour, select);

				if (pNum == 1) {
					return 4; //player 1 is blue by default
				}
				else if (pNum == 2) {
					return 1; //player 2 is red by default
				}
			}
			if (direction == 'p') {
				return select; //return the character representing the selected colour
			}
		}
	}
}

//draws the Player's new score when they are updated
void drawScores(uint8_t score, uint8_t player) {
	tft.setTextSize(textSize+3);
	if (player == 1) {
		tft.setCursor(DISPLAY_WIDTH/3-6*(textSize+3)/2-space, space*5+8*(textSize+3)*2);
	}
	else if (player == 2) {
		tft.setCursor(DISPLAY_WIDTH*2/3-6*(textSize+3)/2+space, space*5+8*(textSize+3)*2);
	}
	tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
	tft.print(score);
	tft.setTextSize(textSize);
}

//draw the congragulatory sentence when a player wins and display a reset button
void win(uint16_t colour, uint8_t player){
	uint16_t GREY = tft.color565(169,169,169);
	tft.setTextSize(textSize+1);
	tft.setCursor(DISPLAY_WIDTH/2-6*(textSize+1)*(sizeof("Player 1 Won!")+1)/2, DISPLAY_HEIGHT-space*4-8*(textSize+1)*2);

	if (player == 1) {
		tft.setTextColor(colour);
		tft.print(" Player 1 Won!");
	}
	else if (player == 2) {
		tft.setTextColor(colour);
		tft.print(" Player 2 Won!");
	}

	delay(1000);

	tft.setTextSize(textSize+1);
	tft.setTextColor(ILI9341_WHITE);
	tft.fillRect(DISPLAY_WIDTH/2-6*(textSize+1)*sizeof("Reset")/2-space/2, DISPLAY_HEIGHT-space*3-8*(textSize)*2, 6*(textSize+1)*sizeof("Reset"), space*2+7*textSize, GREY);
	tft.drawRect(DISPLAY_WIDTH/2-6*(textSize+1)*sizeof("Reset")/2-space/2, DISPLAY_HEIGHT-space*3-8*(textSize)*2, 6*(textSize+1)*sizeof("Reset"), space*2+7*textSize, ILI9341_WHITE);
	tft.setCursor(DISPLAY_WIDTH/2-6*(textSize+1)*sizeof("Reset")/2+1, DISPLAY_HEIGHT-space*3-5*(textSize)*2);
	tft.print("Reset");

	bool flag = true;
	while (flag) {
		while (Serial3.available() == 0) {}
		if (Serial3.read() == 'p') { //If right joystick button is pressed
			tft.setTextSize(textSize+1);
			tft.setTextColor(GREY);
			tft.fillRect(DISPLAY_WIDTH/2-6*(textSize+1)*sizeof("Reset")/2-space/2, DISPLAY_HEIGHT-space*3-8*(textSize)*2, 6*(textSize+1)*sizeof("Reset"), space*2+7*textSize, ILI9341_WHITE);
			tft.drawRect(DISPLAY_WIDTH/2-6*(textSize+1)*sizeof("Reset")/2-space/2, DISPLAY_HEIGHT-space*3-8*(textSize)*2, 6*(textSize+1)*sizeof("Reset"), space*2+7*textSize, GREY);
			tft.setCursor(DISPLAY_WIDTH/2-6*(textSize+1)*sizeof("Reset")/2+1, DISPLAY_HEIGHT-space*3-5*(textSize)*2);
			tft.print("Reset");

			delay(500);

			tft.fillScreen(ILI9341_BLACK);
			flag = false;
		}
	}
}

//draws the players chosen names and starting scores, as well the scoring mode
void drawScoreScreen(uint16_t p1Colour, uint16_t p2Colour, uint8_t bestOf) {
	tft.fillScreen(ILI9341_BLACK);

	tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
	tft.setTextSize(textSize+1);
	tft.setTextWrap(0);

	//print the scoring mode
	tft.setCursor(DISPLAY_WIDTH/2-6*(textSize+1)*(sizeof("Best of: ")+1)/2, space);
	tft.print("Best of: ");
	tft.print(bestOf);

	//print the chosen name of Player 1 to the left
	tft.setCursor(DISPLAY_WIDTH/3-6*(textSize+1)*sizeof("Player 1")/2-space, space*4 +8*(textSize+1));
	tft.setTextColor(p1Colour);
	tft.print("Player 1");

	//print the chosen name of Player 2 to the right
	tft.setCursor(DISPLAY_WIDTH*2/3-6*(textSize+1)*sizeof("Player 2")/2+space, space*4 +8*(textSize+1));
	tft.setTextColor(p2Colour);
	tft.print("Player 2");

	//draw the starting scores of each player
	drawScores(0, 1);
	drawScores(0, 2);

	while (true) {
		if (joystick() == 'p') {
			win(p1Colour, 1);
		}
	}
}

void drawMenu(){
	tft.fillScreen(ILI9341_BLACK);

	tft.setTextColor(ILI9341_WHITE);
	tft.setTextSize(textSize+1);

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
	tft.fillRect(6*textSize*sizeof("Player 1 Colour: ") + 2*space, 0, BORDER, DISPLAY_HEIGHT, ILI9341_WHITE);
	//Horizontal lines between names
	tft.fillRect(0, 7*(textSize+1) + 2*space, DISPLAY_WIDTH, BORDER, ILI9341_WHITE);
	tft.fillRect(0, 7*(textSize+1) +7*textSize + 4*space, DISPLAY_WIDTH, BORDER, ILI9341_WHITE);
	tft.fillRect(0, 7*(textSize+1) +2*7*textSize + 6*space, DISPLAY_WIDTH, BORDER, ILI9341_WHITE);
	tft.fillRect(0, 7*(textSize+1) +3*7*textSize + 8*space, DISPLAY_WIDTH, BORDER, ILI9341_WHITE);

	//print the "best of" options
	tft.setCursor(DISPLAY_WIDTH - (DISPLAY_WIDTH-(6*textSize*sizeof("Player 1 Colour: ") + 2*space))/4, space*3+7*(textSize+1));
	tft.print("7");
	tft.setCursor(DISPLAY_WIDTH - (DISPLAY_WIDTH-(6*textSize*sizeof("Player 1 Colour: ") + 2*space))*3/4, space*3+7*(textSize+1));
	tft.print("3");
	tft.fillRect(DISPLAY_WIDTH - (DISPLAY_WIDTH-(6*textSize*sizeof("Player 1 Colour: ") + 2*space))/2-1*textSize, space*3+7*(textSize+1)-1*textSize,
							7*textSize, 9*textSize, ILI9341_WHITE);
	tft.setTextColor(ILI9341_BLACK, ILI9341_WHITE);
	tft.setCursor(DISPLAY_WIDTH - (DISPLAY_WIDTH-(6*textSize*sizeof("Player 1 Colour: ") + 2*space))/2, space*3+7*(textSize+1));
	tft.print("5");
	tft.setTextColor(ILI9341_WHITE,ILI9341_BLACK);

	uint16_t colour[] = {ILI9341_MAGENTA, ILI9341_RED, ILI9341_YELLOW, ILI9341_GREEN, ILI9341_BLUE, ILI9341_CYAN};
	//Print the player 1 and Player 2 colour options

	for (int i=1; i<=2; i++) {
		for (int j=0; j<6; j++) {
			if((i==1 && j==4) || (i==2 && j==1)) {
				tft.fillRect(DISPLAY_WIDTH - (DISPLAY_WIDTH-(6*textSize*sizeof("Player 1 Colour: ") + 2*space))*(6-j)/7-1*textSize, //top right x coordingate
											space*(3+2*i)+8*(textSize+1)+i*8*(textSize)-1*textSize, //top left y coordinate
											8*textSize, 8*textSize, ILI9341_WHITE);
			}
			else {
				tft.fillRect(DISPLAY_WIDTH - (DISPLAY_WIDTH-(6*textSize*sizeof("Player 1 Colour: ") + 2*space))*(6-j)/7-1*textSize, //top right x coordingate
											space*(3+2*i)+8*(textSize+1)+i*8*(textSize)-1*textSize, //top left y coordinate
											8*textSize, 8*textSize, ILI9341_BLACK);
			}
			tft.fillRect(DISPLAY_WIDTH - (DISPLAY_WIDTH-(6*textSize*sizeof("Player 1 Colour: ") + 2*space))*(6-j)/7, //top right x coordingate
										space*(3+2*i)+8*(textSize+1)+i*8*(textSize), //top left y coordinate
										6*textSize, 6*textSize, colour[j]);
		}
	}

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

	String setting[] = {"Best Of: ", "Player 1 Colour", "Player 2 Colour", "Play"};

	while(true) {
		//char direction = Serial3.read();
		char direction = joystick();
		if (direction == 'd' && select <3) {
			tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
			tft.setCursor(space, space*(3+2*select)+7*(textSize+1) + select*7*textSize);
			tft.print(setting[select]);

			select++;
			tft.setTextColor(ILI9341_BLACK, ILI9341_WHITE);
			if (select == 3) {
				tft.setCursor(space, DISPLAY_HEIGHT-space-7*textSize);
			}
			else {
				tft.setCursor(space, space*(3+2*select)+7*(textSize+1) + select*7*textSize);
			}
			tft.print(setting[select]);
			delay(del);
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

			select--;
			tft.setTextColor(ILI9341_BLACK, ILI9341_WHITE);
			tft.setCursor(space, space*(3+2*select)+7*(textSize+1) + select*7*textSize);
			tft.print(setting[select]);
			delay(del);
		}
		else if (direction == 'r') {
			if (select == 0) {
				tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
				tft.setCursor(space, space*(3+2*select)+7*(textSize+1) + select*7*textSize);
				tft.print(setting[select]);
				bestOf = bestOfSelect(bestOf);
				tft.setTextColor(ILI9341_BLACK, ILI9341_WHITE);
				tft.setCursor(space, space*(3+2*select)+7*(textSize+1) + select*7*textSize);
				tft.print(setting[select]);
				delay(del);
			}
			else if (select == 1) {
				tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
				tft.setCursor(space, space*(3+2*select)+7*(textSize+1) + select*7*textSize);
				tft.print(setting[select]);
				p1Colour = playerColourSelect(1, colour, p1Colour);
				tft.setTextColor(ILI9341_BLACK, ILI9341_WHITE);
				tft.setCursor(space, space*(3+2*select)+7*(textSize+1) + select*7*textSize);
				tft.print(setting[select]);
				delay(del);
			}
			else if (select == 2) {
				tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
				tft.setCursor(space, space*(3+2*select)+7*(textSize+1) + select*7*textSize);
				tft.print(setting[select]);
				p2Colour = playerColourSelect(2, colour, p2Colour);
				tft.setTextColor(ILI9341_BLACK, ILI9341_WHITE);
				tft.setCursor(space, space*(3+2*select)+7*(textSize+1) + select*7*textSize);
				tft.print(setting[select]);
				delay(del);
			}
		}
		else if (direction == 'p' && select == 3) {
			Serial3.write(bestOf);
			delay(10);
			Serial3.write(colourCharacter[p1Colour]);
			delay(10);
			Serial3.write(colourCharacter[p2Colour]);
			delay(10);
			Serial3.write('s');
			//play the game
			drawScoreScreen(colour[p1Colour], colour[p2Colour], bestOf);
		}
	}
}

/*
int main(){
	setup();

	drawSplashScreen();

	return 0;
}
*/
