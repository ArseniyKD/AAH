#include <Arduino.h>
#include<Adafruit_ILI9341.h>
#include <math.h> //M_PI is pi
//#include "menu.h"

#define TFT_DC 9
#define TFT_CS 10
#define SD_CS 6

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

#define DISPLAY_WIDTH  320
#define DISPLAY_HEIGHT 240

//Left Joystick
#define JOY_VERT_0  A1
#define JOY_HORIZ_0 A0
#define JOY_SEL_0   2

//Right Joystick
#define JOY_VERT_1  A11
#define JOY_HORIZ_1 A10
#define JOY_SEL_1   13


#define JOY_CENTER   512
#define JOY_DEADZONE 32

#define PUCK_RAD 8
#define MALLET_RAD 12
#define COLLISION_DIST 20

#define BORDER 2
#define GOAL 80

int CheckPin = 12;

int P1Score;
int P2Score;

int bestOf;			//game is best of 5 by default
uint8_t p1Colour; //player 1 is blue by default
uint8_t p2Colour; //player 2 is red by default

uint16_t colour[] = {ILI9341_MAGENTA, ILI9341_RED, ILI9341_YELLOW, ILI9341_GREEN, ILI9341_BLUE, ILI9341_CYAN};


uint8_t textSize = 1;
uint8_t space = 10;
int del = 250;

struct Circle{
	float xCoord;
	float yCoord;
	float xVelocity;
	float yVelocity;
	uint16_t Colour;
};

Circle Puck;
Circle MalletP1;
Circle MalletP2;
int speedLimit = 11;
int speedLimitM = 8;
int maxScore;

void drawSplashScreen();
void setup(){
	init();

	Serial.begin(9600);
	tft.begin();
	Serial3.begin(9600);

	tft.setRotation(3);
	pinMode(JOY_SEL_1, INPUT_PULLUP);
	pinMode(CheckPin, INPUT);
	drawSplashScreen();
}

void drawField() {
	tft.fillScreen(ILI9341_BLACK);
	for (int i=0; i<BORDER; i++) {
		tft.drawRect(0+i, 0+i, DISPLAY_WIDTH-2*i, DISPLAY_HEIGHT-2*i, ILI9341_WHITE);
	}
	//draw middle line
	tft.fillRect(DISPLAY_WIDTH/2-BORDER/2, BORDER, BORDER, DISPLAY_HEIGHT-2*BORDER, ILI9341_WHITE);
	//draw left goal
	tft.fillRect(0, DISPLAY_HEIGHT/2 - GOAL/2, BORDER, GOAL, ILI9341_BLACK);
	//draw right goal
	tft.fillRect(DISPLAY_WIDTH-BORDER, DISPLAY_HEIGHT/2 - GOAL/2, BORDER, GOAL, ILI9341_BLACK);
}

int PuckMovement(){

	tft.fillCircle(Puck.xCoord, Puck.yCoord, PUCK_RAD, ILI9341_BLACK);
	//redraw center line if puck passes over it
	if ((Puck.xCoord-PUCK_RAD >= DISPLAY_WIDTH/2-BORDER/2-PUCK_RAD*2) && (Puck.xCoord+PUCK_RAD <= DISPLAY_WIDTH/2+BORDER/2+PUCK_RAD*2)) {
		tft.fillRect(DISPLAY_WIDTH/2-BORDER/2, Puck.yCoord-PUCK_RAD-2, BORDER, PUCK_RAD*2+3, ILI9341_WHITE);
	}

	Puck.xCoord += Puck.xVelocity;
	Puck.yCoord += Puck.yVelocity;

	if(Puck.xCoord - PUCK_RAD + Puck.xVelocity <= 0 || Puck.xCoord + PUCK_RAD + Puck.xVelocity >= DISPLAY_WIDTH){
		Puck.xVelocity *= -0.95;

		if(Puck.xCoord < DISPLAY_WIDTH/2){
			if(Puck.yCoord > DISPLAY_HEIGHT/2 - GOAL/2 && Puck.yCoord < DISPLAY_HEIGHT/2 + GOAL/2){
				return -1;
			}
		}

		if(Puck.xCoord > DISPLAY_WIDTH/2){
			if(Puck.yCoord > DISPLAY_HEIGHT/2 - GOAL/2 && Puck.yCoord < DISPLAY_HEIGHT/2 + GOAL/2){
				return 1;
			}
		}
	}

	if(Puck.yCoord - PUCK_RAD + Puck.yVelocity <= 0 || Puck.yCoord + PUCK_RAD + Puck.yVelocity >= DISPLAY_HEIGHT){
		Puck.yVelocity *= -0.95;
	}

	Puck.xCoord = constrain(Puck.xCoord, BORDER + PUCK_RAD, DISPLAY_WIDTH - BORDER - PUCK_RAD - 1);

	Puck.yCoord = constrain(Puck.yCoord, BORDER + PUCK_RAD, DISPLAY_HEIGHT - BORDER - PUCK_RAD - 1);

	tft.fillCircle(Puck.xCoord,Puck.yCoord, PUCK_RAD, Puck.Colour);
	delay(20);
	return 0;
}

