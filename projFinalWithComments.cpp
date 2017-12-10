#include <Arduino.h>
#include<Adafruit_ILI9341.h>
#include <math.h> //M_PI is pi

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

//define the correct radii for the puck and mallets. COLLISION_DIST is the maximum
//distance between the center of the puck and the center of a mallet before collision
//is detected.
#define PUCK_RAD 8
#define MALLET_RAD 12
#define COLLISION_DIST 20

#define BORDER 2 //width of the border in the background
#define GOAL 80 //width of the goal

//this pin is used to figure out which arduino is the game arduino and which one
//is the score keeping one.
int CheckPin = 12;

//this keeps track of the scores for each player
int P1Score;
int P2Score;

int bestOf;			//game is best of 5 by default
uint8_t p1Colour; //player 1 is blue by default
uint8_t p2Colour; //player 2 is red by default

//an array of all the colours available to the players.
uint16_t colour[] = {ILI9341_MAGENTA, ILI9341_RED, ILI9341_YELLOW, ILI9341_GREEN, ILI9341_BLUE, ILI9341_CYAN};

//the settings for the menu screen
uint8_t textSize = 1;
uint8_t space = 10;
int del = 250;

//this struct keeps all the necessary information for the processing of the Puck
//and mallets. It contains the X and Y coordinates of the object, as well as its
//X and Y velocities. The colour of each object also gets stored here.
struct Circle{
	float xCoord;
	float yCoord;
	float xVelocity;
	float yVelocity;
	uint16_t Colour;
};

//the three Circle objects are for the Puck, first player's mallet and second
//player's mallet.
Circle Puck;
Circle MalletP1;
Circle MalletP2;

//the speed limits for both the puck (named speedLimit) and the mallet (named speedLimitM)
//the puck is allowed to be quicker than the mallet.
int speedLimit = 11;
int speedLimitM = 8;

//the score at which a win is declared. The possible values are 2/3/4
int maxScore;

//forward declaration of the drawSplashScreen() method.
void drawSplashScreen();

//this is the setup method which initialises the arduino and all relevant components
void setup(){
	init();

	//initialisation of the Serial communication between the arduinos. also initialises
	//the screen
	Serial.begin(9600);
	tft.begin();
	Serial3.begin(9600);

	//set the screen's orientation and initialises the correct pins for their inputs.
	tft.setRotation(3);
	pinMode(JOY_SEL_1, INPUT_PULLUP);
	pinMode(CheckPin, INPUT);
}

//draws the field on which the game is played.
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

