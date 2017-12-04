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

/*
//Right Joystick
#define JOY_VERT_1  A1
#define JOY_HORIZ_1 A0
#define JOY_SEL_1   2
*/

#define JOY_CENTER   512
#define JOY_DEADZONE 32

#define PUCK_RAD 8
#define MALLET_RAD 12
#define COLLISION_DIST 20


float collision_X, collision_Y;
//int collisionTime;

struct Circle{
	float xCoord;
	float yCoord;
	float xVelocity;
	float yVelocity;
};

Circle Puck;
Circle Mallet[2];
int speedLimit = 11;
int speedLimitM = 8;



void setup(){
	init();

	Serial.begin(9600);
	tft.begin();

	tft.setRotation(3);
	tft.fillScreen(ILI9341_BLACK);

	Puck.xCoord = DISPLAY_WIDTH / 2;
	Puck.yCoord = DISPLAY_HEIGHT / 2;

	tft.fillCircle(Puck.xCoord, Puck.yCoord, PUCK_RAD, ILI9341_RED);

	Puck.xVelocity = 10;
	Puck.yVelocity = 10;

	Mallet[0].xCoord = 25;
	Mallet[0].yCoord = DISPLAY_HEIGHT / 2;

	Mallet[1].xCoord = DISPLAY_WIDTH - 25;
	Mallet[1].yCoord = DISPLAY_HEIGHT / 2;

	tft.fillCircle(Mallet[0].xCoord, Mallet[0].yCoord, MALLET_RAD, ILI9341_BLUE);
	tft.fillCircle(Mallet[1].xCoord, Mallet[1].yCoord, MALLET_RAD, ILI9341_BLUE);
}



void PuckMovement(){

	tft.fillCircle(Puck.xCoord, Puck.yCoord, PUCK_RAD, ILI9341_BLACK);

	Puck.xCoord += Puck.xVelocity;
	Puck.yCoord += Puck.yVelocity;

	if(Puck.xCoord - PUCK_RAD + Puck.xVelocity <= 0 || Puck.xCoord + PUCK_RAD + Puck.xVelocity >= DISPLAY_WIDTH){
		Puck.xVelocity *= -0.95;
	}
	if(Puck.yCoord - PUCK_RAD + Puck.yVelocity <= 0 || Puck.yCoord + PUCK_RAD + Puck.yVelocity >= DISPLAY_HEIGHT){
		Puck.yVelocity *= -0.95;
	}

	Puck.xCoord = constrain(Puck.xCoord, 0 + PUCK_RAD, DISPLAY_WIDTH - PUCK_RAD);
	Puck.yCoord = constrain(Puck.yCoord, 0 + PUCK_RAD, DISPLAY_HEIGHT - PUCK_RAD);

	tft.fillCircle(Puck.xCoord,Puck.yCoord, PUCK_RAD, ILI9341_RED);
	delay(20);

}

void correctPuckLocation(float distance, int num){
	if(distance < COLLISION_DIST){
		tft.fillCircle(Puck.xCoord, Puck.yCoord, PUCK_RAD, ILI9341_BLACK);
		if(Mallet[num].xCoord <= Puck.xCoord){
			Puck.xCoord += COLLISION_DIST - distance;
		}
		if(Puck.xCoord < Mallet[num].xCoord){
			Puck.xCoord -= COLLISION_DIST - distance;
		}
		if(Mallet[num].yCoord < Puck.yCoord){
			Puck.yCoord += COLLISION_DIST - distance;
		}
		if(Puck.yCoord < Mallet[num].yCoord){
			Puck.yCoord -= COLLISION_DIST - distance;
		}

	}

}

void collisionPoint(int num){
	collision_X = ((Mallet[num].xCoord*PUCK_RAD)+(Puck.xCoord*MALLET_RAD))/(COLLISION_DIST);
	collision_Y = ((Mallet[num].yCoord*PUCK_RAD)+(Puck.xCoord*MALLET_RAD))/(COLLISION_DIST);
}

float getDist(float X1, float X2, float Y1, float Y2){
	return sqrt(pow(X1 - X2, 2) + pow(Y1 - Y2, 2));
}

