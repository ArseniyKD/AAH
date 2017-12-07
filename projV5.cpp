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

#define PUCK_RAD 8
#define MALLET_RAD 12
#define COLLISION_DIST 20

#define BORDER 2
#define GOAL 80

float collision_X, collision_Y;
//int collisionTime;
int CheckPin = 12;

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



void setup(){
	init();

	Serial.begin(9600);
	tft.begin();
	Serial3.begin(9600);

	tft.setRotation(3);
	pinMode(JOY_SEL_1, INPUT_PULLUP);
	pinMode(CheckPin, INPUT);
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

void collisionPoint(Circle *Mallet){
	collision_X = ((Mallet->xCoord*PUCK_RAD)+(Puck.xCoord*MALLET_RAD))/(COLLISION_DIST);
	collision_Y = ((Mallet->yCoord*PUCK_RAD)+(Puck.xCoord*MALLET_RAD))/(COLLISION_DIST);
}

float getDist(float X1, float X2, float Y1, float Y2){
	return sqrt(pow(X1 - X2, 2) + pow(Y1 - Y2, 2));
}

void vectorReflection(Circle *Mallet) {
	int puckMass = 0.1;
	int malletMass = 1;
	float restitution = 1;

	float n[2] = {Puck.xCoord - Mallet->xCoord, Puck.yCoord - Mallet->yCoord};
	// for(int i = 0; i <2; i++){
	// 	Serial.print("n values: ");
	// 	Serial.println(n[i]);
	// }

	//find the unit vector for the normal line
  float un[2]	 = {n[0]/sqrtf((n[0]*n[0])+(n[1]*n[1])), n[1]/sqrtf((n[0]*n[0])+(n[1]*n[1]))};
	// for(int i = 0; i <2; i++){
	// 	Serial.print("un values: ");
	// 	Serial.println(un[i]);
	// }

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
	//if (num == 0) {
	int xVal, yVal;
	if(n == 0){
		xVal = analogRead(JOY_HORIZ_0);
		yVal = analogRead(JOY_VERT_0);
	}
	else if(n==1){
		xVal = analogRead(JOY_HORIZ_1);
		yVal = analogRead(JOY_VERT_1);

	}

	/*
	if(num == 1) {
		int xVal = analogRead(JOY_HORIZ_1);
		int yVal = analogRead(JOY_VERT_1);
	}
	*/

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
			Serial.write('u');
			delay(1000);
		}

		else if (yVal > JOY_CENTER + JOY_DEADZONE){
			Serial.write('d');
			delay(1000);
		}

		if(xVal < JOY_CENTER - JOY_DEADZONE){
			Serial.write('r');
			delay(1000);
		}

		else if(xVal > JOY_CENTER + JOY_DEADZONE){
			Serial.write('l');
			delay(1000);
		}

		if(digitalRead(JOY_SEL_1) == LOW){
			Serial.write('p');
			delay(1000);
		}

		if(Serial.available() != 0){
			char byteRead = Serial.read();
			delay(1000);
			Serial.print(byteRead);

			if(byteRead == 'c'){
				delay(1000);
				char colourRead = Serial.read();
				Serial.print(colourRead);
				if(colourRead == 'm'){MalletP1.Colour = ILI9341_MAGENTA;}
				else if (colourRead == 'r'){MalletP1.Colour = ILI9341_RED;}
				else if (colourRead == 'y'){MalletP1.Colour = ILI9341_YELLOW;}
				else if (colourRead == 'g'){MalletP1.Colour = ILI9341_GREEN;}
				else if (colourRead == 'b'){MalletP1.Colour = ILI9341_BLUE;}
				else if (colourRead == 'c'){MalletP1.Colour = ILI9341_CYAN;}
			}

			if(byteRead == 'C'){
				delay(1000);
				char colourRead = Serial.read();
				Serial.print(colourRead);
				if(colourRead == 'm'){MalletP2.Colour = ILI9341_MAGENTA;}
				else if (colourRead == 'r'){MalletP2.Colour = ILI9341_RED;}
				else if (colourRead == 'y'){MalletP2.Colour = ILI9341_YELLOW;}
				else if (colourRead == 'g'){MalletP2.Colour = ILI9341_GREEN;}
				else if (colourRead == 'b'){MalletP2.Colour = ILI9341_BLUE;}
				else if (colourRead == 'c'){MalletP2.Colour = ILI9341_CYAN;}
			}

			if(byteRead == 'b'){
				delay(1000);
				char gameNumRead = Serial.read();
				Serial.print(gameNumRead);
				if(gameNumRead == '3'){maxScore = 2;}
				else if (gameNumRead == '5'){maxScore = 3;}
				else if (gameNumRead == '7'){maxScore = 4;}
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
	int P1Score = 0;
	int P2Score = 0;
	while(P1Score < maxScore && P2Score < maxScore){
		setupGame();
		bool flag = true;

		while(flag){
			int scoreUpdate = PuckMovement();

			if(scoreUpdate == -1){
				P1Score++;
				Serial.write('p');
				flag = false;
			}

			else if(scoreUpdate == 1){
				P2Score++;
				Serial.write('P');
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
	tft.fillScreen(ILI9341_BLACK);
	delay(250);
}

void endScr(){
	while(digitalRead(JOY_SEL_1) == HIGH){}
	Serial3.write('r');
}

void GameMachine(){
	enum State {INIT, ACK, PREGAME, GAME, ENDSCREEN};
	State currentState = INIT;

	while(true){
		if(currentState == INIT){
			if(Serial.available() != 0){
				int byteRead = Serial.read();
				if(byteRead == 65){
					currentState = ACK;
				}
			}

		}

		if(currentState == ACK){
			Serial.write((char)65);
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



int main(){
	setup();

	if(digitalRead(CheckPin) == HIGH){
		GameMachine();
	}
	else if(digitalRead(CheckPin) == LOW){
		//ScoreMachine();
	}
	//collisionTime = millis();


	return 0;
}