//this is the method that controls the movement of the puck within the field. It will
//only do the reflections from the walls. It is of type int so that it is easier
//to keep track of goals. If 0 is returned, then there was no goal. If either -1
//or 1 is returned, then there was a goal and the score should be updated.
int PuckMovement(){

	//gets rid of the current circle.
	tft.fillCircle(Puck.xCoord, Puck.yCoord, PUCK_RAD, ILI9341_BLACK);
	//redraw center line if puck passes over it
	if ((Puck.xCoord-PUCK_RAD >= DISPLAY_WIDTH/2-BORDER/2-PUCK_RAD*2) && (Puck.xCoord+PUCK_RAD <= DISPLAY_WIDTH/2+BORDER/2+PUCK_RAD*2)) {
		tft.fillRect(DISPLAY_WIDTH/2-BORDER/2, Puck.yCoord-PUCK_RAD-2, BORDER, PUCK_RAD*2+3, ILI9341_WHITE);
	}

	//update the location of the puck by the velocity values.
	Puck.xCoord += Puck.xVelocity;
	Puck.yCoord += Puck.yVelocity;

	//a check if either the right edge or the left edge has been hit. if it was,
	//then reverse the xVelocity and decrease it by 5%. This decrease is what
	//gives the puck a sense of friction.
	if(Puck.xCoord - PUCK_RAD + Puck.xVelocity <= 0 || Puck.xCoord + PUCK_RAD + Puck.xVelocity >= DISPLAY_WIDTH){
		Puck.xVelocity *= -0.95;

		//checks if the puck hit the goal on the left side of the screen. if it has,
		//then returns -1 for the score to update.
		if(Puck.xCoord < DISPLAY_WIDTH/2){
			if(Puck.yCoord > DISPLAY_HEIGHT/2 - GOAL/2 && Puck.yCoord < DISPLAY_HEIGHT/2 + GOAL/2){
				return -1;
			}
		}

		//checks if the puck hit the goal on the right side of the screen, if it has,
		//then returns 1 for the score to update.
		if(Puck.xCoord > DISPLAY_WIDTH/2){
			if(Puck.yCoord > DISPLAY_HEIGHT/2 - GOAL/2 && Puck.yCoord < DISPLAY_HEIGHT/2 + GOAL/2){
				return 1;
			}
		}
	}

	//checks if either the top edge or bottom edge has been hit, it it was, then
	//reverse the yVelocity and decrease it by 5%. This decrease is what gives the
	//puck a sense of friction.
	if(Puck.yCoord - PUCK_RAD + Puck.yVelocity <= 0 || Puck.yCoord + PUCK_RAD + Puck.yVelocity >= DISPLAY_HEIGHT){
		Puck.yVelocity *= -0.95;
	}

	//constrain the coordinates so that the puck does not leave the screen.
	Puck.xCoord = constrain(Puck.xCoord, BORDER + PUCK_RAD, DISPLAY_WIDTH - BORDER - PUCK_RAD - 1);
	Puck.yCoord = constrain(Puck.yCoord, BORDER + PUCK_RAD, DISPLAY_HEIGHT - BORDER - PUCK_RAD - 1);

	//redraw the puck in the right place.
	tft.fillCircle(Puck.xCoord,Puck.yCoord, PUCK_RAD, Puck.Colour);

	//this delay is vital to the flow of the game. if this delay is not there, then
	//there is too much flickering and the provided Adafruit starts to create random
	//fragmentation of images. if the delay is any higher, then the game seems to
	//slow down by a noticeable amount. If you go even higher, the game starts to
	//noticeably "lag", creating a terrible, jittery look to the game.
	delay(20);
	return 0;
}

//this method puts the puck outside of the mallet. with our collision detection
//the puck constantly ends up inside of the mallet. Thus, this method puts the
//puck at the correct edge of the correct mallet. This is done by adding or
//subtracting the difference between the correct  collision distance and the
//current distance between the centers of the two Circles.
void correctPuckLocation(float distance, Circle *Mallet){
	if(distance < COLLISION_DIST){
		tft.fillCircle(Puck.xCoord, Puck.yCoord, PUCK_RAD, ILI9341_BLACK);

		//redraw the line in the middle.
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

		//keeps the puck within the screen. This way, the mallet cannot force the puck
		//outside the screen.
		Puck.xCoord = constrain(Puck.xCoord, BORDER + PUCK_RAD, DISPLAY_WIDTH - BORDER - PUCK_RAD - 1);
		Puck.yCoord = constrain(Puck.yCoord, BORDER + PUCK_RAD, DISPLAY_HEIGHT - BORDER - PUCK_RAD - 1);
	}
}

//this method returns the distance between two points. uses the basic distance formula
float getDist(float X1, float X2, float Y1, float Y2){
	return sqrt(pow(X1 - X2, 2) + pow(Y1 - Y2, 2));
}

//this method correctly reflects the puck from the mallet, keeping momentum in
//mind. The method finds the components of the puck and mallet velocities normal
//to the collision plane, and tangential to it. The tangential components are
//conserved, and the normal components are calculated based on conservation of
//momentum
then reflects the puck from that tangent.
void vectorReflection(Circle *Mallet) {
	int puckMass = 0.1;
	int malletMass = 1;
	float restitution = 1;

	//find the line normal to the collision plane. ie. the line from the center
	//of the mallet to the center of the puck
	float n[2] = {Puck.xCoord - Mallet->xCoord, Puck.yCoord - Mallet->yCoord};

	//find the unit vector for the normal line
  float un[2]	 = {n[0]/sqrtf((n[0]*n[0])+(n[1]*n[1])), n[1]/sqrtf((n[0]*n[0])+(n[1]*n[1]))};

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

	//constrains both the x and y velocities of the puck to the puck's speed limit.
	Puck.xVelocity = constrain((vPuckfn*un[0] + vPuckt*ut[0])*restitution, -speedLimit, speedLimit);
	Puck.yVelocity = constrain((vPuckfn*un[1] + vPuckt*ut[1])*restitution, -speedLimit, speedLimit);

	//add the x or y portions of the normal and tangential velocities
	Mallet->xVelocity = (vMalletfn*un[0] + vMallett*un[0])*restitution;
	Mallet->yVelocity = (vMalletfn*un[1] + vMallett*un[1])*restitution;

}