void correctPuckLocation(float distance, Circle *Mallet){
	if(distance < COLLISION_DIST){
		tft.fillCircle(Puck.xCoord, Puck.yCoord, PUCK_RAD, ILI9341_BLACK);

		if ((Puck.xCoord-PUCK_RAD >= DISPLAY_WIDTH/2-BORDER/2-PUCK_RAD*2) && (Puck.xCoord+PUCK_RAD <= DISPLAY_WIDTH/2+BORDER/2+PUCK_RAD*2)) {
			tft.fillRect(DISPLAY_WIDTH/2-BORDER/2, Puck.yCoord-PUCK_RAD-2, BORDER, PUCK_RAD*2+3, ILI9341_WHITE);
		}

		if(Mallet->xCoord <= Puck.xCoord){
			Puck.xCoord += COLLISION_DIST - distance;
		}

		if(Puck.xCoord < Mallet->xCoord){
			Puck.xCoord -= COLLISION_DIST - distance;
		}

		if(Mallet->yCoord < Puck.yCoord){
			Puck.yCoord += COLLISION_DIST - distance;
		}

		if(Puck.yCoord < Mallet->yCoord){
			Puck.yCoord -= COLLISION_DIST - distance;
		}

		Puck.xCoord = constrain(Puck.xCoord, BORDER + PUCK_RAD, DISPLAY_WIDTH - BORDER - PUCK_RAD - 1);
		Puck.yCoord = constrain(Puck.yCoord, BORDER + PUCK_RAD, DISPLAY_HEIGHT - BORDER - PUCK_RAD - 1);
	}
}

float getDist(float X1, float X2, float Y1, float Y2){
	return sqrt(pow(X1 - X2, 2) + pow(Y1 - Y2, 2));
}

void vectorReflection(Circle *Mallet) {
	int puckMass = 0.1;
	int malletMass = 1;
	float restitution = 1;

	float n[2] = {Puck.xCoord - Mallet->xCoord, Puck.yCoord - Mallet->yCoord};

	//find the unit vector for the normal line
  float un[2]	 = {n[0]/sqrtf((n[0]*n[0])+(n[1]*n[1])), n[1]/sqrtf((n[0]*n[0])+(n[1]*n[1]))};

	//no longer need vector n, only un, can dealocate n if necessary

	//find the unit vector for the line tangent to the collision
	float ut[2] = {-un[1], un[0]};

	//compute un dot product velocity to find normal component initially
	float vPuckin = Puck.xVelocity*un[0] + Puck.yVelocity*un[1];

	float vMalletin = Mallet->xVelocity*un[0] + Mallet->yVelocity*un[1];

	//compute ut dot product velocity to find tangential component which is constant before and after collision
	float vPuckt = Puck.xVelocity*ut[0] + Puck.yVelocity*ut[1];

	float vMallett = Mallet->xVelocity*ut[0] + Mallet->yVelocity*ut[1];

	//compute normal component after collision using formula from http://www.vobarian.com/collisions/2dcollisions2.pdf
	float vPuckfn = (vPuckin*(puckMass-malletMass)+2*malletMass*vMalletin)/(puckMass + malletMass);

	float vMalletfn = (vMalletin*(malletMass-puckMass)+2*puckMass*vPuckin)/(puckMass + malletMass);

	Puck.xVelocity = constrain((vPuckfn*un[0] + vPuckt*ut[0])*restitution, -speedLimit, speedLimit);
	Puck.yVelocity = constrain((vPuckfn*un[1] + vPuckt*ut[1])*restitution, -speedLimit, speedLimit);

	//add the x or y portions of the normal and tangential velocities
	Mallet->xVelocity = (vMalletfn*un[0] + vMallett*un[0])*restitution;
	Mallet->yVelocity = (vMalletfn*un[1] + vMallett*un[1])*restitution;

}