void vectorReflection(int num) {
	int puckMass = 0.1;
	int malletMass = 1;
	float restitution = 1;

	float n[2] = {Puck.xCoord - Mallet[num].xCoord, Puck.yCoord - Mallet[num].yCoord};
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

	float vMalletin = Mallet[num].xVelocity*un[0] + Mallet[num].yVelocity*un[1];

	//compute ut dot product velocity to find tangential component which is constant before and after collision
	float vPuckt = Puck.xVelocity*ut[0] + Puck.yVelocity*ut[1];

	float vMallett = Mallet[num].xVelocity*ut[0] + Mallet[num].yVelocity*ut[1];

	//compute normal component after collision using formula from http://www.vobarian.com/collisions/2dcollisions2.pdf
	float vPuckfn = (vPuckin*(puckMass-malletMass)+2*malletMass*vMalletin)/(puckMass + malletMass);

	float vMalletfn = (vMalletin*(malletMass-puckMass)+2*puckMass*vPuckin)/(puckMass + malletMass);

	//add the x or y portions of the normal and tangential velocities
	Puck.xVelocity = constrain((vPuckfn*un[0] + vPuckt*ut[0])*restitution, -speedLimit, speedLimit);
	Puck.yVelocity = constrain((vPuckfn*un[1] + vPuckt*ut[1])*restitution, -speedLimit, speedLimit);

	Mallet[num].xVelocity = (vMalletfn*un[0] + vMallett*un[0])*restitution;
	Mallet[num].yVelocity = (vMalletfn*un[1] + vMallett*un[1])*restitution;

}

void checkForCollision(int num){


	float distance = getDist(Mallet[num].xCoord, Puck.xCoord, Mallet[num].yCoord, Puck.yCoord);

	if(distance <= COLLISION_DIST){
		//collisionTime = millis();
		correctPuckLocation(distance, num);

		vectorReflection(num);
	}
}

bool antiFlicker(int changeInX, int changeInY, int num){
	if(changeInX < JOY_CENTER - JOY_DEADZONE || changeInX > JOY_CENTER + JOY_DEADZONE
	|| changeInY > JOY_CENTER + JOY_DEADZONE || changeInY < JOY_CENTER - JOY_DEADZONE){
		return true;
	}
	else{
		Mallet[num].xVelocity = 0;
		Mallet[num].yVelocity = 0;
		return false;
	}
}

void MalletMovement(int num){
	//if (num == 0) {
		int xVal = analogRead(JOY_HORIZ_0);
		int yVal = analogRead(JOY_VERT_0);
	//}

	/*
	if(num == 1) {
		int xVal = analogRead(JOY_HORIZ_1);
		int yVal = analogRead(JOY_VERT_1);
	}
	*/

	if(antiFlicker(xVal, yVal, num)){

		tft.fillCircle(Mallet[num].xCoord, Mallet[num].yCoord, MALLET_RAD, ILI9341_BLACK);

		if(yVal < JOY_CENTER - JOY_DEADZONE){
	    //move the cursor up with a speed proportional to yVal
	    Mallet[num].yCoord -= constrain((JOY_CENTER - yVal) / 40, -speedLimitM, speedLimitM);
		  Mallet[num].yVelocity = constrain((yVal - JOY_CENTER) / 40, -speedLimitM, speedLimitM);
	  }

    //if the joystick has been pushed down
    else if(yVal > JOY_CENTER + JOY_DEADZONE){
	    //move the cursor down with a speed proportional to yVal
	    Mallet[num].yCoord += constrain((yVal - JOY_CENTER) / 40, -speedLimitM, speedLimitM);
		  Mallet[num].yVelocity = constrain((yVal - JOY_CENTER) / 40, -speedLimitM, speedLimitM);
    }

    //if the joystick has been moved left
    if(xVal > JOY_CENTER + JOY_DEADZONE){
	    //move the cursor left with a speed proportional to xVal
	    Mallet[num].xCoord -= constrain((xVal - JOY_CENTER)/40, -speedLimitM, speedLimitM);
		  Mallet[num].xVelocity = constrain((JOY_CENTER - xVal)/40, -speedLimitM, speedLimitM);
    }

    //if the joystick has been moved right
    else if( xVal < JOY_CENTER - JOY_DEADZONE){
      //move the cursor right with a speed proportional to xVal
      Mallet[num].xCoord += constrain((JOY_CENTER - xVal)/40, -speedLimitM, speedLimitM);
	  	Mallet[num].xVelocity = constrain((JOY_CENTER - xVal)/40, -speedLimitM, speedLimitM);
    }

		//constrain the location of the cursor to be within the screen dimensions
    Mallet[num].xCoord = constrain(Mallet[num].xCoord, MALLET_RAD ,DISPLAY_WIDTH - MALLET_RAD);
    //constrain the location of the cursor to be within the screen dimensions
    Mallet[num].yCoord = constrain(Mallet[num].yCoord, MALLET_RAD ,DISPLAY_HEIGHT - MALLET_RAD);
	}

	tft.fillCircle(Mallet[num].xCoord, Mallet[num].yCoord, MALLET_RAD, ILI9341_BLUE);

}


int main(){
	setup();
	//collisionTime = millis();
	while(true){
		PuckMovement();
		MalletMovement(0);
		MalletMovement(1);
		checkForCollision(0);
		checkForCollision(1);
		
		//repeat in the opposite order to prevent one mallet form being updated with
		//priority over the other
		PuckMovement();
		MalletMovement(1);
		MalletMovement(0);
		checkForCollision(1);
		checkForCollision(0);
	}

	return 0;
}