//this method checks whether or not a mallet collided with a puck. if they did, then
//it reflects the puck correctly.
void checkForCollision(Circle *Mallet){
	float distance = getDist(Mallet->xCoord, Puck.xCoord, Mallet->yCoord, Puck.yCoord);

	if(distance <= COLLISION_DIST){
		correctPuckLocation(distance, Mallet);
		vectorReflection(Mallet);
	}
}

//this method gets rid of the flickering in a stationary mallet. If a mallet is not
//moving, then it's velocities are set to 0.
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

//this method gets the input from the specified joystick and moves the mallets accordingly
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

	//if the joystick is moved, then the mallet gets moved.
	if(antiFlicker(xVal, yVal, Mallet)){

		tft.fillCircle(Mallet->xCoord, Mallet->yCoord, MALLET_RAD, ILI9341_BLACK);

		//if the joystick has been pushed up
		if(yVal < JOY_CENTER - JOY_DEADZONE){
	    //move the mallet up with a speed proportional to yVal
	    Mallet->yCoord -= constrain((JOY_CENTER - yVal) / 40, -speedLimitM, speedLimitM);
		  Mallet->yVelocity = constrain((yVal - JOY_CENTER) / 40, -speedLimitM, speedLimitM);
	  }

    //if the joystick has been pushed down
    else if(yVal > JOY_CENTER + JOY_DEADZONE){
	    //move the mallet down with a speed proportional to yVal
	    Mallet->yCoord += constrain((yVal - JOY_CENTER) / 40, -speedLimitM, speedLimitM);
		  Mallet->yVelocity = constrain((yVal - JOY_CENTER) / 40, -speedLimitM, speedLimitM);
    }

    //if the joystick has been moved left
    if(xVal > JOY_CENTER + JOY_DEADZONE){
	    //move the mallet left with a speed proportional to xVal
	    Mallet->xCoord -= constrain((xVal - JOY_CENTER)/40, -speedLimitM, speedLimitM);
		  Mallet->xVelocity = constrain((JOY_CENTER - xVal)/40, -speedLimitM, speedLimitM);
    }

    //if the joystick has been moved right
    else if( xVal < JOY_CENTER - JOY_DEADZONE){
      //move the mallet right with a speed proportional to xVal
      Mallet->xCoord += constrain((JOY_CENTER - xVal)/40, -speedLimitM, speedLimitM);
	  	Mallet->xVelocity = constrain((JOY_CENTER - xVal)/40, -speedLimitM, speedLimitM);
    }


		//constrain the location of the mallet to be within the left half of the feild
		if(Mallet->xCoord < DISPLAY_WIDTH / 2){
	  	Mallet->xCoord = constrain(Mallet->xCoord, BORDER + MALLET_RAD, DISPLAY_WIDTH/2 - BORDER/2 - MALLET_RAD - 1);
		}
		else{
			//constrain the location of the mallet to be within the right half of the feild
			Mallet->xCoord = constrain(Mallet->xCoord, DISPLAY_WIDTH/2 + BORDER/2 + MALLET_RAD + 1, DISPLAY_WIDTH - BORDER - MALLET_RAD - 1);
		}

		//constrain the location of the mallet to be within the screen dimensions
		Mallet->yCoord = constrain(Mallet->yCoord, BORDER + MALLET_RAD, DISPLAY_HEIGHT - BORDER - MALLET_RAD - 1);

	}

	tft.fillCircle(Mallet->xCoord, Mallet->yCoord, MALLET_RAD, Mallet->Colour);
}