void checkForCollision(Circle *Mallet){
	float distance = getDist(Mallet->xCoord, Puck.xCoord, Mallet->yCoord, Puck.yCoord);

	if(distance <= COLLISION_DIST){
		correctPuckLocation(distance, Mallet);
		vectorReflection(Mallet);
	}
}

bool antiFlicker(int changeInX, int changeInY, Circle *Mallet){
	if(changeInX < JOY_CENTER - JOY_DEADZONE || changeInX > JOY_CENTER + JOY_DEADZONE
	|| changeInY > JOY_CENTER + JOY_DEADZONE || changeInY < JOY_CENTER - JOY_DEADZONE){
		return true;
	}
	else{
		Mallet->xVelocity = 0;
		Mallet->yVelocity = 0;
		return false;
	}
}

void MalletMovement(Circle *Mallet, int n){
	int xVal, yVal;
	if(n == 0){
		xVal = analogRead(JOY_HORIZ_0);
		yVal = analogRead(JOY_VERT_0);
	}
	else if(n==1){
		xVal = analogRead(JOY_HORIZ_1);
		yVal = analogRead(JOY_VERT_1);

	}

	if(antiFlicker(xVal, yVal, Mallet)){

		tft.fillCircle(Mallet->xCoord, Mallet->yCoord, MALLET_RAD, ILI9341_BLACK);

		if(yVal < JOY_CENTER - JOY_DEADZONE){
	    //move the cursor up with a speed proportional to yVal
	    Mallet->yCoord -= constrain((JOY_CENTER - yVal) / 40, -speedLimitM, speedLimitM);
		  Mallet->yVelocity = constrain((yVal - JOY_CENTER) / 40, -speedLimitM, speedLimitM);
	  }

    //if the joystick has been pushed down
    else if(yVal > JOY_CENTER + JOY_DEADZONE){
	    //move the cursor down with a speed proportional to yVal
	    Mallet->yCoord += constrain((yVal - JOY_CENTER) / 40, -speedLimitM, speedLimitM);
		  Mallet->yVelocity = constrain((yVal - JOY_CENTER) / 40, -speedLimitM, speedLimitM);
    }

    //if the joystick has been moved left
    if(xVal > JOY_CENTER + JOY_DEADZONE){
	    //move the cursor left with a speed proportional to xVal
	    Mallet->xCoord -= constrain((xVal - JOY_CENTER)/40, -speedLimitM, speedLimitM);
		  Mallet->xVelocity = constrain((JOY_CENTER - xVal)/40, -speedLimitM, speedLimitM);
    }

    //if the joystick has been moved right
    else if( xVal < JOY_CENTER - JOY_DEADZONE){
      //move the cursor right with a speed proportional to xVal
      Mallet->xCoord += constrain((JOY_CENTER - xVal)/40, -speedLimitM, speedLimitM);
	  	Mallet->xVelocity = constrain((JOY_CENTER - xVal)/40, -speedLimitM, speedLimitM);
    }

		// if (num == 0) {
		// 	//constrain the location of the cursor to be within the left half of the feild
		if(Mallet->xCoord < DISPLAY_WIDTH / 2){
	    	Mallet->xCoord = constrain(Mallet->xCoord, BORDER + MALLET_RAD, DISPLAY_WIDTH/2 - BORDER/2 - MALLET_RAD - 1);
		}
		else{

		// else if (num == 1) {
		// 	//constrain the location of the cursor to be within the right half of the feild
			Mallet->xCoord = constrain(Mallet->xCoord, DISPLAY_WIDTH/2 + BORDER/2 + MALLET_RAD + 1, DISPLAY_WIDTH - BORDER - MALLET_RAD - 1);
		}
		//constrain the location of the cursor to be within the screen dimensions
		Mallet->yCoord = constrain(Mallet->yCoord, BORDER + MALLET_RAD, DISPLAY_HEIGHT - BORDER - MALLET_RAD - 1);

		//tft.fillCircle(Mallet[num].xCoord, Mallet[num].yCoord, MALLET_RAD, ILI9341_BLUE);
	}
	tft.fillCircle(Mallet->xCoord, Mallet->yCoord, MALLET_RAD, Mallet->Colour);
}


