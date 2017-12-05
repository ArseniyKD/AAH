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

struct Circle{
	float xCoord;
	float yCoord;
	float xVelocity;
	float yVelocity;
};

Circle Puck;
Circle MalletP1;
Circle MalletP2;
int speedLimit = 11;
int speedLimitM = 8;

void drawField();

void setup(){
	init();

	Serial.begin(9600);
	tft.begin();

	tft.setRotation(3);
	drawField();

	Puck.xCoord = DISPLAY_WIDTH / 2;
	Puck.yCoord = DISPLAY_HEIGHT / 2;

	tft.fillCircle(Puck.xCoord, Puck.yCoord, PUCK_RAD, ILI9341_RED);

	Puck.xVelocity = 10;
	Puck.yVelocity = 10;

	MalletP1.xCoord = 25;
	MalletP1.yCoord = DISPLAY_HEIGHT / 2;

	MalletP2.xCoord = DISPLAY_WIDTH - 25;
	MalletP2.yCoord = DISPLAY_HEIGHT / 2;

	tft.fillCircle(MalletP1.xCoord, MalletP1.yCoord, MALLET_RAD, ILI9341_BLUE);
	tft.fillCircle(MalletP2.xCoord, MalletP2.yCoord, MALLET_RAD, ILI9341_BLUE);
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

void PuckMovement(){

	tft.fillCircle(Puck.xCoord, Puck.yCoord, PUCK_RAD, ILI9341_BLACK);
	//redraw center line if puck passes over it
	if ((Puck.xCoord-PUCK_RAD >= DISPLAY_WIDTH/2-BORDER/2-PUCK_RAD*2) && (Puck.xCoord+PUCK_RAD <= DISPLAY_WIDTH/2+BORDER/2+PUCK_RAD*2)) {
		tft.fillRect(DISPLAY_WIDTH/2-BORDER/2, Puck.yCoord-PUCK_RAD-2, BORDER, PUCK_RAD*2+3, ILI9341_WHITE);
	}

	Puck.xCoord += Puck.xVelocity;
	Puck.yCoord += Puck.yVelocity;

	if(Puck.xCoord - PUCK_RAD + Puck.xVelocity <= 0 || Puck.xCoord + PUCK_RAD + Puck.xVelocity >= DISPLAY_WIDTH){
		Puck.xVelocity *= -0.95;
	}
	if(Puck.yCoord - PUCK_RAD + Puck.yVelocity <= 0 || Puck.yCoord + PUCK_RAD + Puck.yVelocity >= DISPLAY_HEIGHT){
		Puck.yVelocity *= -0.95;
	}

	Puck.xCoord = constrain(Puck.xCoord, BORDER + PUCK_RAD, DISPLAY_WIDTH - BORDER - PUCK_RAD - 1);
	Puck.yCoord = constrain(Puck.yCoord, BORDER + PUCK_RAD, DISPLAY_HEIGHT - BORDER - PUCK_RAD - 1);

	tft.fillCircle(Puck.xCoord,Puck.yCoord, PUCK_RAD, ILI9341_RED);
	delay(20);

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
	tft.fillCircle(Mallet->xCoord, Mallet->yCoord, MALLET_RAD, ILI9341_BLUE);
}


int main(){
	setup();
	//collisionTime = millis();
	while(true){
		PuckMovement();
		MalletMovement(&MalletP1, 0);
		MalletMovement(&MalletP2, 1);
		checkForCollision(&MalletP1);
		checkForCollision(&MalletP2);

		//repeat in the opposite order to prevent one mallet form being updated with
		//priority over the other
		PuckMovement();
		MalletMovement(&MalletP2, 1);
		MalletMovement(&MalletP1, 0);
		checkForCollision(&MalletP2);
		checkForCollision(&MalletP1);
	}

	return 0;
}