//the pregame part of the game FSM.
void pregame(){
	//one joystick sends constantly LRUDP values. if S is received, end function
	bool flag = true;
	int xVal, yVal;

	//this loop runs until an s is received from the other arduino.
	while(flag){
		//this reads in the input from the right joystick to control the menu on the other
		//screen.
		xVal = analogRead(JOY_HORIZ_1);
		yVal = analogRead(JOY_VERT_1);

		//these next four if statements check for which direction the joystick is pressed
		//and sends the correct character to the other arduino. the delay ensures that
		//the serial buffer does not get filled with too many characters.
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

		//checks if the joystick has been pressed in.
		if(digitalRead(JOY_SEL_1) == LOW){
			Serial3.write('p');
			delay(500);
		}

		//this part receives all the game settings.
		if(Serial3.available() != 0){
			char byteRead = Serial3.read();
			//this delay lets the other arduino to send over all the settings before it
			//reads the inputs.
			delay(100);

			//if "c" is read, then the next character is guaranteed to be a character that
			//represents the colour of the first mallet.
			if(byteRead == 'c'){
				char colourRead = Serial3.read();
				if(colourRead == 'm'){MalletP1.Colour = ILI9341_MAGENTA;}
				else if (colourRead == 'r'){MalletP1.Colour = ILI9341_RED;}
				else if (colourRead == 'y'){MalletP1.Colour = ILI9341_YELLOW;}
				else if (colourRead == 'g'){MalletP1.Colour = ILI9341_GREEN;}
				else if (colourRead == 'b'){MalletP1.Colour = ILI9341_BLUE;}
				else if (colourRead == 'c'){MalletP1.Colour = ILI9341_CYAN;}
			}
			//if "C" is read, then the next character is guaranteed to be a character that
			//represents the colour of the second mallet.
			if(byteRead == 'C'){
				char colourRead = Serial3.read();
				if(colourRead == 'm'){MalletP2.Colour = ILI9341_MAGENTA;}
				else if (colourRead == 'r'){MalletP2.Colour = ILI9341_RED;}
				else if (colourRead == 'y'){MalletP2.Colour = ILI9341_YELLOW;}
				else if (colourRead == 'g'){MalletP2.Colour = ILI9341_GREEN;}
				else if (colourRead == 'b'){MalletP2.Colour = ILI9341_BLUE;}
				else if (colourRead == 'c'){MalletP2.Colour = ILI9341_CYAN;}
			}

			//if "b" is read, then the next number is guaranteed to be the number of max games.
			if(byteRead == 'b'){
				int gameNumRead = Serial3.read();
				if(gameNumRead == 3){maxScore = 2;}
				else if (gameNumRead == 5){maxScore = 3;}
				else if (gameNumRead == 7){maxScore = 4;}
			}

			//if "s" is read, then the game should start.
			if(byteRead == 's'){
				flag = false;
			}
		}
	}
}

//this method sets up the game at the very start and between each round.
void setupGame(){
	Puck.Colour = ILI9341_WHITE;

	//redraws the entire field.
	drawField();

	//sets the puck to the center of the screen and resets its velocity to 0
	Puck.xCoord = DISPLAY_WIDTH / 2;
	Puck.yCoord = DISPLAY_HEIGHT / 2;
	Puck.xVelocity = Puck.yVelocity = 0;

	tft.fillCircle(Puck.xCoord, Puck.yCoord, PUCK_RAD, Puck.Colour);

	//sets the two mallets to the correct parts of the field.
	MalletP1.xCoord = 25;
	MalletP1.yCoord = DISPLAY_HEIGHT / 2;
	MalletP2.xCoord = DISPLAY_WIDTH - 25;
	MalletP2.yCoord = DISPLAY_HEIGHT / 2;

	tft.fillCircle(MalletP1.xCoord, MalletP1.yCoord, MALLET_RAD, MalletP1.Colour);
	tft.fillCircle(MalletP2.xCoord, MalletP2.yCoord, MALLET_RAD, MalletP2.Colour);
}

//this is the game state it runs until either score reaches the max score value
void game(){
	//set the scores initially to 0.
	P1Score = 0;
	P2Score = 0;

	//this is the loop that keeps track of each round.
	while(P1Score < maxScore && P2Score < maxScore){
		//setup the round
		setupGame();
		bool flag = true;

		//this is the loop that controls the entire game. it ends when either player scores a goal.
		while(flag){
			//this keeps track of the scores.
			int scoreUpdate = PuckMovement();

			//if -1 is returned by PuckMovement(), then update the first player's score,
			//send "P" to the other arduino and start the next round.
			if(scoreUpdate == -1){
				P1Score++;
				Serial3.write('P');
				flag = false;
			}

			//if 1 is returned by PuckMovement(), then update the second player's score,
			//send "p" to the other arduino and start the next round.
			else if(scoreUpdate == 1){
				P2Score++;
				Serial3.write('p');
				flag = false;
			}

			//if 0 is returned, update the mallet positions and check for collision.
			else{
				MalletMovement(&MalletP1, 0);
				MalletMovement(&MalletP2, 1);
				checkForCollision(&MalletP1);
				checkForCollision(&MalletP2);
			}
		}
	}

	//once the game is over, redraw the splash screen.
	drawSplashScreen();
	delay(250);
}