void pregame(){
	//one joystick sends constantly LRUDP values. if S is received, end function
	bool flag = true;
	int xVal, yVal;

	while(flag){
		xVal = analogRead(JOY_HORIZ_1);
		yVal = analogRead(JOY_VERT_1);

		if(yVal < JOY_CENTER - JOY_DEADZONE){
			Serial3.write('u');
			delay(500);
		}

		else if (yVal > JOY_CENTER + JOY_DEADZONE){
			Serial3.write('d');
			delay(500);
		}

		if(xVal < JOY_CENTER - JOY_DEADZONE){
			Serial3.write('r');
			delay(500);
		}

		else if(xVal > JOY_CENTER + JOY_DEADZONE){
			Serial3.write('l');
			delay(500);
		}

		if(digitalRead(JOY_SEL_1) == LOW){
			Serial3.write('p');
			delay(500);
		}

		if(Serial3.available() != 0){
			char byteRead = Serial3.read();
			delay(100);

			if(byteRead == 'c'){
				char colourRead = Serial3.read();
				if(colourRead == 'm'){MalletP1.Colour = ILI9341_MAGENTA;}
				else if (colourRead == 'r'){MalletP1.Colour = ILI9341_RED;}
				else if (colourRead == 'y'){MalletP1.Colour = ILI9341_YELLOW;}
				else if (colourRead == 'g'){MalletP1.Colour = ILI9341_GREEN;}
				else if (colourRead == 'b'){MalletP1.Colour = ILI9341_BLUE;}
				else if (colourRead == 'c'){MalletP1.Colour = ILI9341_CYAN;}
			}

			if(byteRead == 'C'){
				char colourRead = Serial3.read();
				if(colourRead == 'm'){MalletP2.Colour = ILI9341_MAGENTA;}
				else if (colourRead == 'r'){MalletP2.Colour = ILI9341_RED;}
				else if (colourRead == 'y'){MalletP2.Colour = ILI9341_YELLOW;}
				else if (colourRead == 'g'){MalletP2.Colour = ILI9341_GREEN;}
				else if (colourRead == 'b'){MalletP2.Colour = ILI9341_BLUE;}
				else if (colourRead == 'c'){MalletP2.Colour = ILI9341_CYAN;}
			}

			if(byteRead == 'b'){
				int gameNumRead = Serial3.read();
				if(gameNumRead == 3){maxScore = 2;}
				else if (gameNumRead == 5){maxScore = 3;}
				else if (gameNumRead == 7){maxScore = 4;}
			}

			if(byteRead == 's'){
				flag = false;
			}
		}
	}
}

void setupGame(){
	Puck.Colour = ILI9341_WHITE;

	drawField();

	Puck.xCoord = DISPLAY_WIDTH / 2;
	Puck.yCoord = DISPLAY_HEIGHT / 2;
	Puck.xVelocity = Puck.yVelocity = 0;

	tft.fillCircle(Puck.xCoord, Puck.yCoord, PUCK_RAD, Puck.Colour);

	MalletP1.xCoord = 25;
	MalletP1.yCoord = DISPLAY_HEIGHT / 2;
	MalletP2.xCoord = DISPLAY_WIDTH - 25;
	MalletP2.yCoord = DISPLAY_HEIGHT / 2;

	tft.fillCircle(MalletP1.xCoord, MalletP1.yCoord, MALLET_RAD, MalletP1.Colour);
	tft.fillCircle(MalletP2.xCoord, MalletP2.yCoord, MALLET_RAD, MalletP2.Colour);
}