//this is the end screen sequence. unitl the second joystick is pressed in, the game
//will stay in this state.
void endScr(){
	while(digitalRead(JOY_SEL_1) == HIGH){}
	Serial3.write('p');
}

//this is the GameMachine FSM. It keeps track of the current state of the game and
//ensures that the two arduinos are synced up.
void GameMachine(){
	enum State {INIT, ACK, PREGAME, GAME, ENDSCREEN};
	State currentState = INIT;

	//draw the splash screen for the first time when the arduino is swtiched on.
	drawSplashScreen();

	while(true){

		//the initialisation state syncs up the two arduinos.
		if(currentState == INIT){
			//the state does not progress until an "A" is received.
			if(Serial3.available() != 0){
				int byteRead = Serial3.read();
				if(byteRead == 65){
					currentState = ACK;
				}
			}
		}

		//this arduino sends over the acknowledgement of connection.
		if(currentState == ACK){
			Serial3.write((char)65);
			currentState = PREGAME;
		}

		//the pregame state. it progresses to the game state once the pregame() method finishes running.
		if(currentState == PREGAME){
			pregame();
			currentState = GAME;
		}

		//the game state. it progresses to the end screen state once game() method finishes running.
		if(currentState == GAME){
			game();
			currentState = ENDSCREEN;
		}

		//the end screen state. it resets the entire FSM once the endScr() method finishes running.
		if(currentState == ENDSCREEN){
			endScr();
			currentState = INIT;
		}
	}
}

void drawMenu();

//This method draws the splash screen with the name of the game and instructions
//for how to start
void drawSplashScreen(){
	tft.fillScreen(ILI9341_BLACK);
	tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
	tft.setTextSize(textSize+2);
	tft.setTextWrap(false);
	//write the name of the game
	tft.setCursor((DISPLAY_WIDTH-6*(textSize+2)*sizeof("Arduino"))/2, DISPLAY_HEIGHT/2 - space - 2*8*(textSize+2));
	tft.print("Arduino");
	tft.setCursor((DISPLAY_WIDTH-6*(textSize+2)*sizeof("Air Hockey"))/2, DISPLAY_HEIGHT/2 - space - 8*(textSize+2));
	tft.print("Air Hockey");

	//write the starting instructions
	tft.setCursor(DISPLAY_WIDTH/2-(6*(textSize)*sizeof("-- Press Right Joystick Buttonn to Start --"))/2, DISPLAY_HEIGHT/2 + space);
	tft.setTextSize(textSize);
	tft.print("-- Press Right Joystick Button to Start --");

}

void menuSelect();

//highlights a "Best of" option to show it is selected
void drawHighlightBestOf(uint8_t select) {
	tft.fillRect(DISPLAY_WIDTH - (DISPLAY_WIDTH-(6*textSize*sizeof("Player 1 Colour: ") + 2*space))*(4-select)/4-2*textSize, space*3+7*(textSize+1)-2*textSize,
							9*textSize, 11*textSize, ILI9341_WHITE);
	tft.setCursor(DISPLAY_WIDTH - (DISPLAY_WIDTH-(6*textSize*sizeof("Player 1 Colour: ") + 2*space))*(4-select)/4, space*3+7*(textSize+1));
	tft.setTextColor(ILI9341_BLACK);

	tft.print(2*select+1); //print the selected option
}

//redraws a "Best of" option without highlight to show it is not selected
void redrawBestOf(uint8_t select) {
	tft.fillRect(DISPLAY_WIDTH - (DISPLAY_WIDTH-(6*textSize*sizeof("Player 1 Colour: ") + 2*space))*(4-select)/4-2*textSize, space*3+7*(textSize+1)-2*textSize,
							9*textSize, 11*textSize, ILI9341_BLACK);
	tft.setCursor(DISPLAY_WIDTH - (DISPLAY_WIDTH-(6*textSize*sizeof("Player 1 Colour: ") + 2*space))*(4-select)/4, space*3+7*(textSize+1));
	tft.setTextColor(ILI9341_WHITE);

	tft.print(2*select+1); //print the selected option
}

//This method allows the user to scroll through the options for the "Best of"
//setting, and either select their prefered option or retain the previous
//selection and scroll though other settings
//Note: variable select = 1, 2, or 3, for best of 3, 5, or 7 accordingly
uint8_t bestOfSelect(uint8_t prevSelect) {
	uint8_t select = (prevSelect-1)/2; //set the current selection according to the
	//previously selected setting

	delay(del);

	//allow the user to scroll through the options until they exit without selecting
	//a new option or until they select a new option
	while(true) {
		while(Serial3.available() == 0){} //wait for input from the other arduino
		char direction = Serial3.read();//read the joystick input sent from the other arduino
		//if the user goes right and they are not at the last entry then scroll right
		if (direction == 'r' && select<3) {
			redrawBestOf(select);

			select ++;
			drawHighlightBestOf(select);
			delay(del);
		}
		//if the use goes left and they are not at the first entry then scroll left
		else if (direction == 'l' && select>1) {
			redrawBestOf(select);

			select --;
			drawHighlightBestOf(select);
			delay(del);
		}
		//if the user goes left from the first entry, leave the bestOf scrolling
		//method and return the previous selection
		else if (direction =='l' && select==1) {
			redrawBestOf(select);

			select = (prevSelect-1)/2;
			drawHighlightBestOf(select);

			return prevSelect;//therefore use default
		}
		//if the user selects an entry, return their selection and leave the method
		if (direction == 'p') {
			return (2*select+1);
		}
	}
}

//highlights a colour option to show it is selected
void drawHighlightColour(uint8_t pNum, uint8_t select) {
	tft.fillRect(DISPLAY_WIDTH - (DISPLAY_WIDTH-(6*textSize*sizeof("Player 1 Colour: ") + 2*space))*(6-select)/7-1*textSize, //top right x coordingate
								space*(3+2*pNum)+8*(textSize+1)+pNum*8*(textSize)-1*textSize, //top left y coordinate
								8*textSize, 8*textSize, ILI9341_WHITE);
	tft.fillRect(DISPLAY_WIDTH - (DISPLAY_WIDTH-(6*textSize*sizeof("Player 1 Colour: ") + 2*space))*(6-select)/7, //top right x coordingate
								space*(3+2*pNum)+8*(textSize+1)+pNum*8*(textSize), //top left y coordinate
								6*textSize, 6*textSize, colour[select]);
}

//redraws a colour option without highlight to show it is not selected
void redrawColour(uint8_t pNum, uint8_t select) {
	tft.fillRect(DISPLAY_WIDTH - (DISPLAY_WIDTH-(6*textSize*sizeof("Player 1 Colour: ") + 2*space))*(6-select)/7-1*textSize, //top right x coordingate
								space*(3+2*pNum)+8*(textSize+1)+pNum*8*(textSize)-1*textSize, //top left y coordinate
								8*textSize, 8*textSize, ILI9341_BLACK);
	tft.fillRect(DISPLAY_WIDTH - (DISPLAY_WIDTH-(6*textSize*sizeof("Player 1 Colour: ") + 2*space))*(6-select)/7, //top right x coordingate
								space*(3+2*pNum)+8*(textSize+1)+pNum*8*(textSize), //top left y coordinate
								6*textSize, 6*textSize, colour[select]);
}