void game(){
	P1Score = 0;
	P2Score = 0;
	Serial.println(P1Score);
	Serial.println(maxScore);
	while(P1Score < maxScore && P2Score < maxScore){
		setupGame();
		bool flag = true;

		while(flag){
			int scoreUpdate = PuckMovement();

			if(scoreUpdate == -1){
				P1Score++;
				Serial3.write('P');
				flag = false;
			}

			else if(scoreUpdate == 1){
				P2Score++;
				Serial3.write('p');
				flag = false;
			}

			else{
				MalletMovement(&MalletP1, 0);
				MalletMovement(&MalletP2, 1);
				checkForCollision(&MalletP1);
				checkForCollision(&MalletP2);
			}
		}
	}

	drawSplashScreen();
	delay(250);
}

void endScr(){
	while(digitalRead(JOY_SEL_1) == HIGH){}
	Serial3.write('p');
}

void GameMachine(){
	enum State {INIT, ACK, PREGAME, GAME, ENDSCREEN};
	State currentState = INIT;

	while(true){
		if(currentState == INIT){
			if(Serial3.available() != 0){
				int byteRead = Serial3.read();
				if(byteRead == 65){
					currentState = ACK;
				}
			}

		}

		if(currentState == ACK){
			Serial3.write((char)65);
			currentState = PREGAME;
		}

		if(currentState == PREGAME){
			pregame();
			currentState = GAME;
		}

		if(currentState == GAME){
			game();
			currentState = ENDSCREEN;
		}

		if(currentState == ENDSCREEN){
			endScr();
			currentState = INIT;
		}

	}
}

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

}

void menuSelect();

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

void drawHighlightColour(uint8_t pNum, uint8_t select) {
	tft.fillRect(DISPLAY_WIDTH - (DISPLAY_WIDTH-(6*textSize*sizeof("Player 1 Colour: ") + 2*space))*(6-select)/7-1*textSize, //top right x coordingate
								space*(3+2*pNum)+8*(textSize+1)+pNum*8*(textSize)-1*textSize, //top left y coordinate
								8*textSize, 8*textSize, ILI9341_WHITE);
	tft.fillRect(DISPLAY_WIDTH - (DISPLAY_WIDTH-(6*textSize*sizeof("Player 1 Colour: ") + 2*space))*(6-select)/7, //top right x coordingate
								space*(3+2*pNum)+8*(textSize+1)+pNum*8*(textSize), //top left y coordinate
								6*textSize, 6*textSize, colour[select]);
}

void redrawColour(uint8_t pNum, uint8_t select) {
	tft.fillRect(DISPLAY_WIDTH - (DISPLAY_WIDTH-(6*textSize*sizeof("Player 1 Colour: ") + 2*space))*(6-select)/7-1*textSize, //top right x coordingate
								space*(3+2*pNum)+8*(textSize+1)+pNum*8*(textSize)-1*textSize, //top left y coordinate
								8*textSize, 8*textSize, ILI9341_BLACK);
	tft.fillRect(DISPLAY_WIDTH - (DISPLAY_WIDTH-(6*textSize*sizeof("Player 1 Colour: ") + 2*space))*(6-select)/7, //top right x coordingate
								space*(3+2*pNum)+8*(textSize+1)+pNum*8*(textSize), //top left y coordinate
								6*textSize, 6*textSize, colour[select]);
}

uint8_t playerColourSelect(uint8_t pNum, uint8_t prevSelect) {
	uint8_t select = prevSelect;

	delay(del);

	while (true) {
		if (Serial3.available() != 0) {
			char direction = Serial3.read();
			//char direction = joystick();
			if (direction=='r' && select<5) {
				redrawColour(pNum, select);

				select ++;
				drawHighlightColour(pNum, select);

				delay(del);
			}
			else if (direction=='l' && select>0) {
				redrawColour(pNum, select);

				select--;
				drawHighlightColour(pNum, select);

				delay(del);
			}
			else if (direction=='l' && select==0) {
				redrawColour(pNum, select);

				select = prevSelect;
				drawHighlightColour(pNum, select);

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
void win(uint8_t player){
	uint16_t GREY = tft.color565(169,169,169);
	tft.setTextSize(textSize+1);
	tft.setCursor(DISPLAY_WIDTH/2-6*(textSize+1)*(sizeof("Player 1 Won!")+1)/2, DISPLAY_HEIGHT-space*4-8*(textSize+1)*2);

	if (player == 1) {
		tft.setTextColor(colour[p1Colour]);
		tft.print(" Player 1 Won!");
	}
	else if (player == 2) {
		tft.setTextColor(colour[p2Colour]);
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
void drawScoreScreen() {
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
	tft.setTextColor(colour[p1Colour]);
	tft.print("Player 1");

	//print the chosen name of Player 2 to the right
	tft.setCursor(DISPLAY_WIDTH*2/3-6*(textSize+1)*sizeof("Player 2")/2+space, space*4 +8*(textSize+1));
	tft.setTextColor(colour[p2Colour]);
	tft.print("Player 2");

	P1Score = P2Score = 0;
	int maxScore = (bestOf +1)/2;

	//draw the starting scores of each player
	drawScores(0, 1);
	drawScores(0, 2);
	while(P1Score < maxScore && P2Score < maxScore){
		while(Serial3.available() == 0){}
		char byteRead = Serial3.read();
		if(byteRead == 'p'){
			P1Score++;
			drawScores(P1Score, 1);
		}
		else if(byteRead == 'P'){
			P2Score++;
			drawScores(P2Score, 2);
		}
	}
	if(P1Score > P2Score){
		win(1);
	}
	else{
		win(2);
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

	menuSelect();
}


void menuSelect() {
	uint8_t select = 3; //select the play button by default
	bestOf = 5;			//game is best of 5 by default
	p1Colour = 4; //player 1 is blue by default
	p2Colour = 1;	//player 2 is red by default
	char colourCharacter[] = {'m', 'r', 'y', 'g', 'b', 'c'};

	String setting[] = {"Best Of: ", "Player 1 Colour", "Player 2 Colour", "Play"};

	while(true) {
		while(Serial3.available() == 0){}
		char direction = Serial3.read();
		//char direction = joystick();
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
				p1Colour = playerColourSelect(1, p1Colour);
				tft.setTextColor(ILI9341_BLACK, ILI9341_WHITE);
				tft.setCursor(space, space*(3+2*select)+7*(textSize+1) + select*7*textSize);
				tft.print(setting[select]);
				delay(del);
			}
			else if (select == 2) {
				tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
				tft.setCursor(space, space*(3+2*select)+7*(textSize+1) + select*7*textSize);
				tft.print(setting[select]);
				p2Colour = playerColourSelect(2, p2Colour);
				tft.setTextColor(ILI9341_BLACK, ILI9341_WHITE);
				tft.setCursor(space, space*(3+2*select)+7*(textSize+1) + select*7*textSize);
				tft.print(setting[select]);
				delay(del);
			}
		}
		else if (direction == 'p' && select == 3) {
			delay(1000);
			Serial3.write('b');
			Serial3.write((char)bestOf);
			delay(10);
			Serial3.write('c');
			Serial3.write(colourCharacter[p1Colour]);
			delay(10);
			Serial3.write('C');
			Serial3.write(colourCharacter[p2Colour]);
			delay(10);
			Serial3.write('s');
			//play the game
			break;
			//drawScoreScreen(colour[p1Colour], colour[p2Colour], bestOf);
		}
	}
}



void ScoreMachine(){
	enum State {INIT, MENU, SCOREBOARD, ENDSCREEN};
	State currentState = INIT;

	drawSplashScreen();

	while(true){
		if(currentState == INIT){
			Serial3.write((char)65);
			if(Serial3.available() != 0){
				int byteRead = Serial3.read();
				if(byteRead == 65){
					currentState = MENU;
				}
			}
		}
		if(currentState == MENU){
			while(Serial3.available() == 0){}
			char byteRead = Serial3.read();
			if(byteRead = 'p'){
				drawMenu();
				currentState = SCOREBOARD;
			}
		}
		if(currentState == SCOREBOARD){
			drawScoreScreen();
			currentState = ENDSCREEN;
		}
		if(currentState == ENDSCREEN){
			drawSplashScreen();
			currentState = INIT;
		}
	}
}


int main(){
	setup();

	if(digitalRead(CheckPin) == HIGH){
		GameMachine();
	}
	else if(digitalRead(CheckPin) == LOW){
		ScoreMachine();
	}

	return 0;
}