//This method allows the user to scroll through the colour options for their
//malet and player number, and select their prefered colour, or continue scrolling
//though the other setting and retain their previous selection. This method will
//allow scrolling for either player depending on the player number "pNum"
uint8_t playerColourSelect(uint8_t pNum, uint8_t prevSelect) {
	uint8_t select = prevSelect; //set the initial selection to the previous selection

	delay(del);

	//allow the user to scroll through the options until they select a new option
	//or exit without selecting a new option
	while (true) {
		while(Serial3.available() == 0){} //wait for input from the other arduino
		char direction = Serial3.read(); //read the joystick input sent from the other arduino
		//if the user goes right and they are not at the last entry then scroll right
		if (direction=='r' && select<5) {
			redrawColour(pNum, select);

			select ++;
			drawHighlightColour(pNum, select);

			delay(del);
		}
		//if the use goes left and they are not at the first entry then scroll left
		else if (direction=='l' && select>0) {
			redrawColour(pNum, select);

			select--;
			drawHighlightColour(pNum, select);

			delay(del);
		}
		//if the user goes left from the first entry, leave the player colour
		//scrolling method and return the previous selection
		else if (direction=='l' && select==0) {
			redrawColour(pNum, select);

			select = prevSelect;
			drawHighlightColour(pNum, select);

			return select;
		}
		//if the user selects an entry, return their selection and leave the method
		if (direction == 'p') {
			return select;
		}
	}
}

//draws the Player's new score when it is updated
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

	//print the congragulatory message
	if (player == 1) {
		tft.setTextColor(colour[p1Colour]);
		tft.print(" Player 1 Won!");
	}
	else if (player == 2) {
		tft.setTextColor(colour[p2Colour]);
		tft.print(" Player 2 Won!");
	}

	delay(1000);

	//print the reset button
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
			//redraw the reset button with different colouring to show it has been pressed
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

//draws the player numbers in their selected colours, and the players scores
//as well the scoring mode (ie. best of 3, 5, or 7)
void drawScoreScreen() {
	tft.fillScreen(ILI9341_BLACK);

	tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
	tft.setTextSize(textSize+1);
	tft.setTextWrap(0);

	//print the scoring mode
	tft.setCursor(DISPLAY_WIDTH/2-6*(textSize+1)*(sizeof("Best of: ")+1)/2, space);
	tft.print("Best of: ");
	tft.print(bestOf);

	//print Player 1 in their chosen colour to the left
	tft.setCursor(DISPLAY_WIDTH/3-6*(textSize+1)*sizeof("Player 1")/2-space, space*4 +8*(textSize+1));
	tft.setTextColor(colour[p1Colour]);
	tft.print("Player 1");

	//print Player 2 in their chosen colour to the right
	tft.setCursor(DISPLAY_WIDTH*2/3-6*(textSize+1)*sizeof("Player 2")/2+space, space*4 +8*(textSize+1));
	tft.setTextColor(colour[p2Colour]);
	tft.print("Player 2");

	P1Score = P2Score = 0;
	int maxScore = (bestOf +1)/2;

	//draw the starting scores of each player
	drawScores(0, 1);
	drawScores(0, 2);
	//keep track of player scores until a player recieves the max score
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
	//call the win method for the winning player
	if(P1Score > P2Score){
		win(1);
	}
	else{
		win(2);
	}
}

//draw the menu screen with default selections, then call menuSelect to allow
//the user to scroll through the menu
void drawMenu(){
	tft.fillScreen(ILI9341_BLACK);

	tft.setTextColor(ILI9341_WHITE);
	tft.setTextSize(textSize+1);

	//draw the names of the settings tab
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
	//Horizontal lines between tabs
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
			//print a white background for the default options
			if((i==1 && j==4) || (i==2 && j==1)) {
				tft.fillRect(DISPLAY_WIDTH - (DISPLAY_WIDTH-(6*textSize*sizeof("Player 1 Colour: ") + 2*space))*(6-j)/7-1*textSize, //top right x coordingate
											space*(3+2*i)+8*(textSize+1)+i*8*(textSize)-1*textSize, //top left y coordinate
											8*textSize, 8*textSize, ILI9341_WHITE);
			}
			//print black background for all other options
			else {
				tft.fillRect(DISPLAY_WIDTH - (DISPLAY_WIDTH-(6*textSize*sizeof("Player 1 Colour: ") + 2*space))*(6-j)/7-1*textSize, //top right x coordingate
											space*(3+2*i)+8*(textSize+1)+i*8*(textSize)-1*textSize, //top left y coordinate
											8*textSize, 8*textSize, ILI9341_BLACK);
			}
			//print the colours on top of their backgrounds
			tft.fillRect(DISPLAY_WIDTH - (DISPLAY_WIDTH-(6*textSize*sizeof("Player 1 Colour: ") + 2*space))*(6-j)/7, //top right x coordingate
										space*(3+2*i)+8*(textSize+1)+i*8*(textSize), //top left y coordinate
										6*textSize, 6*textSize, colour[j]);
		}
	}

	//print the play button and highlight it by default
	tft.setCursor(space, DISPLAY_HEIGHT-space-7*textSize);
	tft.setTextColor(ILI9341_BLACK, ILI9341_WHITE);
	tft.print("Play");

	menuSelect(); //allow the user to scroll through the menu
}

//allow the user to scroll through the settings tabs, and call functions to
//scroll through setting options when applicable
void menuSelect() {
	uint8_t select = 3; //select the play button by default
	bestOf = 5;			//game is best of 5 by default
	p1Colour = 4; //player 1 is blue by default
	p2Colour = 1;	//player 2 is red by default

	//array of characters associated with each player colour
	char colourCharacter[] = {'m', 'r', 'y', 'g', 'b', 'c'};

	//array of strings for settings names
	String setting[] = {"Best Of: ", "Player 1 Colour", "Player 2 Colour", "Play"};

	//allow the user to scroll through the settings tabs, and to scroll though
	//the options for the selected setting if they go right from an applicable
	//setting tab name. Start the game if the user pressed play
	while(true) {
		while(Serial3.available() == 0){} //wait for input from the other arduino
		char direction = Serial3.read(); //recieve joystick input from the other arduino
		//if the user goes down and the are not at the bottom entry, scroll down
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
		//if the user scrolls up and they are not at the top entry, scroll up
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
		//if the user scrolls right from any selection apart from "Play", redraw the
		//settings tab name without hightlight, allow the user to scrol through the
		//applicable settings options by calling the applicable function, record the
		//user's selection when they are done, and highlight the setting tab name again
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
		//if the user selects the play button, send the settings information to the
		//other arduino and send the starting character 's' to start the game.
		else if (direction == 'p' && select == 3) {
			tft.setCursor(space, DISPLAY_HEIGHT-space-7*textSize);
			tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
			tft.print("Play");

			delay(1000);
			//send the user's selections to the other arduino
			Serial3.write('b');
			Serial3.write((char)bestOf);
			delay(10);
			Serial3.write('c');
			Serial3.write(colourCharacter[p1Colour]);
			delay(10);
			Serial3.write('C');
			Serial3.write(colourCharacter[p2Colour]);
			delay(10);
			Serial3.write('s'); //signal to start the game
			break; //break from this method
		}
	}
}

//this is the ScoreMachine FSM. It keeps track of the current scores and the menu
//settings. It ensures that the two arduinos are synced up.
void ScoreMachine(){
	enum State {INIT, MENU, SCOREBOARD, ENDSCREEN};
	State currentState = INIT;

	//draw the splash screen the first time the arduino is swtiched on.
	drawSplashScreen();

	while(true){

		//the initialisation state syncs up the two arduinos.
		//continuously sends "A" characters until an "A" is received back.
		if(currentState == INIT){
			Serial3.write((char)65);
			if(Serial3.available() != 0){
				int byteRead = Serial3.read();
				if(byteRead == 65){
					currentState = MENU;
				}
			}
		}

		//the menu state, runs the drawMenu function once the joystick gets pressed in
		//on the other machine. sets the currentState to SCOREBOARD once the drawMenu method finishes running.
		if(currentState == MENU){
			while(Serial3.available() == 0){}
			char byteRead = Serial3.read();
			if(byteRead = 'p'){
				drawMenu();
				currentState = SCOREBOARD;
			}
		}

		//the score board state. goes to ENDSCREEN state once the drawScoreScreen() method finishes running.
		if(currentState == SCOREBOARD){
			drawScoreScreen();
			currentState = ENDSCREEN;
		}

		//the end screen state. Redraws the splash screen and resets the FSM.
		if(currentState == ENDSCREEN){
			drawSplashScreen();
			currentState = INIT;
		}
	}
}

//the main method of the program. sets up the arduino and starts either the
//GameMachine() FSM or ScoreMachine() FSM depending on the CheckPin
int main(){
	setup();

	if(digitalRead(CheckPin) == HIGH){
		GameMachine();
	}
	else if(digitalRead(CheckPin) == LOW){
		ScoreMachine();
	}

	Serial.flush();
	Serial3.flush();
	return 0;
}
